#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <cctype>
#include <locale>
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

std::vector<std::string> atop::util::split( const std::string& s, char delimiter )
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
