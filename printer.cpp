#include "printer.h"

using namespace std;

void printer(float time, atomic<bool>* stop, condition_variable* cv)
{
	using namespace std::chrono;
	milliseconds limit_time{ static_cast<long int>(1000 * time) };
	system_clock::time_point time_start = system_clock::now();
	milliseconds search_time = milliseconds(0);
	system_clock::time_point time_now;
	Square bmove = NULL_MOVE;
	int score = 0;
	int max_depth = 0;
	int currdepth;
	uint64_t nodes;
	mutex mu;

	// Move Generation
	MoveList legal_moves;
	legal_moves.generate(*Threads.board);

	while (true) {
		time_now = system_clock::now();
		search_time = duration_cast<milliseconds>(time_now - time_start);
		unique_lock<mutex> m(mu);

		if (time == -1 || limit_time - search_time > milliseconds(PRINT_MIN_MS)) {
			cv->wait_for(m, milliseconds(PRINT_MIN_MS));
		}
		else {
			cv->wait_for(m, limit_time - search_time);
		}

		// Limit Time
		time_now = system_clock::now();
		search_time = duration_cast<milliseconds>(time_now - time_start);
		nodes = node_count;

		if (time != -1 && search_time >= limit_time) {
			if (ponder) { ponder_continue = true; }
			Threads.stop = true;
			*stop = true;
		}

		// Get candidate move
		TTEntry probe = {};
		Main_TT.probe(Threads.board->get_key(), &probe);
		stringstream buf;
		getpv(buf, Threads.board);

		// Print
		Threads.acquire_cout();
		cout << "info time " << search_time.count() << " depth " << Threads.depth
			<< " currmove " << move(probe.nmove) << " score " << eval_print(probe.eval)
			<< " nodes " << nodes << " nps " << int(double(nodes * 1000) / (1 + search_time.count()))
			<< " pv " << buf.str() << endl;
		Threads.release_cout();

		if (*stop) { break; }
	}

}