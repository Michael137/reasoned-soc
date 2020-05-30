#include <array>
#include <cstdlib>
#include <vector>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "atop.h"

void atop::check_console_output() {}

void atop::check_reqs()
{
	// std::array required_bins{"adb"};

	std::vector<std::string> not_found_bins{};

	if( not_found_bins.size() > 0 )
	{
		spdlog::error( "following required binaries not found:" );
		for( auto& bin: not_found_bins )
			spdlog::error( fmt::format( "\t{0}", bin ) );
		std::abort();
	}
}
