#include <iomanip>
#include <iostream>
#include <string>

// Calculates memory range given in hex.
//
// Input: <from hex>-<to hex>
// Output: gigabytes
//
// E.g.,
//     0x95300000-0x17e3bffff = 3.9 GB
//     95300000-17e3bffff = 3.9 GB

int main(int argc, char* argv[])
{
	std::string in;

	if(argc == 2) {
		in = argv[1];
	} else {
		std::cout << "Hex Range: " << std::endl;
		std::cin >> in;
	}

	size_t delim_pos = in.find( '-' );
	std::string from = in.substr( 0, delim_pos );
	std::string to   = in.substr( delim_pos + 1, in.length() );

	auto from_hex = std::stoull( from, 0, 16 );
	auto to_hex   = std::stoull( to, 0, 16 );

	std::cout << std::setprecision( 4 )
	          << ( static_cast<double>( ( to_hex - from_hex ) ) / 1e9 ) << " GB"
	          << std::endl;

	return 0;
}
