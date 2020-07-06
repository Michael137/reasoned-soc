#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <sstream>
#include <thread>
#include <utility>

#include <docopt/docopt.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <spdlog/spdlog.h>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "atop.h"
#include "fifo.h"
#include "logger.h"

static auto vector_getter = []( void* vec, int idx, const char** out_text ) {
	auto& vector = *static_cast<std::vector<std::string>*>( vec );
	if( idx < 0 || idx >= static_cast<int>( vector.size() ) )
	{
		return false;
	}
	*out_text = vector.at( static_cast<size_t>( idx ) ).c_str();
	return true;
};

bool ComboBox( const char* label, int* currIndex,
               std::vector<std::string>& values )
{
	if( values.empty() )
	{
		return false;
	}
	return ImGui::Combo( label, currIndex, vector_getter,
	                     static_cast<void*>( &values ),
	                     static_cast<int>( values.size() ) );
}

static std::vector<std::pair<std::string, bool>>
imgui_models_vec( std::vector<std::string> const& models )
{
	std::vector<std::pair<std::string, bool>> result;
	result.reserve( models.size() );
	std::for_each( begin( models ), end( models ),
	               [&result]( std::string const& m ) {
		               result.push_back( std::make_pair( m, true ) );
	               } );
	return result;
}

std::vector<std::string>
unzip_imgui_models( std::vector<std::pair<std::string, bool>> const& vec )
{
	std::vector<std::string> res;
	std::for_each( vec.begin(), vec.end(),
	               [&]( std::pair<std::string, bool> const& p ) {
		               if( p.second )
			               res.push_back( p.first );
	               } );

	return res;
}

static std::string adb_getprop(std::string_view prop)
{
  auto output = atop::check_console_output(fmt::format("adb shell getprop {0}", prop));
  if(output.size() > 0)
    return output[0];
  else
    return "";
}

static void adb_setprop(std::string_view prop, std::string prop_val)
{
  atop::check_console_output(fmt::format("adb shell setprop {0} {1}", prop, prop_val));
}

static void toggle_driver_logging()
{
  std::string prop = "debug.nn.vlog";
  auto output = adb_getprop(prop);
  int prop_val = 0;
  if(!output.empty())
      prop_val = std::stoi(output);

  // Toggle
  int new_val = 1 - prop_val;
  LOG(fmt::format("Setting {0} from {1} to {2}", prop, prop_val, new_val));
  adb_setprop(prop, std::to_string(new_val));
}

// Until is_ready() is in the C++ standard use this to check
// whether a std::future result is ready
template<typename R> bool is_ready( std::future<R> const& f )
{
	return f.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;
}

using namespace std::chrono_literals;

static constexpr auto USAGE =
    R"(atop - accelerator viewer
	  Usage:
			atop [options]

	  Options:
	  		-h, --help        Show usage
			-v, --verbose     Verbose outputs [default: false]
			-s, --sim         Run without device [default: false]
			-V, --version     Show version
		)";

static constexpr auto VERSION_STRING = "atop - Version 0.1";

bool VERBOSE{false};

// From ImGui Log example
struct DmesgLog
{
	ImGuiTextBuffer Buf;
	ImGuiTextFilter Filter;
	ImVector<int> LineOffsets; // Index to lines offset. We maintain this with
	                           // AddLog() calls.
	bool AutoScroll;           // Keep scrolling if already at the bottom.

	DmesgLog()
	{
		AutoScroll = true;
		Clear();
	}

