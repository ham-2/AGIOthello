#include <iostream>
#include <sstream>
#include <string>
#include <chrono>

#include "options.h"
#include "board.h"
#include "benchmark.h"
#include "movegen.h"
#include "search.h"
#include "threads.h"

#include <bitset>

using namespace std;
using namespace std::chrono;

int main() {

	string input, word;

	system_clock::time_point time_start = system_clock::now();
	milliseconds startup_time = milliseconds(0);
	
	Board::init();
	Position::init();
	Threads.init();

	PRNG rng2 = PRNG(3245356235923498ULL);

	system_clock::time_point time_now = system_clock::now();
	startup_time = duration_cast<milliseconds>(time_now - time_start);
	cout << nounitbuf << "AGIOthello alpha\n" 
		<< "Startup took " << startup_time.count() << "ms" << endl;

	while (true) {
		getline(cin, input);

		istringstream ss(input);
		word.clear();
		ss >> skipws >> word;

		if (word == "quit") { break; }

		else if (word == "uci") {
			cout << "id name AGIOthello\n"
				<< "id author Seungrae Kim" << endl;
			// Options
			print_option();
			Threads.acquire_cout();
			cout << "uciok" << endl;
			Threads.release_cout();
		}

		else if (word == "isready") {
			Threads.stop = true;
			Threads.sync();
			Threads.acquire_cout();
			cout << "readyok" << endl;
			Threads.release_cout();
		}

		else if (word == "position") {
			ss >> word;
			string fen;
			if (word == "fen") {
				while (ss >> word && word != "moves") {
					fen += word + " ";
				}
			}
			else if (word == "startpos") {
				fen = startpos_fen;
				ss >> word; // "moves"
			}

			Threads.stop = true;
			Threads.acquire_lock();
			Threads.set_all(fen);
			if (word == "moves") {
				while (ss >> word) {
					Threads.do_move(word);
				}
			}
			Threads.release_lock();
		}

		else if (word == "go") {
			Color c = Threads.get_color();
			float time;
			int max_ply;
			get_time(ss, c, time, max_ply);
			thread t = thread(search_start,
				Threads.threads[0], time, max_ply);
			t.detach();
		}

		else if (word == "stop") {
			Threads.stop = true;
		}

		else if (word == "setoption") {
			set_option(ss);
		}

		else if (word == "perft") {
			int depth;
			ss >> depth;
			Threads.acquire_lock();
			perft(Threads.board, depth);
			Threads.release_lock();
		}

		else if (word == "moves") {
			Threads.acquire_lock();
			while (ss >> word) {
				Threads.do_move(word);
			}
			Threads.release_lock();
		}

		else if (word == "undo") {
			Threads.acquire_lock();
			while (ss >> word) {
				Threads.undo_move(word);
			}
			Threads.release_lock();
		}

		else if (word == "showboard") {
			int threadidx;
			threadidx = ss >> threadidx ? threadidx : 0;
			Threads.show(threadidx);
		}

		else if (word == "generate") {
			Threads.gen();
		}

		else if (word == "test") {
			ss >> word;
			Threads.test_eval();
		}

	}

	return 44;
}
