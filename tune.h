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

constexpr int LOSS_SMOOTH = 65536;

struct Net_train {
	std::mutex m;

	double L0_a[SIZE_F0 * SIZE_F1];
	double L0_b[SIZE_F1];

	double L1_a[SIZE_F1 * SIZE_F2];
	double L1_b[SIZE_F2];

	double L2_a[SIZE_F2 * SIZE_F3];
	double L2_b[SIZE_F3];

	double L3_a[SIZE_F3];
};

void convert_to_double(Net_train* dst, Net* src);
void convert_to_int(Net* dst, Net_train* src);

void do_learning(Net* dst, Net* src, int64_t games, int threads, double lr);

#endif
