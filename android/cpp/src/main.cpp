#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>

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

	sf::Clock deltaClock;
	while( window.isOpen() )
	{
		sf::Event event;
		while( window.pollEvent( event ) )
		{
			ImGui::SFML::ProcessEvent( event );

			if( event.type == sf::Event::Closed )
			{
				atop::logger::verbose_info( "Window close requested...exiting" );
				window.close();
			}
		}

		ImGui::SFML::Update( window, deltaClock.restart() );

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
		ImGui::End();

		window.clear();
		ImGui::SFML::Render( window );
		window.display();
	}

	ImGui::SFML::Shutdown();

	return 0;
}
