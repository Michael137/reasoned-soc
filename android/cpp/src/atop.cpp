#include <errno.h>
#include <sys/wait.h>

#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <future>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include "ctpl.h"

#include "atop.h"
#include "logger.h"
#include "util.h"

using namespace std::chrono_literals;

// Constants
static const std::string TFLITE_BENCHMARK_BIN
    = "/data/local/tmp/benchmark_model";
static const std::string SNPE_BENCHMARK_BIN
    = "/data/local/tmp/snpebm/artifacts/arm-android-clang6.0/bin/snpe-net-run";

static inline void handle_system_return( int status,
                                         bool terminate_on_err = false )
{
	if( status < 0 )
		throw std::runtime_error( fmt::format(
		    "Error when running system cmd: {0}", std::strerror( errno ) ) );

	if( !WIFEXITED( status ) )
	{
		if( terminate_on_err )
			throw std::runtime_error( fmt::format(
			    "Program exited abnormally: !WIFEXITED({0})", status ) );
		else
			atop::logger::warn( fmt::format(
			    "Program exited abnormally: !WIFEXITED({0})", status ) );
	}
}

// TODO: check exit code using WEXITSTATUS
atop::shell_out_t atop::check_console_output( std::string const& cmd )
{
	std::vector<char> buffer( 512 );
	std::unique_ptr<FILE, decltype( &pclose )> pipe( popen( cmd.c_str(), "r" ),
	                                                 pclose );
	std::string output;

	if( !pipe )
	{
		throw std::runtime_error( "popen() failed!" );
	}

	// TODO: Use fread() instead?
	while( fgets( buffer.data(), static_cast<int>( buffer.size() ), pipe.get() )
	       != nullptr )
	{
		output += buffer.data();
	}

	atop::util::trim( output );

	return atop::util::split( output, '\n' );
}

static bool in_adb_root()
{
	return atop::check_console_output( "adb shell whoami" )[0] == "root";
}

void atop::check_reqs()
{
	std::array required_bins{"adb"};
	std::vector<std::string> not_found_bins{};

	for( auto& bin: required_bins )
	{
		if( atop::check_console_output( fmt::format( "which {0}", bin ) ).size()
		    == 0 )
			not_found_bins.push_back( bin );
	}

	if( not_found_bins.size() > 0 )
	{
		spdlog::error( "Following required binaries not found:" );
		for( auto& bin: not_found_bins )
			spdlog::error( fmt::format( "\t{0}", bin ) );
		std::abort();
	}

	// Is device connected?
	// NOTE: command will always return a header line followed by
	//       one line for each connected device
	auto devices = atop::check_console_output( "adb devices" );
	if( devices.size() == 1 )
		atop::logger::log_and_exit( "No devices connected" );
	else if( devices.size() > 2 )
		atop::logger::log_and_exit(
		    "atop expects only a single connected Android device" );

	auto adb_output  = atop::util::split( devices[1], '\t' );
	auto device_name = adb_output[0];
	auto adb_status  = adb_output[1];

	if( adb_status == "unauthorized" )
		atop::logger::log_and_exit(
		    "Permission denied...please allow adb access to your device (e.g., "
		    "enable USB debugging)" );

	// Is adb connected as root?
	if( !in_adb_root() )
	{
		atop::logger::warn( "atop requires adb in root...restarting as root" );
		handle_system_return( std::system( "adb root" ) );
		if( !in_adb_root() )
			atop::logger::log_and_exit( "Failed to restart adb in root" );
	}

	atop::logger::verbose_info(
	    fmt::format( "Found device: {0}", device_name ) );

	// TODO: check adb write permissions
}

static atop::shell_out_t check_adb_shell_output( std::string const& cmd )
{
	return atop::check_console_output( fmt::format( "adb shell \"{}\"", cmd ) );
}

static atop::shell_out_t check_dmesg_log()
{
	return check_adb_shell_output( "dmesg" );
}

