#ifndef UTIL_H_IN
#define UTIL_H_IN

#include <random>
#include <string>
// #include <iterator>

namespace atop
{
namespace util
{
void ltrim( std::string& s );
void rtrim( std::string& s );
void trim( std::string& s );

std::vector<std::string> split( const std::string&, char delimiter );

double extract_time( std::string const& );

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

	virtual char const* what() const noexcept override
	{
		return "Function not yet implemented.";
	}
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
		std::uniform_int_distribution<> dis(
		    0, static_cast<int>( std::distance( start, end ) - 1 ) );
		std::advance( start, dis( this->gen ) );
		return start;
	}

	template<typename Container>
	auto operator()( const Container& c ) -> decltype( *begin( c ) )&
	{
		return *select( begin( c ), end( c ) );
	}

   private:
	std::mt19937 gen;
};

inline std::string bool2string( bool b ) { return b ? "true" : "false"; }

} // namespace util
} // namespace atop

#endif // UTIl_H_IN