#ifndef LEARNING_INCLUDED
#define LEARNING_INCLUDED

#include <cassert>
#include <cstdint>
#include <iomanip>
#include <mutex>

#include "benchmark.h"
#include "eval.h"
#include "position.h"
#include "movegen.h"
#include "network.h"
#include "threads.h"

struct Net_train {
	std::mutex m;

	alignas(32)
	float L0_a[SIZE_F0 * SIZE_F1];
	float L0_b[SIZE_F1];

	float L1_a[SIZE_F1 * SIZE_F2];
	float L1_b[SIZE_F2];

	float L2_a[SIZE_F2 * SIZE_F3];
	float L2_b[SIZE_F3];

	float L3_a[SIZE_F3 * SIZE_OUT];
	float L3_b[SIZE_OUT];
};

void do_learning(Net* dst, Net* src, uint64_t games,
	int threads, int find_depth, int rand_depth, double lr);

#endif
