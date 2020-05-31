#ifndef UTIL_H_IN
#define UTIL_H_IN

#include <string>

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
	explicit NotImplementedException(std::string const& msg): logic_error(msg) {}
	NotImplementedException(): logic_error("Function not yet implemented") {}

	virtual char const* what() const noexcept override { return "Function not yet implemented."; }
};

} // namespace util
} // namespace atop

#endif // UTIl_H_IN
