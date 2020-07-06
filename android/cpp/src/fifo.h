#ifndef FIFO_H_IN
#define FIFO_H_IN

#include <queue>
#include <thread>

namespace atop
{
namespace fifo
{
template<typename Data_t> class FIFO
{
  public:
	FIFO()  = default;
	~FIFO() = default;

	void push_data( Data_t const& );
	Data_t pop_data();
	bool data_avail() const { return !( this->data.empty() ); }

  private:
	std::mutex mtx{};
	std::queue<Data_t> data{};

	// block if queue reaches this many elements
	size_t max_data_sz = 10;
};

} // namespace fifo
} // namespace atop

#endif // FIFO_H_IN
