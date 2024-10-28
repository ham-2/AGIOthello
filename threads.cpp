#include "threads.h"

#include <iostream>

using namespace std;

Threadmgr Threads;
const char* startpos_fen = "8/8/8/3OX3/3XO3/8/8/8 b";
const char* default_weight = "weights.bin";
	
void lazy_smp(Thread* t) {
	int currdepth;
	TTEntry probe = {};

	SearchParams sp = {
		t->board, &Threads.stop, &Main_TT, t->step
	};

	while (!(t->kill)) {
		unique_lock<mutex> m(t->m);
		Threads.t_wait.wait(m);

		Main_TT.probe(t->board->get_key(), &probe);
		int window_a = 2 << (EVAL_BITS - 6);
		int window_b = 2 << (EVAL_BITS - 6);
		int window_c = probe.eval;

		while (!Threads.stop) {
			currdepth = Threads.depth + 1;

			window_c = alpha_beta(&sp,
				&probe, currdepth,
				window_c - window_a,
				window_c + window_b);

			if (probe.type == 1) {
				window_a *= 4;
			}
			if (probe.type == -1) {
				window_b *= 4;
			}
			if (probe.type == 0) {
				window_a = 2 << (EVAL_BITS - 6);
				window_b = 2 << (EVAL_BITS - 6);
				if (Threads.depth == t->board->get_count_empty()) {
					window_a += window_c < 0 ? EVAL_END : 0;
					window_b += window_c > 0 ? EVAL_END : 0;
				}
			}
		}
	}

	(t->kill) = false;
	return;
}

Thread::Thread(int id, Net* n) {
	board = new Position(n);
	this->id = id;
	step = Primes[id];
}

Thread::~Thread() {
	delete board;
	delete t;
}

void Threadmgr::init() {
	stop = new atomic<bool>;
	n = new Net;
	load_weights(n, default_weight);
	board = new Position(n);
	Threads.threads.push_back(new Thread(0, n));
	set_all(startpos_fen);
}

void Threadmgr::add_thread() {
	Thread* new_thread = new Thread(num_threads, n);
	*(new_thread->board) = *board;
	threads.push_back(new_thread);
	new_thread->t = new thread(lazy_smp, new_thread);
	num_threads++;
}

void Threadmgr::del_thread() {
	Thread* t = threads.back();
	t->kill = true;
	t_wait.notify_all();
	while (t->kill) {
		this_thread::sleep_for(chrono::milliseconds(5));
	}
	threads.pop_back();
	delete t;
	num_threads--;
}

void Threadmgr::set_threads(int new_threads) {
	if (new_threads < 1) { new_threads = 1; }
	else if (new_threads > THREADS_MAX) { new_threads = THREADS_MAX; }

	if (new_threads > num_threads) {
		while (new_threads != num_threads) {
			add_thread();
		}
	}

	if (new_threads < num_threads) {
		while (new_threads != num_threads) {
			del_thread();
		}
	}
}

void Threadmgr::acquire_cout() { cout_lock.lock(); }

void Threadmgr::release_cout() { cout_lock.unlock(); }

void Threadmgr::acquire_lock() {
	for (int i = 0; i < threads.size(); i++) {
		threads[i]->m.lock();
	}
}

void Threadmgr::release_lock() {
	for (int i = 0; i < threads.size(); i++) {
		threads[i]->m.unlock();
	}
}

void Threadmgr::sync() {
	acquire_lock();
	release_lock();
}

void Threadmgr::set_all(string fen) {
	board->set(fen);
	for (int i = 0; i < threads.size(); i++) {
		threads[i]->board->set(fen);
	}
}

void Threadmgr::show(int i) {
	if (i < 0) { board->show(); }
	if (i < threads.size()) {
		threads[i]->board->show();
	}
}

void Threadmgr::set_weights() {
	board->set_accumulator();
	for (int i = 0; i < threads.size(); i++) {
		memcpy(threads[i]->board->get_net(), n, sizeof(Net));
		threads[i]->board->set_accumulator();
	}
}

void Threadmgr::do_move(Square m) {
	Bitboard c;
	board->do_move_wrap(m, &c);
	for (int i = 0; i < threads.size(); i++) {
		threads[i]->board->do_move_wrap(m, &c);
	}
}

void Threadmgr::test_eval() {
	int endeval = eval(*board);

	// Check if ended
	MoveList ml;
	ml.generate(*board);
	if (ml.list == ml.end) {
		board->pass();
		ml.generate(*board);
		if (ml.list == ml.end) {
			endeval ^= EVAL_END;
		}
		board->pass();
	}
	
	cout << "Material: " << eval_print(get_material(*board) ^ EVAL_END)
		<< "\nEvaluation: " << eval_print(endeval) << endl;
}

void Threadmgr::gen()
{
	MoveList ml;
	ml.generate(*board);
	ml.show();
	cout << "\n" << ml.length() << " moves" << endl;
}