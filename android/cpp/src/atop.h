#ifndef ATOP_H_IN
#define ATOP_H_IN

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include <fmt/format.h>

#include "util.h"

extern bool VERBOSE;

namespace atop
{
enum class Frameworks : int
{
	tflite = 0,
	mlperf,
	SNPE
};

std::unordered_map<std::string, Frameworks> const frameworks_table
    = {{"tflite", Frameworks::tflite},
       {"mlperf", Frameworks::mlperf},
       {"snpe", Frameworks::SNPE}};

inline Frameworks string2framework( std::string fr )
{
	auto it = frameworks_table.find( fr );
	if( it != frameworks_table.end() )
	{
		return it->second;
	}
	else
		throw std::runtime_error( fmt::format( "Enum {0} not defined", fr ) );
}

inline std::string framework2string( Frameworks fr )
{
	auto it = std::find_if( std::begin( frameworks_table ),
	                        std::end( frameworks_table ),
	                        [&]( auto&& p ) { return p.second == fr; } );

	if( it != std::end( frameworks_table ) )
	{
		return it->first;
	}
	else
		throw std::runtime_error( "Framework not defined" );
}

void check_reqs();
void check_tflite_reqs();
std::vector<std::string> check_console_output( std::string const& cmd );
std::vector<std::string> get_tflite_models();
std::vector<std::string> get_models_on_device( Frameworks );

// TODO: use std::variant for options?
void run_tflite_benchmark( std::vector<std::string> model_paths,
                           std::unordered_map<std::string, std::string> options,
                           int processes = 1 );

class IoctlDmesgStreamer
{
   public:
	IoctlDmesgStreamer();
	~IoctlDmesgStreamer()                           = default;
	IoctlDmesgStreamer( IoctlDmesgStreamer const& ) = delete;
	IoctlDmesgStreamer( IoctlDmesgStreamer&& )      = delete;

	std::vector<std::string> const& get_data() { return this->latest_data; }
	std::unordered_map<std::string, int> const& get_interactions()
	{
		return this->latest_interactions;
	}

	std::vector<std::string> const& more();
	std::unordered_map<std::string, int> const&
	interactions( bool check_full_log = false, double threshold = 20.0 );

   private:
	double latest_ts;
	std::vector<std::string> latest_data;
	std::unordered_map<std::string, int> latest_interactions;
};

} // namespace atop

#endif // ATOP_H_IN