// Zip turns
//
// { "PROBE 1 ....",
//   "PROBE 2 ....",
//   "PROBE 1 ....",
//   "PROBE 2 ....",
//   "PROBE 2 ...." }
//
// into
//
// {{ "PROBE 1 ....",
//    "PROBE 1 ...." },
//  { "PROBE 2 ....",
//    "PROBE 2 ....",
//    "PROBE 2 ...." }
//    TODO: log can be &&
static atop::ioctl_dmesg_t
process_and_zip_dmesg_log( atop::shell_out_t&& log,
                           std::vector<std::string> const& probes )
{
	// Match dmesg timestamp followed by probe
	std::string probes_regex
	    = std::accumulate( std::next( probes.begin() ), probes.end(), probes[0],
	                       []( std::string const& a, std::string const& b ) {
		                       return a + "|" + b;
	                       } );
	std::string regex_str = std::string( R"(\[[0-9\.\s]+\]\s+)" ) + "("
	                        + probes_regex.c_str() + ")";

	std::regex pattern{regex_str};
	std::smatch match;
	atop::ioctl_dmesg_t results;
	for( auto& line: log )
	{
		if( std::regex_search( line, match, pattern ) )
		{
			results[PROBE_IDX( atop::string2dmesgProbes( match[1].str() ) )]
			    .emplace_back( line );
		}
	}

	return results;
}

atop::IoctlDmesgStreamer::IoctlDmesgStreamer(
    std::vector<std::string> const& probes )
    : utilization_probe( atop::DmesgProbes::IOCTL )
    , is_data_fresh( false )
    , latest_ts( 0.0 )
    , latest_data( process_and_zip_dmesg_log( check_dmesg_log(), probes ) )
    , latest_interactions( {} )
    , probes( probes )
{
	std::vector<double> max_tses;
	max_tses.reserve( this->latest_data.size() );
	for( size_t i = 0; i < this->latest_data.size(); ++i )
		if( this->latest_data[i].size() > 0 )
			max_tses.push_back(
			    atop::util::extract_time( this->latest_data[i].back() ) );

	if( max_tses.size() > 0 )
		this->latest_ts = *std::max_element( max_tses.begin(), max_tses.end() );
}

atop::ioctl_dmesg_t const& atop::IoctlDmesgStreamer::more()
{
	auto data = process_and_zip_dmesg_log( check_dmesg_log(), this->probes );
	for( size_t i = 0; i < data.size(); ++i )
	{
		this->latest_data[i].clear();
		for( auto it = data[i].rbegin(); it != data[i].rend(); ++it )
		{
			if( atop::util::extract_time( *it ) > this->latest_ts )
				this->latest_data[i].push_back( *it );
			else
				break;
		}
	}

	std::vector<double> max_tses;
	max_tses.reserve( this->latest_data.size() );
	for( size_t i = 0; i < this->latest_data.size(); ++i )
		if( this->latest_data[i].size() > 0 )
			max_tses.push_back(
			    atop::util::extract_time( this->latest_data[i][0] ) );

	if( max_tses.size() > 0 )
	{
		this->latest_ts = *std::max_element( max_tses.begin(), max_tses.end() );
		this->is_data_fresh = true;
	}
	else
		this->is_data_fresh = false;

	return this->latest_data;
}

static std::string extract_dmesg_accl_tag( std::string const& str )
{
	std::regex pattern{R"(IOCTL ([A-Za-z0-9_]+):)"};
	std::smatch match;

	if( std::regex_search( str, match, pattern ) )
	{
		std::string ts_str = match[1].str();
		atop::util::trim( ts_str );

		return ts_str;
	}
	else
		return "";
}

std::map<std::string, int> const&
atop::IoctlDmesgStreamer::interactions( bool check_full_log, double threshold )
{
	std::vector<std::string> eligible;
	auto data = this->more()[PROBE_IDX( this->utilization_probe )];
	if( data.size() == 0 )
	{
		std::for_each( this->latest_interactions.begin(),
		               this->latest_interactions.end(),
		               [&]( auto& p ) { p.second = 0; } );
		return this->latest_interactions;
	}
	else
	{
		auto most_recent = atop::util::extract_time( data[0] );
		if( check_full_log )
			eligible = this->latest_data[PROBE_IDX( this->utilization_probe )];
		else
		{
			for( auto it = data.rbegin(); it != data.rend(); ++it )
			{
				if( atop::util::extract_time( *it )
				    >= ( most_recent - threshold ) )
					eligible.push_back( *it );
				else
					break;
			}
		}

		// Reset
		for( auto& p: this->latest_interactions )
			this->latest_interactions[p.first] = 0;
		// this->latest_interactions.clear();

		std::string tag;
		for( auto& line: eligible )
		{
			tag = extract_dmesg_accl_tag( line );
			if( !tag.empty() )
			{
				if( this->latest_interactions.find( tag )
				    == this->latest_interactions.end() )
					this->latest_interactions.insert( {tag, 0} );
				this->latest_interactions[tag] += 1;
			}
		}
	}

	return this->latest_interactions;
}

