#ifndef ATOP_H_IN
#define ATOP_H_IN

#include <string>

extern bool VERBOSE;

namespace atop
{
void check_reqs();
std::vector<std::string> check_console_output( std::string const& cmd );

} // namespace atop

#endif // ATOP_H_IN
