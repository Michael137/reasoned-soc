#include <errno.h>
#include <sys/wait.h>

#include <array>
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
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include "ctpl.h"

#include "atop.h"
#include "logger.h"
#include "util.h"

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
std::vector<std::string> atop::check_console_output( std::string const& cmd )
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

static std::vector<std::string> check_adb_shell_output( std::string const& cmd )
{
	return atop::check_console_output( fmt::format( "adb shell \"{}\"", cmd ) );
}

static std::vector<std::string> check_dmesg_log()
{
	return check_adb_shell_output( "dmesg" );
}

static std::vector<std::string>
process_dmesg_log( std::vector<std::string> const& log,
                   std::string const& probe )
{
	std::vector<std::string> results;
	for( auto& line: log )
	{
		if( line.find( probe ) == std::string::npos )
			continue;

		results.emplace_back( line );
	}

	return results;
}

atop::IoctlDmesgStreamer::IoctlDmesgStreamer()
    : latest_ts( 0.0 )
    , latest_data( process_dmesg_log( check_dmesg_log(), "IOCTL" ) )
    , latest_interactions( {
          //          {"ardeno", 0},
          //          {"kgsl", 0},
          //          {"vidioc", 0},
          //          {"cam_sensor", 0},
          //          {"v4l2", 0},
          //          {"IPA", 0},
          //          {"ICE", 0},
      } )
{
	if( this->latest_data.size() > 0 )
		this->latest_ts = atop::util::extract_time( this->latest_data.back() );
}

std::vector<std::string> const& atop::IoctlDmesgStreamer::more()
{
	auto data = process_dmesg_log( check_dmesg_log(), "IOCTL" );
	this->latest_data.clear();
	for( auto it = data.rbegin(); it != data.rend(); ++it )
	{
		if( atop::util::extract_time( *it ) > this->latest_ts )
			this->latest_data.push_back( *it );
		else
			break;
	}

	if( this->latest_data.size() > 0 )
		this->latest_ts = atop::util::extract_time( this->latest_data[0] );

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
	auto data = this->more();
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
			eligible = this->latest_data;
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

void atop::check_tflite_reqs()
{
	std::string const benchmark_bin = "/data/local/tmp/benchmark_model";
	check_file_exists_on_device( benchmark_bin );
}

std::vector<std::string> atop::get_tflite_models()
{
	// TODO: use filesystem lib
	// TODO: create separate directory for tflite benchmark models
	std::vector<std::string> paths
	    = check_console_output( "adb shell ls -d /data/local/tmp/*.tflite" );
	for( auto& p: paths )
		check_file_exists_on_device( p );
	return paths;
}

std::vector<std::string> atop::get_models_on_device( atop::Frameworks fr )
{
	switch( fr )
	{
		case atop::Frameworks::mlperf:
		case atop::Frameworks::SNPE:
			throw atop::util::NotImplementedException(
			    fmt::format( "Framework {0} not yet implemented",
			                 atop::framework2string( fr ) ) );
		case atop::Frameworks::tflite:
			atop::check_tflite_reqs();
			return atop::get_tflite_models();
	}

	throw std::logic_error( fmt::format( "Framework {0} not supported",
	                                     atop::framework2string( fr ) ) );
}

void atop::run_tflite_benchmark( std::vector<std::string> model_paths,
                                 std::map<std::string, std::string> options,
                                 int processes )
{
	// The user is unlikely to spawn more than
	// 8 threads simultaneously (unless this function
	// is ever exposed to the command line)
	static ctpl::thread_pool pool( 8 );

	static atop::util::RandomSelector rselect{};
	std::stringstream base_cmd;
	base_cmd << "/data/local/tmp/benchmark_model";
	for( auto&& p: options )
	{
		base_cmd << " ";
		base_cmd << fmt::format( "--{0}={1}", p.first, p.second );
	}

	std::string base_cmd_str{base_cmd.str()};
	std::stringstream cmd;
	cmd << base_cmd_str;
	for( int i = 0; i < processes; ++i )
	{
		cmd << fmt::format( " --graph={0} & ", rselect( model_paths ) );
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
	    fmt::format( "Running tflite benchmark using: {0}", cmd.str() ) );

	auto cmd_str = cmd.str();
	[[maybe_unused]] std::future<void> th_queue
	    = pool.push( [&cmd_str]( int ) { check_adb_shell_output( cmd_str ); } );
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
		// Ignore the string label of the row (e.g., "cpu0" in "cpu0 123 123..")
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
		    = ( ( this->del_total_tick[i] - this->del_idle[i] )
		        / static_cast<double>( this->del_total_tick[i] ) )
		      * 100;
	}

	return this->latest_utils;
}