static bool file_exists_on_device( std::string const& pth )
{
	return atop::check_console_output( "adb shell ls " + pth )[0] == pth;
}

static void check_file_exists_on_device( std::string const& pth )
{
	if( !file_exists_on_device( pth ) )
		atop::logger::log_and_exit(
		    fmt::format( "Path '{0}' doesn't exist on device", pth ) );
}

static void check_tflite_reqs()
{
	check_file_exists_on_device( TFLITE_BENCHMARK_BIN );
}

static void check_snpe_reqs()
{
	check_file_exists_on_device( SNPE_BENCHMARK_BIN );
}

static std::vector<std::string> get_tflite_models()
{
	// TODO: create separate directory for tflite benchmark models
	std::vector<std::string> paths = atop::check_console_output(
	    "adb shell ls -d /data/local/tmp/*.tflite" );
	return paths;
}

static std::vector<std::string> get_snpe_models()
{
	std::vector<std::string> paths = atop::check_console_output(
	    "adb shell find /data/local/tmp/snpebm/models -name \"*.dlc\"" );
	return paths;
}

std::vector<std::string> atop::get_models_on_device( atop::Frameworks fr )
{
	switch( fr )
	{
		case atop::Frameworks::mlperf:
			throw atop::util::NotImplementedException(
			    fmt::format( "Framework {0} not yet implemented",
			                 atop::framework2string( fr ) ) );
		case atop::Frameworks::SNPE:
			check_snpe_reqs();
			return get_snpe_models();
		case atop::Frameworks::tflite:
			check_tflite_reqs();
			return get_tflite_models();
	}

	throw std::logic_error( fmt::format( "Framework {0} not supported",
	                                     atop::framework2string( fr ) ) );
}

static std::future<atop::shell_out_t> run_benchmark(
    std::string const& benchmark_bin, fmt::string_view options_fmt,
    fmt::string_view model_fmt, std::vector<std::string> const& model_paths,
    std::map<std::string, std::string> const& options, int processes,
    std::string const& prefix = "",
    std::function<std::string( std::string const& )> const& suffix_gen
    = nullptr,
    std::function<atop::shell_out_t( std::string const& )> const& after_runner
    = nullptr,
    int repeat = 0 )
{
	// The user is unlikely to spawn more than
	// 8 threads simultaneously (unless this function
	// is ever exposed to the command line)
	static ctpl::thread_pool pool( 8 );

	static atop::util::RandomSelector rselect{};
	std::stringstream base_cmd;
	// taskset f0: run benchmark on the big cores of big.LITLE ARM CPUs.
	// This reduces variance between benchmark runs
	base_cmd << prefix << ( ( prefix.empty() ) ? "" : ";" ) << " taskset f0 "
	         << benchmark_bin;
	for( auto&& p: options )
	{
		base_cmd << " ";
		base_cmd << fmt::format( options_fmt, p.first, p.second );
	}

	std::string base_cmd_str{base_cmd.str()};
	std::stringstream cmd;
	std::string model_fmt_str = fmt::format( " {0} & ", model_fmt );
	std::string selected_model;
	cmd << base_cmd_str;
	for( int i = 0; i < processes; ++i )
	{
		selected_model = rselect( model_paths );
		if( suffix_gen != nullptr )
			cmd << " " << suffix_gen( selected_model ) << " ";

		cmd << fmt::format( model_fmt_str, selected_model );

		if( i < processes - 1 )
			cmd << base_cmd_str;
	}

	cmd << "wait";

	if( pool.n_idle() > 0 )
	{
		atop::logger::verbose_info(
		    fmt::format( "{0}/{1} threads availble in {2} thread pool. "
		                 "Scheduling new task...",
		                 pool.n_idle(), pool.size(), __FUNCTION__ ) );
	}
	else
	{
		atop::logger::verbose_info(
		    fmt::format( "Capacity of {0} thread pool reached (max: {1}). "
		                 "Clearing previous threads...",
		                 __FUNCTION__, pool.size() ) );
		pool.stop();
	}

	atop::logger::verbose_info(
	    fmt::format( "Running benchmark using: {0}", cmd.str() ) );

	auto cmd_str = cmd.str();
	std::future<atop::shell_out_t> f
	    = pool.push( [cmd_str, after_runner, selected_model, repeat]( int ) {
		      // TODO: For now throw away stdout except for first benchmark;
		      // for now this is fine but if any other framework requires it
		      // later on the return type of run_benchmark will have to change
		      // to std::vector<atop::shell_out_t>
		      auto out = check_adb_shell_output( cmd_str );
		      for( int i = 0; i < repeat; ++i )
		      {
			      // TODO: could make this sleep parameter configurable
			      std::this_thread::sleep_for( 1s );
			      check_adb_shell_output( cmd_str );
		      }

		      if( after_runner != nullptr )
			      // TODO: should actually operate on the output of the actual
			      // run; log collection should be independent of models run
			      // and simply aggregate the results into a
			      // concurrent_benchmark.csv
			      return after_runner( selected_model );
		      else
			      return out;
	      } );

	return f;
}

