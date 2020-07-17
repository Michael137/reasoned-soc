#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include <re2/re2.h>

#include "util.h"

// trim from start (in place)
void atop::util::ltrim( std::string& s )
{
	s.erase( s.begin(),
	         std::find_if( s.begin(), s.end(), []( int ch ) { return !std::isspace( ch ); } ) );
}

// trim from end (in place)
void atop::util::rtrim( std::string& s )
{
	s.erase(
	    std::find_if( s.rbegin(), s.rend(), []( int ch ) { return !std::isspace( ch ); } ).base(),
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

bool atop::util::regex_find( RE2 const& pattern, std::string_view str,
               std::vector<std::string>& results )
{
    if(!pattern.ok())
        throw std::runtime_error(std::string(__FUNCTION__) + ": invalid pattern provided");

    std::vector<RE2::Arg> args;
    std::vector<RE2::Arg*> arg_ptrs;

    // Since we checked that pattern is ok the
    // cast below is safe
    std::size_t args_count = static_cast<std::size_t>(pattern.NumberOfCapturingGroups());

    args.resize( args_count );
    arg_ptrs.resize( args_count );
    results.resize( args_count );

    for( std::size_t i = 0; i < args_count; ++i )
    {
        args[i] = &results[i];
        arg_ptrs[i] = &args[i];
    }

    re2::StringPiece piece( str );
    return RE2::FindAndConsumeN( &piece, pattern, arg_ptrs.data(),
                                 static_cast<int>(args_count) );
}

std::string atop::util::basepath( std::string const& file_path )
{
	char sep = '/';

	size_t i = file_path.rfind( sep, file_path.length() );
	if( i != std::string::npos )
		return ( file_path.substr( 0, i ) );

	return "";
}
