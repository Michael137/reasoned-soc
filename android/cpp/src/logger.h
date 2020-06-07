#ifndef LOGGER_H_IN
#define LOGGER_H_IN

#include <spdlog/spdlog.h>

namespace atop
{
namespace logger
{
inline void log_and_exit( std::string const& msg )
{
	spdlog::error( msg );
	std::abort();
}

inline void info_if( std::string const& msg, bool cond )
{
	if( cond )
		spdlog::info( msg );
}

inline void verbose_info( std::string const& msg ) { info_if( msg, VERBOSE ); }

inline void warn( std::string const& msg ) { spdlog::warn( msg ); }

} // namespace logger
} // namespace atop

#define LOG( _msg_ ) atop::logger::verbose_info( _msg_ )

#endif // LOGGER_H_IN