std::future<atop::shell_out_t>
atop::run_tflite_benchmark( std::vector<std::string> const& model_paths,
                            std::map<std::string, std::string> const& options,
                            int processes )
{
	return run_benchmark( TFLITE_BENCHMARK_BIN, "--{0}={1}", "--graph={0}",
	                      model_paths, options, processes );
}

static atop::shell_out_t
get_snpe_diagview_output( std::string const& model_path )
{
	auto log_files = check_adb_shell_output(
	    fmt::format( "ls {0}/output/SNPEDiag_*.log",
	                 atop::util::basepath( model_path ).c_str() ) );

	// TODO: check whether diagview exists
	atop::shell_out_t out;
	for( auto&& file: log_files )
	{
		auto more = atop::check_console_output(
		    fmt::format( "adb shell cat {0} > /tmp/diag.log && "
		                 "snpe-diagview --input_log /tmp/diag.log",
		                 file.c_str() ) );

		// Append to output vector
		out.insert( out.end(), std::make_move_iterator( more.begin() ),
		            std::make_move_iterator( more.end() ) );
	}

	return out;
}

std::future<atop::shell_out_t>
atop::run_snpe_benchmark( std::vector<std::string> const& model_paths,
                          std::map<std::string, std::string> const& options,
                          int processes, int num_runs )
{
	// Awkward ADSP_LIBRARY_PATH because paths need to be
	// separated by ";" instead of the usual ":"
	std::string prefix
	    = "export LD_LIBRARY_PATH=/data/local/tmp/snpebm/artifacts/"
	      "arm-android-clang6.0/lib:$LD_LIBRARY_PATH"
	      ";"
	      "export ADSP_LIBRARY_PATH=\\\"/data/local/tmp/snpebm/artifacts/"
	      "arm-android-clang6.0/lib/../../dsp/lib;/system/lib/rfsa/adsp;"
	      "/usr/lib/rfsa/adsp;/system/vendor/lib/rfsa/adsp;"
	      "/dsp;/etc/images/dsp;\\\"";

	for( auto&& m: model_paths )
	{
		std::string base_path = atop::util::basepath( m );

		atop::logger::verbose_info( fmt::format(
		    "Deleting previous benchmark results in {0}/output", base_path ) );

		check_adb_shell_output(
		    fmt::format( "rm -rf {0}/output/SNPEDiag_*.log", base_path ) );
	}

	return run_benchmark(
	    SNPE_BENCHMARK_BIN, "--{0} {1}", "--container {0}", model_paths,
	    options, processes, prefix,
	    []( std::string const& model ) {
		    return fmt::format(
		        "--input_list {0}/target_raw_list.txt --output {0}/output",
		        atop::util::basepath( model ) );
	    },
	    get_snpe_diagview_output, num_runs - 1 /* = repeat */ );
}

static inline bool is_cpu_str( std::string const& line )
{
	return line.size() > 4 && line[0] == 'c' && line[1] == 'p' && line[2] == 'u'
	       && std::isdigit( static_cast<unsigned char>( line[3] ) );
}

