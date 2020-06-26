#include "fifo.h"

#include <map>
#include <string>
#include <thread>

template<typename Data_t>
void atop::fifo::FIFO<Data_t>::push_data( Data_t const& more )
{
	while( this->data.size() > this->max_data_sz )
		continue;

	std::unique_lock<std::mutex> lock( this->mtx );

	this->data.push( more );

	lock.unlock();
}

template<typename Data_t> Data_t atop::fifo::FIFO<Data_t>::pop_data()
{
	std::unique_lock<std::mutex> lock( this->mtx );

	Data_t ret = this->data.front();

	this->data.pop();

	return ret;
}

// Instantiations
using ti1 = std::map<std::string, int>;
template ti1 atop::fifo::FIFO<ti1>::pop_data();
template void atop::fifo::FIFO<ti1>::push_data( ti1 const& more );

using ti2 = std::map<std::string, double>;
template ti2 atop::fifo::FIFO<ti2>::pop_data();
template void atop::fifo::FIFO<ti2>::push_data( ti2 const& more );
