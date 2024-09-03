#include "search.h"

#include "alphabeta.h"
#include "eval.h"
#include "table.h"
#include "printer.h"

using namespace std;

bool stop_if_mate = true;

bool ponder = false;
atomic<bool> ponder_continue(false);

int multipv = 1;

void get_time(istringstream& ss, Color c, float& time, int& max_ply) {
	string word;
	time = 1.0f;
	max_ply = 60;
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
		
	int multipv_max = multipv;
	Square bmove = SQ_END;
	atomic<bool>* complete = new atomic<bool>;
	*complete = false;
	condition_variable* print_cond = new condition_variable;
	Position* board = t->board;

	node_count.exchange(0);
	Main_TT.increment();

	thread print_t = thread(printer, time, complete, print_cond);

	Main_TT.clear_entry(board->get_key());
	Threads.depth.exchange(1);

	// Start parallel search
	Threads.t_wait.notify_all();
	Threads.threads[0]->m.lock();

	TTEntry probe = {};
	Main_TT.probe(board->get_key(), &probe);

	while (Threads.depth <= max_ply) {
		alpha_beta(*board, &(Threads.stop),
			Threads.depth, &probe,
			board->get_side(), t->step);
			
		// Forced Stop
		if (Threads.stop) { break; }

		Main_TT.probe(board->get_key(), &probe);

		// Stop if mate
		if (stop_if_mate && is_mate(probe.eval)) {
			break;
		}

		// Print and search again
		Threads.depth++;
		print_cond->notify_all();
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
	cout << "bestmove " << probe.nmove << endl;		
	Threads.release_cout();

	// Ponder
	if (ponder_continue && !(stop_if_mate && is_mate(probe.eval)))
	{
		Threads.do_move(bmove);
		Threads.depth.exchange(1);
		Threads.stop = false;
		ponder_continue = false;

		// Start parallel search
		Threads.t_wait.notify_all();

		Threads.threads[0]->m.lock();

		TTEntry probe_ponder = {};
		Main_TT.probe(board->get_key(), &probe_ponder);
		while (Threads.depth <= max_ply && !Threads.stop) {
			alpha_beta(*board, &(Threads.stop),
				Threads.depth, &probe_ponder,
				~(board->get_side()), t->step);

			Main_TT.probe(board->get_key(), &probe_ponder);
			++Threads.depth;
		}

		Threads.stop = true;
		Threads.threads[0]->m.unlock();
	}

	return;
}