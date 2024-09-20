#ifndef LEARNING_INCLUDED
#define LEARNING_INCLUDED

#include <cstdint>
#include <iomanip>
#include <mutex>

#include "eval.h"
#include "position.h"
#include "movegen.h"
#include "network.h"
#include "threads.h"

constexpr int LOSS_SMOOTH = 8192;
constexpr int THREAD_LOOP = 100;

struct Net_train {
	std::mutex m;

	alignas(32)
	float L0_a[SIZE_F0 * SIZE_F1];
	float L0_b[SIZE_F1];

	float L1_a[SIZE_F1 * SIZE_F2];
	float L1_b[SIZE_F2];

	float L2_a[SIZE_F2 * SIZE_F3];
	float L2_b[SIZE_F3];

	float L3_a[SIZE_F3];
	float L3_b;
};

void convert_to_double(Net_train* dst, Net* src);
void convert_to_int(Net* dst, Net_train* src);

void do_learning(Net* dst, Net* src, int64_t games, int threads, int find_depth, double lr);

#endif
