#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <iostream>

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "atop.h"
#include "logger.h"
#include "util.h"

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
	return atop::check_console_output("adb shell whoami")[0] == "root";
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
	if(!in_adb_root())
	{
		atop::logger::warn("atop requires adb in root...restarting as root");
		std::system("adb root");
		if(!in_adb_root())
			atop::logger::log_and_exit("Failed to restart adb in root");
	}

	atop::logger::verbose_info(
	    fmt::format( "Found device: {0}", device_name ) );

	// TODO: check adb write permissions
}
