#ifndef UTIL_H_IN
#define UTIL_H_IN

#include <random>
#include <string>

#include <re2/re2.h>

namespace atop
{
namespace util
{
void ltrim( std::string& s );
void rtrim( std::string& s );
void trim( std::string& s );
std::string basepath( std::string const& file_path );

std::vector<std::string> split( const std::string&, char delimiter );

bool regex_find( RE2 const& pattern, std::string_view str,
                           std::vector<std::string>& results );

class NotImplementedException : public std::logic_error
{
  public:
	explicit NotImplementedException( std::string const& msg )
	    : logic_error( msg )
	{
	}
	NotImplementedException()
	    : logic_error( "Function not yet implemented" )
	{
	}

	virtual char const* what() const noexcept override { return "Function not yet implemented."; }
};

class RandomSelector
{
  public:
	RandomSelector()
	    : gen( std::mt19937( std::random_device()() ) )
	{
	}

	template<typename Iter> Iter select( Iter start, Iter end )
	{
		std::uniform_int_distribution<> dis( 0,
		                                     static_cast<int>( std::distance( start, end ) - 1 ) );
		std::advance( start, dis( this->gen ) );
		return start;
	}

	template<typename Container> auto operator()( const Container& c ) -> decltype( *begin( c ) )&
	{
		return *select( begin( c ), end( c ) );
	}

  private:
	std::mt19937 gen;
};

inline std::string bool2string( bool b ) { return b ? "true" : "false"; }
inline bool string2bool( std::string_view b ) { return ( b == "true" || b == "1" ) ? true : false; }

} // namespace util
} // namespace atop

#endif // UTIl_H_IN