	void Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back( 0 );
	}

	void AddLog( const char* fmt, ... ) IM_FMTARGS( 2 )
	{
		int old_size = Buf.size();
		va_list args;
		va_start( args, fmt );
		Buf.appendfv( fmt, args );
		va_end( args );
		for( int new_size = Buf.size(); old_size < new_size; old_size++ )
			if( Buf[old_size] == '\n' )
				LineOffsets.push_back( old_size + 1 );
	}

	void Draw( const char* title, bool* p_open = NULL )
	{
		if( !ImGui::Begin( title, p_open ) )
		{
			ImGui::End();
			return;
		}

		// Options menu
		if( ImGui::BeginPopup( "Options" ) )
		{
			ImGui::Checkbox( "Auto-scroll", &AutoScroll );
			ImGui::EndPopup();
		}

		// Main window
		if( ImGui::Button( "Options" ) )
			ImGui::OpenPopup( "Options" );
		ImGui::SameLine();
		bool clear = ImGui::Button( "Clear" );
		ImGui::SameLine();
		bool copy = ImGui::Button( "Copy" );
		ImGui::SameLine();
		Filter.Draw( "Filter", -150.0f );

		ImGui::Separator();
		ImGui::BeginChild( "scrolling", ImVec2( 0, 0 ), false,
		                   ImGuiWindowFlags_HorizontalScrollbar );

		if( clear )
			Clear();
		if( copy )
			ImGui::LogToClipboard();

		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );
		const char* buf     = Buf.begin();
		const char* buf_end = Buf.end();
		if( Filter.IsActive() )
		{
			for( int line_no = 0; line_no < LineOffsets.Size; line_no++ )
			{
				const char* line_start = buf + LineOffsets[line_no];
				const char* line_end
				    = ( line_no + 1 < LineOffsets.Size )
				          ? ( buf + LineOffsets[line_no + 1] - 1 )
				          : buf_end;
				if( Filter.PassFilter( line_start, line_end ) )
					ImGui::TextUnformatted( line_start, line_end );
			}
		}
		else
		{
			ImGuiListClipper clipper;
			clipper.Begin( LineOffsets.Size );
			while( clipper.Step() )
			{
				for( int line_no = clipper.DisplayStart;
				     line_no < clipper.DisplayEnd; line_no++ )
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end
					    = ( line_no + 1 < LineOffsets.Size )
					          ? ( buf + LineOffsets[line_no + 1] - 1 )
					          : buf_end;
					ImGui::TextUnformatted( line_start, line_end );
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if( AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() )
			ImGui::SetScrollHereY( 1.0f );

		ImGui::EndChild();
		ImGui::End();
	}
};

static void ShowBenchmarkSummary( bool* popen,
                                  atop::BenchmarkStats const& stats )
{
	ImGui::Begin( "Benchmark Summary", popen );
	for( auto&& p: stats.stats )
	{
		ImGui::TextUnformatted(
		    fmt::format( "{0}: {1} ms", p.first,
		                 static_cast<double>( p.second ) / 1.0e3 )
		        .c_str() );
	}
	ImGui::End();
}

static void ShowIoctlBreakdown(
    std::map<std::string, std::map<std::string, int>> const& breakdown,
    int num_runs )
{
	ImGui::Begin( "Breakdown" );
	for( auto&& p: breakdown )
	{
		ImGui::TextUnformatted( fmt::format( "{0}: ", p.first ).c_str() );
		for( auto&& cmd_p: p.second )
		{
			ImGui::TextUnformatted(
			    fmt::format( "\t{0}: {1}", cmd_p.first,
			                 static_cast<float>( cmd_p.second ) / num_runs )
			        .c_str() );
		}
	}
	ImGui::End();
}

