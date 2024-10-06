#include <iostream>
#include <sstream>
#include <string>
#include <chrono>

#include "options.h"
#include "board.h"
#include "benchmark.h"
#include "tune.h"
#include "movegen.h"
#include "search.h"
#include "threads.h"
#include "network.h"

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

	system_clock::time_point time_now = system_clock::now();
	startup_time = duration_cast<milliseconds>(time_now - time_start);
	cout << nounitbuf << "\nTopoki Is Good Beta\n" 
		<< "Startup took " << startup_time.count() << "ms\n" << endl;

	while (true) {
		getline(cin, input);

		istringstream ss(input);
		word.clear();
		ss >> skipws >> word;

		if (word == "quit") { break; }

		else if (word == "uci") {
			cout << "id name Topoki Is Good beta\n"
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

		// Weight

		else if (word == "load") {
			ss >> word;
			word = std::filesystem::current_path().string() + "/" + word;
			Threads.acquire_lock();
			load_weights(Threads.n, word);
			Threads.set_weights();
			Threads.release_lock();
		}

		else if (word == "save") {
			ss >> word;
			word = std::filesystem::current_path().string() + "/" + word;
			save_weights(Threads.n, word);
		}

		else if (word == "zero") {
			zero_weights(Threads.n);
			Threads.set_weights();
		}

		else if (word == "rand") {
			ss >> word;
			rand_weights_all(Threads.n, stoi(word));
			Threads.set_weights();
		}

		else if (word == "tune") {
			int cycle = 0;
			int find_depth[32] = { };
			int rand_depth[32] = { };
			uint64_t games_[32] = { };
			double lr[32] = { };

			ss >> word;
			int threads = stoi(word);

			cout << "\nLearning with: " << threads << " Threads\n" << endl;

			while (ss >> word) {
				find_depth[cycle] = stoi(word);

				ss >> word;
				rand_depth[cycle] = stoi(word);

				ss >> word;
				games_[cycle] = int64_t(stoi(word)) * 1000;

				ss >> word;
				lr[cycle] = stod(word);

				cycle++;
			}

			thread t = thread(do_learning_cycle, Threads.n,
				games_, threads, find_depth, rand_depth, lr, cycle);
			t.detach();

		}

		else if (word == "testnet") {
			Net* n = nullptr;
			int type;
			string dir;

			ss >> word;
			if (word[1] == 'n') {
				n = new Net;
				ss >> word;
				if (load_weights(n, word)) { continue; }
				type = 2;
			}
			else if (word[1] == 'g') {
				type = 1;
			}
			else if (word[1] == 'r') {
				type = 0;
			}
			else if (word[1] == 'b') {
				ss >> word;
				dir = word;
				type = 3;
			}

			ss >> word;
			int threads = stoi(word);

			ss >> word;
			int games = stoi(word);

			ss >> word;
			int depth_start = stoi(word);

			ss >> word;
			int depth_search = stoi(word);

			if (type == 3) {
				test_batch(dir, threads, games, depth_start, depth_search);
			}
			else {
				test_net(threads, games, depth_start, depth_search, type, n);
			}
			if (n != nullptr) { delete n; }
		}

		// Commands for debugging

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
				Threads.undo_move();
			}
			Threads.release_lock();
		}

		else if (word == "showboard") {
			int threadidx;
			threadidx = ss >> threadidx ? threadidx : 0;
			Threads.show(threadidx);
		}

		else if (word == "eval") {
			ss >> word;
			Threads.test_eval();
		}

		else if (word == "perft") {
			int depth;
			ss >> depth;
			Threads.acquire_lock();
			perft(Threads.board, depth);
			Threads.release_lock();
		}

		else if (word == "verify") {
			Threads.board->verify();
		}

		else if (word == "solve") {
			solve();
		}

		else if (word == "nettest") {
			net_speedtest();
		}

		else if (word == "gen") {
			Threads.gen();
		}

		else if (word == "encode") {
			encode_literal(Threads.n);
		}

		//else if (word == "decode") {
		//	string w[] = { "" };
		//	decode_literal(Threads.n, w);
		//}

		else if (word == "stats") {
			get_stats(Threads.n);
		}

		else if (word == "rand1") {
			ss >> word;
			rand_weights_1(Threads.n, stoi(word));
			Threads.set_weights();
		}
	}

	return 44;
}
