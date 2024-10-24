#include "search.h"

#include "alphabeta.h"
#include "eval.h"
#include "table.h"
#include "printer.h"

using namespace std;

bool ponder = false;
atomic<bool> ponder_continue(false);

int multipv = 1;

void get_time(istringstream& ss, Color c, float& time, int& max_ply) {
	string word;
	time = 1.0f;
	max_ply = 100;
	while (ss >> word) {
		if (word == "wtime") {
			ss >> word;
			if (c) {
				time = stof(word) / 15000;
			}
		}
		else if (word == "btime") {
			ss >> word;
			if (!c) {
				time = stof(word) / 15000;
			}
		}
		else if (word == "winc") {
			ss >> word;
			if (c) {
				time += stof(word) / 1000;
			}
		}
		else if (word == "binc") {
			ss >> word;
			if (!c) {
				time += stof(word) / 1000;
			}
		}
		else if (word == "movetime") {
			ss >> word;
			time = stof(word) / 1000;
		}
		else if (word == "infinite") {
			ss >> word;
			time = -1;
			break;
		}
		else if (word == "depth") {
			ss >> word;
			max_ply = stoi(word);
		}
	}
}

void search_start(Thread* t, float time, int max_ply)
{
	Threads.stop = false;

	Square bmove = NULL_MOVE;
	atomic<bool>* complete = new atomic<bool>;
	*complete = false;
	condition_variable* print_cond = new condition_variable;
	Position* board = t->board;

	node_count.exchange(0);
	Main_TT.increment();

	thread print_t = thread(printer, time, complete, print_cond);
	TTEntry probe = {};
	Main_TT.probe(board->get_key(), &probe);
	Threads.depth.exchange(1);

	// Start parallel search
	Threads.t_wait.notify_all();
	Threads.threads[0]->m.lock();

	int window_a = 2 << (EVAL_BITS - 6);
	int window_b = 2 << (EVAL_BITS - 6);
	int window_c = probe.eval;
	
	SearchParams sp = {
		board, &(Threads.stop), &Main_TT, t->step
	};

	while (Threads.depth <= max_ply) {
		window_c = alpha_beta(&sp,
			&probe, Threads.depth,
			window_c - window_a,
			window_c + window_b);
			
		// Forced Stop
		if (Threads.stop) { break; }

		if (probe.type == 1) {
			window_a *= 4;
		}
		if (probe.type == -1) {
			window_b *= 4;
		}
		if (probe.type == 0) {
			// Print and search again
			Threads.depth++;
			print_cond->notify_all();

			window_a = 2 << (EVAL_BITS - 6);
			window_b = 2 << (EVAL_BITS - 6);
			if (Threads.depth == board->get_count_empty()) {
				window_a += window_c < 0 ? EVAL_END : 0;
				window_b += window_c > 0 ? EVAL_END : 0;
			}
		}
	}

	// terminate print thread
	*complete = true;
	print_cond->notify_all();
	print_t.join();
	delete complete;
	Threads.stop = true;
	Threads.threads[0]->m.unlock();
	Threads.sync();

	// Print Bestmove
	Main_TT.probe(board->get_key(), &probe);
	Threads.acquire_cout();
	std::cout << "bestmove " << probe.nmove << endl;
	Threads.release_cout();

	// Ponder
	if (ponder_continue)
	{
		Threads.do_move(bmove);
		//Threads.depth--;
		Threads.stop = false;
		ponder_continue = false;

		// Start parallel search
		Threads.t_wait.notify_all();

		Threads.threads[0]->m.lock();

		TTEntry probe_ponder = {};
		Main_TT.probe(board->get_key(), &probe_ponder);
		while (Threads.depth <= max_ply && !Threads.stop) {
			alpha_beta(&sp, &probe_ponder, Threads.depth);

			Main_TT.probe(board->get_key(), &probe_ponder);
			++Threads.depth;
		}

		Threads.stop = true;
		Threads.threads[0]->m.unlock();
	}

	return;
}