int main( int argc, const char** argv )
{
	std::map<std::string, docopt::value> args = docopt::docopt(
	    USAGE, {std::next( argv ), std::next( argv, argc )},
	    true /* show if help is requested */, VERSION_STRING );

	VERBOSE        = args["--verbose"].asBool();
	const bool sim = args["--sim"].asBool();

	if( !sim )
		atop::check_reqs();

	atop::logger::verbose_info( "Starting atop" );

	sf::RenderWindow window(
	    sf::VideoMode( sf::VideoMode::getDesktopMode().width,
	                   sf::VideoMode::getDesktopMode().height ),
	    "atop - accelerator viewer" );
	window.setFramerateLimit( 60 );
	ImGui::SFML::Init( window );

	constexpr auto window_scale_factor = 1.0;
	constexpr auto font_scale_factor   = 3.0;
	ImGui::GetStyle().ScaleAllSizes( window_scale_factor );
	ImGui::GetIO().FontGlobalScale = font_scale_factor;

	static int data_src_rb = 0;
	static int util_rb     = 0;

	static int delegate_rb                     = 0;
	static int sel_framework                   = 0;
	static std::vector<std::string> frameworks = {"tflite", "mlperf", "SNPE"};

	// TODO: is models + selected_models a better structure than the
	// embedded boolean?
	static std::vector<std::pair<std::string, bool>> models{
	    imgui_models_vec( atop::get_models_on_device( atop::string2framework(
	        frameworks[static_cast<size_t>( sel_framework )] ) ) )};

	static int num_procs       = 1;
	static int num_runs        = 1;
	static int num_cpu_threads = 6; // Snapdragon has 6 CPUs
	static int num_warmup_runs = 0;

	static bool utilization_paused = false;
	static bool cpu_fallback       = false;
	static bool fixed_scale        = true;
	static bool show_log_b         = false;
	static bool bench_summary_cb   = true;

	static atop::BenchmarkStats bench_summary;
	static std::map<std::string, std::map<std::string, int>> ioctl_breakdown;

	atop::IoctlDmesgStreamer streamer;

	// TODO: cpu utilization can be refreshed more often
	atop::CpuUtilizationStreamer cpu_streamer;

	auto data = streamer.get_interactions();
	std::map<std::string, double> cpu_data;

	static std::map<std::string, int> max_interactions{};

	static DmesgLog log;

	static bool timer_win_b = true;

	static std::queue<std::future<atop::shell_out_t>> benchmark_futures_q;

	static int workloads_running = 0;

	static bool data_got_consumed = false;

	static std::atomic<bool> exiting = false;

        std::string old_driver_logging_prop = adb_getprop("debug.nn.vlog");
        static bool driver_logging = (old_driver_logging_prop == "1") ? true : false;

	sf::Clock deltaClock;

	atop::logger::verbose_info( "Finished initialization" );

	atop::fifo::FIFO<std::map<std::string, int>> ioctl_dmesg_fifo;
	auto stream_ioctl_dmesg = [&]() {
		while( !exiting )
		{
			ioctl_dmesg_fifo.push_data(
			    streamer.interactions( true /* check full log */ ) );

			// As long as refresh rate is higher than stream latency
			std::this_thread::sleep_for( 2s );
		}
	};
	std::thread streamer_th{stream_ioctl_dmesg};

	atop::fifo::FIFO<std::map<std::string, double>> cpu_fifo;
	auto stream_cpu = [&]() {
		while( !exiting )
		{
			cpu_fifo.push_data( cpu_streamer.utilizations() );
			std::this_thread::sleep_for( 1s );
		}
	};
	std::thread cpu_th{stream_cpu};

	while( window.isOpen() )
	{
		sf::Event event;
		while( window.pollEvent( event ) )
		{
			ImGui::SFML::ProcessEvent( event );

			if( event.type == sf::Event::Closed )
			{
				atop::logger::verbose_info(
				    "Window close requested...exiting" );
				window.close();
                                exiting = true;
			}
		}

		if( !utilization_paused && ioctl_dmesg_fifo.data_avail() )
		{
			data             = ioctl_dmesg_fifo.pop_data();
			data_got_consumed = false;
		}

		if( !utilization_paused && cpu_fifo.data_avail() )
			// TODO: measure stream CPU latency
			cpu_data = cpu_fifo.pop_data();

		ImGui::SFML::Update( window, deltaClock.restart() );

		ImGui::SetNextWindowPos( ImVec2( 0.0f, ImGui::GetIO().DisplaySize.y ),
		                         0, ImVec2( 0.0f, 1.0f ) );
		ImGui::SetNextWindowSize( ImVec2( 550.0f, 100.0f ), 0 );
		ImGui::Begin( "Stream latency", &timer_win_b,
		              ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse
		                  | ImGuiWindowFlags_NoMove
		                  | ImGuiWindowFlags_NoBringToFrontOnFocus
		                  | ImGuiWindowFlags_NoTitleBar
		                  | ImGuiWindowFlags_MenuBar );
		std::string latency_text = fmt::format(
		    "Stream latency: {0} s",
		    streamer.get_duration() / static_cast<float>( 1.0e3 ) );
		ImGui::TextUnformatted( latency_text.c_str() );
		ImGui::End();

		/* Options window */
		ImGui::Begin( "Options" );

		// Data source
		ImGui::Text( "Source:" );
		ImGui::SameLine();
		ImGui::RadioButton( "dmesg", &data_src_rb, 0 );
		ImGui::SameLine();
		ImGui::RadioButton( "perf", &data_src_rb, 1 );

		// Utilization measure
		ImGui::Text( "Util. Type:" );
		ImGui::SameLine();
		ImGui::RadioButton( "interaction", &util_rb, 0 );
		ImGui::SameLine();
		ImGui::RadioButton( "timing", &util_rb, 1 );

		if( ImGui::Checkbox( "Fixed Scaling", &fixed_scale ) )
			atop::logger::verbose_info( "Fixed scaling toggled" );
		ImGui::SameLine();
		if( ImGui::Checkbox( "Verbose (stdout)", &VERBOSE ) )
			atop::logger::verbose_info( "Verbose toggled on" );

		if( ImGui::Button( ( utilization_paused ) ? "Resume" : "Pause" ) )
		{
			atop::logger::verbose_info(
			    fmt::format( "{0} data stream",
			                 ( utilization_paused ) ? "Resumed" : "Paused" ) );
			utilization_paused ^= true;
		}

		ImGui::End();

		/* Utilization window */
		ImGui::Begin( "Utilization" );

		auto win_size  = ImGui::GetWindowSize();
		auto win_width = win_size.x;
		// auto win_height = win_size.y;
		std::vector<int> interactions;
		interactions.reserve( data.size() );
		std::vector<std::string> labels;
		labels.reserve( data.size() );

		if( data.size() || cpu_data.size() )
		{
			for( auto& p: data )
			{
				labels.push_back( p.first );
				interactions.push_back( p.second );
			}

			// TODO: separate discrete cpu stream vs. floating
			for( auto&& kv: cpu_data )
			{
				labels.push_back( kv.first );
				interactions.push_back( static_cast<int>( kv.second ) );
			}

			auto cur_max = std::max_element( std::begin( interactions ),
			                                 std::end( interactions ) );
			auto max_label_sz
			    = std::max_element(
			          std::begin( labels ), std::end( labels ),
			          [&]( std::string const& s1, std::string const& s2 ) {
				          return s1.size() < s2.size();
			          } )
			          ->size();

			for( size_t i = 0; i < interactions.size(); ++i )
			{
				double scale   = 0.0;
				auto cur_label = labels[i];
				if( fixed_scale )
				{
					auto it = max_interactions.find( cur_label );
					if( it == max_interactions.end() )
						max_interactions[cur_label] = interactions[i];
					int max_ = std::max( max_interactions[cur_label],
					                     interactions[i] );

					scale = static_cast<double>( interactions[i] )
					        / static_cast<double>( max_ );

					max_interactions[cur_label] = max_;
				}
				else
				{
					scale = static_cast<double>( interactions[i] )
					        / static_cast<double>( *cur_max );
				}

				auto num_ticks = ( static_cast<double>( win_width ) * scale
				                   / font_scale_factor )
				                 / 8.0; // TODO: handle scaling
				if( num_ticks > 0.0 )
					num_ticks = std::max( 1.0, std::floor( num_ticks ) );

				std::stringstream bar_padding{""};
				auto label_sz = cur_label.size();
				for( size_t j = 0; j < max_label_sz - label_sz; ++j )
					bar_padding << ' ';

				std::stringstream ss;
				for( int j = 0; j < static_cast<int>( num_ticks ); ++j )
					ss << '#';
				ImGui::TextUnformatted( fmt::format( "{0}{1}| {2}", cur_label,
				                                     bar_padding.str(),
				                                     ss.str() )
				                            .c_str() );
			}
		}

		ImGui::End();

		if( streamer.is_data_fresh && !data_got_consumed )
		{
			atop::ioctl_breakdown(
			    ioctl_breakdown,
			    streamer.get_data()[PROBE_IDX( atop::DmesgProbes::IOCTL )],
			    atop::DmesgProbes::IOCTL );
			atop::ioctl_breakdown(
			    ioctl_breakdown,
			    streamer.get_data()[PROBE_IDX( atop::DmesgProbes::INFO )],
			    atop::DmesgProbes::INFO );
		}
		ShowIoctlBreakdown( ioctl_breakdown, num_runs );

		ShowBenchmarkSummary( &bench_summary_cb, bench_summary );

		if( benchmark_futures_q.size() > 0
		    && is_ready<atop::shell_out_t>( benchmark_futures_q.front() ) )
		{
			if( bench_summary_cb )
			{
				atop::shell_out_t benchmark_out
				    = benchmark_futures_q.front().get();

				atop::summarize_benchmark_output(
				    benchmark_out,
				    atop::string2framework(
				        frameworks[static_cast<size_t>( sel_framework )] ),
				    bench_summary );
			}

			benchmark_futures_q.pop();
			workloads_running--;
			atop::logger::verbose_info( "Oldest task finished" );
		}

		ImGui::Begin( "Workload Simulator" );

		if( ComboBox( "Framework", &sel_framework, frameworks ) )
		{
			atop::logger::verbose_info( fmt::format(
			    "Changed model framework to: {0}",
			    frameworks[static_cast<size_t>( sel_framework )] ) );

			models = imgui_models_vec(
			    atop::get_models_on_device( atop::string2framework(
			        frameworks[static_cast<size_t>( sel_framework )] ) ) );

			// Reset delegate because not all frameworks support all
			// delegates
			delegate_rb = 0;
		}

		// tflite benchmark framework only delegates quantized models to
		// Hexagon DSPs
		bool tflite_hexagon_selected
		    = ( frameworks[static_cast<size_t>( sel_framework )]
		        == atop::framework2string( atop::Frameworks::tflite ) )
		      && ( delegate_rb == 0 /* hexagon dsp delegate */ || delegate_rb == 4 /* qti-dsp */ );

		if( ImGui::ListBoxHeader( "Models" ) )
		{
			for( size_t n = 0; n < models.size(); n++ )
			{
				bool disable_model = tflite_hexagon_selected
				                     && ( models[n].first.find( "_quant" )
				                          == std::string::npos );

				if( disable_model )
				{
					ImGui::PushItemFlag( ImGuiItemFlags_Disabled, true );
					ImGui::PushStyleVar( ImGuiStyleVar_Alpha,
					                     ImGui::GetStyle().Alpha * 0.5f );
					models[n].second = false;
				}

				if( ImGui::Selectable( models[n].first.c_str(),
				                       models[n].second ) )
					models[n].second ^= true;

				if( disable_model )
				{
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
			}

			ImGui::ListBoxFooter();
		}

		if( ImGui::Button( "Select All" ) )
			for( size_t n = 0; n < models.size(); n++ )
				models[n].second = true;
		ImGui::SameLine();
		if( ImGui::Button( "De-select All" ) )
			for( size_t n = 0; n < models.size(); n++ )
				models[n].second = false;
		ImGui::SameLine();
		ImGui::Checkbox( "Summarize", &bench_summary_cb );

		ImGui::InputInt( "Processes", &num_procs );

		if( ImGui::Button( "Run" ) )
		{
			ioctl_breakdown.clear();
			switch( atop::string2framework(
			    frameworks[static_cast<size_t>( sel_framework )] ) )
			{
				case atop::Frameworks::tflite:
				{
					std::string nnapi_accelerator_name
					    = ( delegate_rb == 4 )
					          ? "qti-dsp"
					          : ( ( delegate_rb == 5 )
					                  ? "qti-gpu"
					                  : ( ( delegate_rb == 6 ) ? "paintbox"
					                                           : "" ) );
					bool use_nnapi
					    = ( delegate_rb == 2 || delegate_rb == 4
					        || delegate_rb == 5 || delegate_rb == 6 );

					// TODO: tflite benchmark tool has undocumented CSV
					// export flag "profiling_output_csv_file". Requires
					// enable_op_profiling
					benchmark_futures_q.emplace( atop::run_tflite_benchmark(
					    unzip_imgui_models( models ),
					    {
					        {"num_threads", std::to_string( num_cpu_threads )},
					        {"warmup_runs", std::to_string( num_warmup_runs )},
					        {"num_runs", std::to_string( num_runs )},
					        {"hexagon_profiling", "false"},
					        {"enable_op_profiling", "false"},
					        // cpu fallback false => disable nnapi cpu true
					        {"disable_nnapi_cpu",
					         atop::util::bool2string( !cpu_fallback
					                                  && use_nnapi )},
                                                {"require_full_delegation",
                                                  atop::util::bool2string(!cpu_fallback)},
					        {"use_hexagon",
					         atop::util::bool2string( delegate_rb == 0 )},
					        {"use_gpu",
					         atop::util::bool2string( delegate_rb == 1 )},
					        {"use_nnapi", atop::util::bool2string( use_nnapi )},
					        {"nnapi_accelerator_name", nnapi_accelerator_name},
                                                {"time_driver", atop::util::bool2string(driver_logging)},
					    },
					    num_procs ) );
				}
				break;
				case atop::Frameworks::SNPE:
				{
					std::map<std::string, std::string> opts
					    = {{"profiling_level", "detailed"},
					       {"perf_profile", "high_performance"}};
					if( delegate_rb == 0 )
						opts.insert( {"use_dsp", ""} );
					else if( delegate_rb == 1 )
						opts.insert( {"use_gpu", ""} );

					if( cpu_fallback )
						opts.insert( {"enable_cpu_fallback", ""} );

					benchmark_futures_q.emplace(
					    atop::run_snpe_benchmark( unzip_imgui_models( models ),
					                              opts, num_procs, num_runs ) );
				}
				break;
				case atop::Frameworks::mlperf:
					throw atop::util::NotImplementedException(
					    "mlperf benchmarks not yet implemented" );
			};

			workloads_running++;

			ImGui::End();
		}
		// TODO: warn user if he schedules more thean thread pool size since
		//       current thread pool queue manager waits for a task to
		//       finish before removing a task
		ImGui::SameLine();
		ImGui::TextUnformatted(
		    fmt::format( "{0} running", workloads_running ).c_str() );

		ImGui::Begin( "Benchmark Options" );
		switch( atop::string2framework(
		    frameworks[static_cast<size_t>( sel_framework )] ) )
		{
			case atop::Frameworks::tflite:
				ImGui::RadioButton( "Hexagon DSP", &delegate_rb, 0 );
				ImGui::SameLine();
				ImGui::RadioButton( "GPU", &delegate_rb, 1 );
				ImGui::SameLine();
				ImGui::RadioButton( "NNAPI", &delegate_rb, 2 );
				ImGui::SameLine();
				ImGui::RadioButton( "CPU Only", &delegate_rb, 3 );

				ImGui::RadioButton( "nnapi-dsp", &delegate_rb, 4 );
				ImGui::SameLine();
				ImGui::RadioButton( "nnapi-gpu", &delegate_rb, 5 );
				ImGui::SameLine();
				ImGui::RadioButton( "nnapi-paintbox", &delegate_rb, 6 );

				ImGui::Checkbox( "w/ CPU Fallback", &cpu_fallback );
                                ImGui::SameLine();
                                if(ImGui::Checkbox( "w/ Driver Inst.", &driver_logging ))
                                  toggle_driver_logging();
				ImGui::InputInt( "Runs", &num_runs );
				ImGui::InputInt( "Warmup Runs", &num_warmup_runs );
				ImGui::InputInt( "CPU Threads", &num_cpu_threads );
				break;
			case atop::Frameworks::SNPE:
				ImGui::RadioButton( "Hexagon DSP", &delegate_rb, 0 );
				ImGui::SameLine();
				ImGui::RadioButton( "GPU", &delegate_rb, 1 );
				ImGui::SameLine();
				ImGui::RadioButton( "CPU Only", &delegate_rb, 3 );
				ImGui::Checkbox( "w/ CPU Fallback", &cpu_fallback );
				ImGui::InputInt( "Runs", &num_runs );
				ImGui::InputInt( "Warmup Runs", &num_warmup_runs );
				break;
			case atop::Frameworks::mlperf:
				throw atop::util::NotImplementedException(
				    "mlperf benchmarks not yet implemented" );
		}
		ImGui::End();

		// TODO: add option to change log to logcat, stdout, etc.
		ImGui::SetNextWindowSize( ImVec2( 500, 400 ), ImGuiCond_FirstUseEver );
		ImGui::Begin( "Dmesg Log", &show_log_b );
		if( streamer.is_data_fresh && !data_got_consumed )
		{
			auto data_to_log
			    = streamer.get_data()[PROBE_IDX( streamer.utilization_probe )];
			for( auto it = data_to_log.rbegin(); it != data_to_log.rend();
			     ++it )
				log.AddLog( "%s\n", ( *it ).c_str() );
		}
		ImGui::End();

		// Actually call in the regular Log helper (which will Begin() into
		// the same window as we just did)
		log.Draw( "Dmesg Log", &show_log_b );

                // One iteration => data in streamer has been processed
                data_got_consumed = true;

		window.clear();
		ImGui::SFML::Render( window );
		window.display();
	}

	ImGui::SFML::Shutdown();

        streamer_th.join();
        cpu_th.join();

        adb_setprop("debug.nn.vlog", old_driver_logging_prop);

	return 0;
}
