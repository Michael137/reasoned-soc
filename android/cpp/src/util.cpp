#include <algorithm>
#include <cctype>
#include <locale>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "util.h"

// trim from start (in place)
void atop::util::ltrim( std::string& s )
{
	s.erase( s.begin(), std::find_if( s.begin(), s.end(), []( int ch ) {
		         return !std::isspace( ch );
	         } ) );
}

// trim from end (in place)
void atop::util::rtrim( std::string& s )
{
	s.erase( std::find_if( s.rbegin(), s.rend(),
	                       []( int ch ) { return !std::isspace( ch ); } )
	             .base(),
	         s.end() );
}

// trim from both ends (in place)
void atop::util::trim( std::string& s )
{
	ltrim( s );
	rtrim( s );
}

std::vector<std::string> atop::util::split( const std::string& s,
                                            char delimiter )
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream( s );
	while( std::getline( tokenStream, token, delimiter ) )
	{
		tokens.push_back( token );
	}
	return tokens;
}

double atop::util::extract_time( std::string const& str )
{
	// Match timestamp s.a. "[ 3633.459327] ..."
	// In dmesg this represents seconds since kernel boot
	std::regex pattern{R"(\[(.*?)\])"};
	std::smatch match;

	if( std::regex_search( str, match, pattern,
	                       std::regex_constants::match_continuous ) )
	{
		std::string ts_str = match[1].str();
		atop::util::trim( ts_str );

		return std::stod( ts_str );
	}
	else
		throw std::runtime_error(
		    "Error during log line parsing: no timestamp found" );
}
