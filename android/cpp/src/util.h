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

std::vector<std::string> split( const std::string& s, char delimiter );

} // namespace util
} // namespace atop

#endif // UTIl_H_IN
