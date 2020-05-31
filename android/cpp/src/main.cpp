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

using namespace std::chrono_literals;

static constexpr auto USAGE =
    R"(atop - accelerator viewer
	  Usage:
			atop [options]

	  Options:
	  		-h, --help        Show usage
			-v, --verbose     Verbose outputs [default: false]
			-b, --benchmark   Run sample workload [default: false]
			-p N, --procs=N   How many processes to spawn for sample workload (requires -b) [default: 1]
			-g, --gpu         Run sample workload on GPU only (requires -b) [default: false]
			-n, --nnapi       Run sample workload using NNAPI delegate (requires -b) [default: false]
			-d, --dsp         Run sample workload on DSP only (requires -b) [default: true]
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

	VERBOSE              = args["--verbose"].asBool();
	const bool benchmark = args["--benchmark"].asBool();
	const long procs     = args["--procs"].asLong();
	const bool on_gpu    = args["--gpu"].asBool();
	const bool on_nnapi  = args["--nnapi"].asBool();
	const bool on_dsp    = args["--dsp"].asBool();
	const bool sim       = args["--sim"].asBool();

	if( ( on_gpu || on_nnapi || !on_dsp || ( procs != 1 ) ) && benchmark )
	{
		spdlog::error( "Specified --proc, --gpu, --nnapi or --dsp without the "
		               "--benchmark flag" );
		std::abort();
	}

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
	// TODO: should be populated from path on device which in turn is derived
	// from chosen framework
	static std::vector<std::pair<std::string, bool>> models
	    = {{"PLACEHOLDER1", false},
	       {"PLACEHOLDER2", false},
	       {"PLACEHOLDER3", false}};

	static int num_procs = 1;

	constexpr auto streamer_refresh_rate = 2s;
	atop::IoctlDmesgStreamer streamer;
	auto timer_prev = std::chrono::system_clock::now();
	auto timer_cur  = timer_prev;
	auto data       = streamer.get_interactions();

	float stream_latency = 0.0;

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
		if( std::chrono::duration_cast<std::chrono::seconds>( timer_cur
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

		ImGui::Checkbox( "Verbose (stdout)", &VERBOSE );

		ImGui::Button( "Pause" );

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

		auto cur_max  = std::max_element( std::begin( interactions ),
                                         std::end( interactions ) );
		int num_ticks = 0;
		for( size_t i = 0; i < interactions.size(); ++i )
		{
			float scale = static_cast<float>( interactions[i] )
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
		ImGui::RadioButton( "Hexagon DSP", &delegate_rb, 0 );
		ImGui::SameLine();
		ImGui::RadioButton( "GPU", &delegate_rb, 1 );
		ImGui::SameLine();
		ImGui::RadioButton( "NNAPI", &delegate_rb, 2 );

		ComboBox( "Framework", &sel_framework, frameworks );

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

		ImGui::Button( "Run" );
		ImGui::End();

		window.clear();
		ImGui::SFML::Render( window );
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}