static std::vector<std::vector<uint64_t>> get_proc_stat_cpu_info()
{
	auto out     = atop::check_console_output( "adb shell cat /proc/stat" );
	auto out_end = std::remove_if(
	    out.begin(), out.end(),
	    [&]( std::string const& line ) { return !is_cpu_str( line ); } );
	out.erase( out_end, out.end() );

	std::vector<std::vector<uint64_t>> res;
	res.reserve( out.size() );

	for( auto&& e: out )
	{
		auto split = atop::util::split( e, ' ' );
		// Ignore the string label of the row (e.g., "cpu0" in "cpu0 123
		// 123..")
		split.erase( split.begin() );

		res.push_back( {} );
		for( size_t i = 0; i < split.size(); ++i )
		{
			char* end;
			res.back().push_back( std::strtoull( split[i].c_str(), &end, 10 ) );
		}
	}

	return res;
}

atop::CpuUtilizationStreamer::CpuUtilizationStreamer()
    : total_tick()
    , total_tick_old()
    , idle()
    , idle_old()
    , del_total_tick()
    , del_idle()
    , latest_utils()
    , num_cpus( 0 )
{
	auto info      = get_proc_stat_cpu_info();
	auto sz        = info.size();
	this->num_cpus = static_cast<int>( sz );

	this->idle_old.resize( sz, 0 );
	this->del_total_tick.resize( sz, 0 );
	this->del_idle.resize( sz, 0 );
	this->total_tick.resize( sz, 0 );
	this->total_tick_old.resize( sz, 0 );
	this->idle.resize( sz, 0 );
}

std::map<std::string, double> const&
atop::CpuUtilizationStreamer::utilizations()
{
	auto info = get_proc_stat_cpu_info();

	for( size_t i = 0; i < this->total_tick.size(); ++i )
	{
		this->total_tick_old[i] = this->total_tick[i];
		this->idle_old[i]       = this->idle[i];

		this->total_tick[i] = std::accumulate( info[i].begin(), info[i].end(),
		                                       static_cast<uint64_t>( 0 ),
		                                       std::plus<uint64_t>() );

		this->idle[i] = info[i][3];

		this->del_total_tick[i] = this->total_tick[i] - this->total_tick_old[i];
		this->del_idle[i]       = this->idle[i] - this->idle_old[i];

		this->latest_utils[fmt::format( "cpu{0}", i )]
		    = ( static_cast<double>( this->del_total_tick[i]
		                             - this->del_idle[i] )
		        / static_cast<double>( this->del_total_tick[i] ) )
		      * 100;
	}

	return this->latest_utils;
}

static void summarize_tflite_benchmark_output( atop::shell_out_t const& out,
                                               atop::BenchmarkStats& stats )
{
	char* end;
	for( auto& line: out )
	{
		if( line.rfind( "PRE-PROCESSING", 0 ) == 0 )
		{
			stats.stats["preproc"] = strtoull(
			    atop::util::split( line, ' ' )[1].c_str(), &end, 10 );
		}
		if( line.rfind( "Inference timings in us", 0 ) == 0 )
		{
			// Extract "Init" and "Inference (avg)" results
			std::regex pattern{
			    R"(Init: ([0-9]+)([0-9a-zA-Z:\s,\(\)].*)Inference \(avg\): ([0-9]+))"};
			std::smatch match;

			if( std::regex_search( line, match, pattern ) )
			{
				stats.stats["init"]
				    = strtoull( match[1].str().c_str(), &end, 10 );
				// Inference time in tflite benchmark doesn't include
				// pre-/post-processing
				stats.stats["inference"]
				    = strtoull( match[3].str().c_str(), &end, 10 )
				      - stats.stats["offload"];
			}
		}
	}
}

