#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>

// Calculates memory range given in hex.
//
// Input: <from hex>-<to hex>
// Output: gigabytes
//
// E.g.,
//     0x95300000-0x17e3bffff = 3.9 GB
//     95300000-17e3bffff = 3.9 GB

void print_usage()
{
	std::cout << "Usage: \n"
	             "\t By memory map: hex2gb -f filename\n"
	             "\t By range: hex2gb -r <from hex>-<to hex>\n"
	          << std::endl;
}

std::tuple<std::string, std::string> split2( std::string str,
                                             char delim = '\n' )
{
	size_t delim_pos = str.find( delim );
	return std::make_tuple( str.substr( 0, delim_pos ),
	                        str.substr( delim_pos + 1, str.length() ) );
}

double hex2bytes( std::string str )
{
	auto [from, to] = split2( str, '-' );

	auto from_hex = std::stoull( from, 0, 16 );
	auto to_hex   = std::stoull( to, 0, 16 );

	return static_cast<double>( ( to_hex - from_hex ) );
}

std::string prettify_bytes( double bytes )
{
	std::string units[] = {"B", "KB", "MB", "GB"};
	int unit_ptr        = 0;
	while( bytes >= 1000 )
	{
		bytes /= 1000;
		if( ++unit_ptr == sizeof( units ) / sizeof( units[0] ) )
			break;
	}

	std::stringstream ss;
	ss << std::setprecision( 4 ) << bytes << " " << units[unit_ptr];
	return ss.str();
}

void iter_mmap( std::string filename )
{
	std::ifstream is{filename};
	std::string line;
	std::string str_range, device_name;

	while( getline( is, line ) )
	{
		std::tie( str_range, device_name ) = split2( line, ':' );
		std::cout << device_name << ": ";
		std::cout << prettify_bytes( hex2bytes( str_range ) ) << std::endl;
	}
	std::cout << std::endl;
}

int main( int argc, char* argv[] )
{
	if( argc == 3 )
	{
		std::string flag{argv[1]};
		if( flag == "-f" )
		{
			iter_mmap( argv[2] );
		}
		else if( flag == "-r" )
		{
			std::cout << prettify_bytes( hex2bytes( argv[2] ) ) << " GB"
			          << std::endl;
		}
		else
		{
			print_usage();
		}
	}
	else
	{
		print_usage();
	}

	return 0;
}
