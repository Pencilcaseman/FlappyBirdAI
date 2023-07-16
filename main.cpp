#include <librapid>
#include <surge/surge.hpp>

#include "include/brain.hpp"

using Scalar	= librapid::half;
using Backend	= librapid::backend::CUDA;
using BirdBrain = Brain<Scalar, Backend>;
using Array		= BirdBrain::Array;

int main() {
	fmt::print("Hello, World\n");

	librapid::setNumThreads(4);

	BirdBrain brain;
	brain << 2 << 5 << 5 << 2;
	brain.construct();

	fmt::print("{}\n", brain.forward(librapid::fromData<Scalar, Backend>({0.1, 0.2})));

	int64_t iters = 10000;
	auto input	  = librapid::fromData<Scalar, Backend>({0.1, 0.2});
	double start  = librapid::now();
	for (int64_t i = 0; i < iters; ++i) { auto res = brain.forward(input); }
	double end = librapid::now();

	fmt::print("Elapsed: {}\n", librapid::formatTime(end - start));
	fmt::print("Average: {}\n", librapid::formatTime((end - start) / double(iters)));

	return 0;
}
