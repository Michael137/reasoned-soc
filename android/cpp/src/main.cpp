#include <functional>
#include <iostream>

#include <spdlog/spdlog.h>

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <iostream>

int main( [[maybe_unused]] int argc, [[maybe_unused]] const char** argv )
{
	spdlog::info( "Starting atop" );

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
