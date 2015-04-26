#ifndef __minHeap_HPP_INCLUDED__
#define __minHeap_HPP_INCLUDED__

#include <tuple>
#include <vector>

class MinHeap{
	private:
		std::vector<std::tuple<float, float>> minHeap;
		bool Compare(std::tuple<float, float>, std::tuple<float, float>);

	public:
		void Insert(std::tuple<float, float>);
		std::tuple<float, float> Delete();
		bool empty();
};

#endif