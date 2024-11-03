#ifndef THREADS_INCLUDED
#define THREADS_INCLUDED

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>
#include <thread>

#include "alphabeta.h"
#include "eval.h"
#include "position.h"
#include "movegen.h"

using namespace std;

constexpr int THREADS_MAX = 32;

class Thread {
public:
	Position* board;
	int id;
	int step;
	mutex m;
	condition_variable cv;
	atomic<bool> kill;
	thread* t;

	Thread(int id, Net* n);
	~Thread();
};

class Threadmgr {
public:
	Position* board;
	mutex cout_lock;

	vector<Thread*> threads;
	atomic<bool> stop;
	atomic<int> depth;
	atomic<int> depth_max;
	condition_variable t_wait;

	Net* n;

	void init();
	void add_thread();
	void del_thread();
	void set_threads(int n);
	void acquire_cout();
	void release_cout();
	void acquire_lock();
	void release_lock();
	void sync();
	void set_all(string fen);
	void show(int i);
	void set_weights();
	void do_move(Square s);
	void do_move(Square s, Bitboard* c);
	void undo_move(Square s, Bitboard* c);
	inline void do_move(string ms) { do_move(parse_square(ms)); }

	void gen();
	void test_eval();

	inline Color get_color() { return board->get_side(); };
};

extern Threadmgr Threads;
extern const char* startpos_fen;

void lazy_smp(Thread* t);

constexpr int Primes[] = { 1,
		2731,	2741,	2749,	2753,	2767,	2777,	2789,	2791,	2797,	2801,
		2803,	2819,	2833,	2837,	2843,	2851,	2857,	2861,	2879,	2887,
		2897,	2903,	2909,	2917,	2927,	2939,	2953,	2957,	2963,	2969,
		2971,	2999,	3001,	3011,	3019,	3023,	3037,	3041,	3049,	3061,
		3067,	3079,	3083,	3089,	3109,	3119,	3121,	3137,	3163,	3167,
		3169,	3181,	3187,	3191,	3203,	3209,	3217,	3221,	3229,	3251,
		3253,	3257,	3259,	3271,	3299,	3301,	3307,	3313,	3319,	3323,
		3329,	3331,	3343,	3347,	3359,	3361,	3371,	3373,	3389,	3391,
		3407,	3413,	3433,	3449,	3457,	3461,	3463,	3467,	3469,	3491,
		3499,	3511,	3517,	3527,	3529,	3533,	3539,	3541,	3547,	3557,
		3559,	3571,	3581,	3583,	3593,	3607,	3613,	3617,	3623,	3631,
		3637,	3643,	3659,	3671,	3673,	3677,	3691,	3697,	3701,	3709,
		3719,	3727,	3733,	3739,	3761,	3767,	3769
};

#endif