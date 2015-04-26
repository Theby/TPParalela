#ifndef __minHeap_HPP_INCLUDED__
#define __minHeap_HPP_INCLUDED__

#include <tuple>
#include <vector>

class MinHeap{
	private:
		std::vector<std::tuple<float, int>> minHeap;
		bool Compare(std::tuple<float, int>, std::tuple<float, int>);

	public:
		void Insert(std::tuple<float, int>);
		std::tuple<float, int> Delete();
		bool empty();
		void printMainHeap();
};

#endif