static void summarize_snpe_benchmark_output( atop::shell_out_t const& out,
                                             atop::BenchmarkStats& stats )
{
	bool start_parsing = false;
	char* end;

	// SNPE Stats
	std::map<std::string, uint64_t> snpe_stats;

	unsigned header_cnt = 0;

	for( auto&& line: out )
	{
		// Assuming the result sections are ordered as:
		// 1. Dnn Runtime Load/Deserialize/Create/De-Init Statistics
		// 2. Average Statistics
		// 3. Layer Times section
		if( line.rfind(
		        "Dnn Runtime Load/Deserialize/Create/De-Init Statistics", 0 )
		    == 0 )
		{
			start_parsing = true;
			header_cnt++;
			continue;
		}

		if( line.rfind( "Layer Times", 0 ) == 0 )
		{
			start_parsing = false;
			continue;
		}

		if( start_parsing )
		{
			// Match e.g. Create Networks(s): 12345 us
			std::regex pattern{R"(([A-Za-z\s\(\)-]+):\s([0-9]+) us)"};
			std::smatch match;

			if( std::regex_search( line, match, pattern ) )
			{
				uint64_t val = strtoull( match[2].str().c_str(), &end, 10 );
				if( auto it{snpe_stats.find( match[1].str() )};
				    it != std::end( snpe_stats ) )
					( *it ).second += val;
				else
					snpe_stats.insert(
					    std::pair<std::string, uint64_t>( match[1], val ) );
			}
		}
	}

	for( auto& e: snpe_stats )
		e.second = e.second / header_cnt;

	// SNPE workloads are already pre-processed
	stats.stats["preproc"] = 0;

	stats.stats["inference"] = snpe_stats["Forward Propagate Time"];

	stats.stats["offload"]
	    = ( snpe_stats["RPC Init Time"] - snpe_stats["Accelerator Init Time"] )
	      + ( snpe_stats["RPC Execute"] - snpe_stats["Accelerator"] );
	stats.stats["postproc"] = 0;
	stats.stats["init"]     = snpe_stats["Init"];
}

void atop::summarize_benchmark_output( shell_out_t const& out,
                                       atop::Frameworks fr,
                                       atop::BenchmarkStats& stats )
{
	switch( fr )
	{
		case atop::Frameworks::tflite:
			summarize_tflite_benchmark_output( out, stats );
			break;
		case atop::Frameworks::SNPE:
			summarize_snpe_benchmark_output( out, stats );
			break;
		case atop::Frameworks::mlperf:
			throw atop::util::NotImplementedException(
			    "Framework mlperf not yet implemented" );
	};
}

static void ioctl_breakdown_impl(
    std::map<std::string, std::map<std::string, int>>& breakdown,
    atop::shell_out_t const& data, std::string const& pattern_str )
{
	std::smatch match;
	std::regex pattern{pattern_str};

	for( auto&& line: data )
	{
		if( std::regex_search( line, match, pattern ) )
		{
			std::string app = match[1].str();
			std::string cmd = match[2].str();
			// Application in map?
			if( auto it{breakdown.find( app )}; it == std::end( breakdown ) )
				breakdown.insert(
				    std::pair<std::string, std::map<std::string, int>>( app,
				                                                        {} ) );

			if( auto ioctl_it{breakdown[app].find( cmd )};
			    ioctl_it == std::end( breakdown[app] ) )
				breakdown[app].insert( std::pair<std::string, int>( cmd, 0 ) );

			breakdown[app][cmd]++;
		}
	}
}

void atop::ioctl_breakdown(
    std::map<std::string, std::map<std::string, int>>& breakdown,
    atop::shell_out_t const& data, atop::DmesgProbes probe )
{
	switch( probe )
	{
		case atop::DmesgProbes::IOCTL:
		{
			std::string tag_pattern = R"([\(\)a-z\s:0-9\-_]*)";
			std::string cmd_pattern = R"(\(cmd: ([a-zA-Z0-9\s]+) \[[0-9]+\]\))";
			std::string app_pattern = tag_pattern
			                          + R"(\(app: ([a-zA-Z_:@\-0-9]+)\))" + " "
			                          + cmd_pattern + tag_pattern;
			auto pattern_str
			    = R"(\[[0-9\.\s*]+\] IOCTL [a-zA-Z]+)" + app_pattern;
			ioctl_breakdown_impl( breakdown, data, pattern_str );
		}
		break;
		case atop::DmesgProbes::INFO:
		{
			auto pattern_str
			    = R"(\[[0-9\.\s*]+\] INFO: \(app: ([a-zA-Z_:@\-0-9]+)\) ([a-zA-Z\s_\/\-0-9]+))";
			ioctl_breakdown_impl( breakdown, data, pattern_str );
		}
		break;
		default:
			throw atop::util::NotImplementedException(
			    "Breakdown for given probe not implemented" );
	}
}
