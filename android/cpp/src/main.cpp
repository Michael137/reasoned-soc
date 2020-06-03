#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <unordered_map>
#include <utility>

#include <docopt/docopt.h>
#include <imgui-SFML.h>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "atop.h"
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

bool ListBox( const char* label, int* currIndex,
              std::vector<std::string>& values )
{
	if( values.empty() )
	{
		return false;
	}
	return ImGui::ListBox( label, currIndex, vector_getter,
	                       static_cast<void*>( &values ),
	                       static_cast<int>( values.size() ) );
}

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
init_imgui_models_vec( std::vector<std::string> const& models )
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

	sf::RenderWindow window( sf::VideoMode( 2560, 1920 ),
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

	// TODO: is models + selected_models a better structure than the embedded
	// boolean?
	static std::vector<std::pair<std::string, bool>> models{
	    init_imgui_models_vec(
	        atop::get_models_on_device( atop::string2framework(
	            frameworks[static_cast<size_t>( sel_framework )] ) ) )};

	static int num_procs       = 1;
	static int num_runs        = 1;
	static int num_warmup_runs = 0;

	static bool utilization_paused = false;
	static bool cpu_fallback       = false;

	constexpr auto streamer_refresh_rate = 2s;
	atop::IoctlDmesgStreamer streamer;
	std::chrono::time_point<std::chrono::system_clock> timer_prev
	    = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> timer_cur;

	auto data = streamer.get_interactions();

	float stream_latency = 0.0;

	atop::logger::verbose_info( "Finished initializtion" );

	static bool timer_win_b = true;
	sf::Clock deltaClock;
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
			}
		}

		timer_cur = std::chrono::system_clock::now();
		if( !utilization_paused
		    && std::chrono::duration_cast<std::chrono::seconds>( timer_cur
		                                                         - timer_prev )
		               .count()
		           >= streamer_refresh_rate.count() )
		{
			auto start = std::chrono::system_clock::now();
			data       = streamer.interactions( true );
			timer_prev = timer_cur;
			auto end   = std::chrono::system_clock::now();

			// TODO: measure latency within streamer class
			stream_latency
			    = static_cast<float>(
			          std::chrono::duration_cast<std::chrono::milliseconds>(
			              end - start )
			              .count() )
			      / 1000;
		}

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
		std::string latency_text
		    = fmt::format( "Stream latency: {0} s", stream_latency );
		ImGui::TextUnformatted( latency_text.c_str() );
		ImGui::End();

		/* Options window */
		ImGui::Begin( "Options" );

		// Data source
		ImGui::RadioButton( "dmesg", &data_src_rb, 0 );
		ImGui::SameLine();
		ImGui::RadioButton( "perf", &data_src_rb, 1 );

		// Utilization measure
		ImGui::RadioButton( "interaction", &util_rb, 0 );
		ImGui::SameLine();
		ImGui::RadioButton( "timing", &util_rb, 1 );

		if( ImGui::Checkbox( "Verbose (stdout)", &VERBOSE ) )
		{
			atop::logger::verbose_info( "Verbose toggled on" );
		}

		ImGui::Button( "Pause" );
		utilization_paused ^= true;
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

		for( auto& p: data )
		{
			labels.push_back( p.first );
			interactions.push_back( p.second );

			// std::cout << p.first << " " << p.second << std::endl;
		}

		auto cur_max = std::max_element( std::begin( interactions ),
		                                 std::end( interactions ) );
		for( size_t i = 0; i < interactions.size(); ++i )
		{
			int num_ticks = 0;
			float scale   = static_cast<float>( interactions[i] )
			              / static_cast<float>( *cur_max );
			// TODO: handle scaling
			float scaled = ( win_width * scale
			                 / static_cast<float>( font_scale_factor ) )
			               / 8;
			if( scaled > 0 )
				scaled
				    = std::max( 1, static_cast<int>( std::floor( scaled ) ) );

			num_ticks = static_cast<int>( scaled );
			// std::cout << scale << " " << scaled << " " << num_ticks <<
			// std::endl;

			std::stringstream ss;
			for( int j = 0; j < num_ticks; ++j )
				ss << '#';
			ImGui::TextUnformatted(
			    fmt::format( "{0}| {1}", labels[i], ss.str() ).c_str() );
		}

		ImGui::End();

		ImGui::Begin( "Workload Simulator" );

		if( ComboBox( "Framework", &sel_framework, frameworks ) )
		{
			atop::logger::verbose_info( fmt::format(
			    "Changed model framework to: {0}",
			    frameworks[static_cast<size_t>( sel_framework )] ) );

			// TODO: populate models
		}

		if( ImGui::ListBoxHeader( "Models" ) )
		{
			for( size_t n = 0; n < models.size(); n++ )
			{
				if( ImGui::Selectable( models[n].first.c_str(),
				                       models[n].second ) )
					models[n].second ^= true;
			}

			ImGui::ListBoxFooter();
		}

		ImGui::InputInt( "Processes", &num_procs );

		if( ImGui::Button( "Run" ) )
		{
			atop::run_tflite_benchmark(
			    unzip_imgui_models( models ),
			    {{"num_threads", std::to_string( num_procs )},
			     {"warmup_runs", std::to_string( num_warmup_runs )},
			     {"num_runs", std::to_string( num_runs )},
			     {"hexagon_profiling", "false"},
			     {"enable_op_profiling", "false"},
			     {"disable_nnapi_cpu", atop::util::bool2string( cpu_fallback )},
			     {"use_hexagon", atop::util::bool2string( delegate_rb == 0 )},
			     {"use_nnapi", atop::util::bool2string( delegate_rb == 1 )},
			     {"use_gpu", atop::util::bool2string( delegate_rb == 2 )}},
			    num_procs );
			ImGui::End();
		}

		// TODO: this should change depending on framework used
		ImGui::Begin( "Benchmark Options" );
		ImGui::RadioButton( "Hexagon DSP", &delegate_rb, 0 );
		ImGui::SameLine();
		ImGui::RadioButton( "GPU", &delegate_rb, 1 );
		ImGui::SameLine();
		ImGui::RadioButton( "NNAPI", &delegate_rb, 2 );
		ImGui::Checkbox( "w/ CPU Fallback", &cpu_fallback );
		ImGui::InputInt( "Runs", &num_runs );
		ImGui::InputInt( "Warmup Runs", &num_warmup_runs );

		window.clear();
		ImGui::SFML::Render( window );
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}
