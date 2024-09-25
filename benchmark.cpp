#include "benchmark.h"

using namespace std;
using namespace std::chrono;

uint64_t nodes = 0;

uint64_t _perft(Position* board, int depth) {
	if (depth == 0) {
		return 1ULL;
	}
	else {
		uint64_t value = 0;
		MoveList ml;
		ml.generate(*board);
		for (Square* s = ml.list; s < ml.end; s++) {	
			Undo u;
			board->do_move(*s, &u);
			value += _perft(board, depth - 1);
			board->undo_move();
		}
		if (ml.list == ml.end) {
			if (board->get_passed()) {
				return 1ULL;
			}
			else {
				Undo u;
				board->do_null_move(&u);
				value += _perft(board, depth - 1);
				board->undo_null_move();
			}
		}
		return value;
	}
}

void perft(Position* board, int depth) {
	system_clock::time_point time_start = system_clock::now();
	milliseconds time = milliseconds(0);

	uint64_t value = 0;
	MoveList ml;
	ml.generate(*board);
	for (Square* s = ml.list; s < ml.end; s++) {
		Undo u;
		board->do_move(*s, &u);
		uint64_t sub = _perft(board, depth - 1);
		cout << *s << " " << sub << endl;
		value += sub;
		board->undo_move();
	}
	cout << value << '\n';

	system_clock::time_point time_now = system_clock::now();
	time = duration_cast<milliseconds>(time_now - time_start);
	cout << "took " << time.count() << "ms, " <<
		double(value) / time.count() / 1000 << " MNps" << endl;
}

int _solve() {

	nodes++;

	if (Threads.stop) { return EVAL_FAIL; }

	// Move generation
	MoveList legal_moves;
	legal_moves.generate(*Threads.board);

	Key root_key = Threads.board->get_key();

	// No legal moves
	if (legal_moves.list == legal_moves.end) {

		if (Threads.board->get_passed()) {
			int score = get_material(*Threads.board) ^ EVAL_END;
			Main_TT.register_entry(root_key, 64, score, GAME_END);
			return score;
		}

		else {
			Undo u;
			Threads.board->do_null_move(&u);
			int after_pass = -_solve();
			Threads.board->undo_null_move();
			Main_TT.register_entry(root_key, 64, after_pass, NULL_MOVE);
			return after_pass;
		}
	}

	// Legal moves
	else {
		Square s;
		Square nmove = NULL_MOVE;
		int comp_eval;
		int new_eval = EVAL_INIT;

		int i = 0;
		while ((i < legal_moves.length()) && !(Threads.stop)) {
			s = *(legal_moves.list + i);

			// Do move
			Undo u;
			Threads.board->do_move(s, &u);

			comp_eval = -_solve();

			if (comp_eval > new_eval) {
				new_eval = comp_eval;
				nmove = s;
			}

			Threads.board->undo_move();
			i++;
		}

		Main_TT.register_entry(root_key, 64, new_eval, nmove);

		return new_eval;
	}
}

void solve() {
	Threads.stop = false;
	nodes = 0;
	system_clock::time_point time_start = system_clock::now();
	milliseconds time = milliseconds(0);

	int eval = _solve();
	Threads.stop = true;

	system_clock::time_point time_now = system_clock::now();
	time = duration_cast<milliseconds>(time_now - time_start);

	stringstream buf;
	int depth = 0;
	getpv(buf, Threads.board, depth);
	cout << "elapsed time: " << time.count() << "ms" 
		<< "\neval: " << eval_print(eval)
		<< "\nnodes: " << nodes
		<< ", " << double(nodes) / time.count() / 1000 << " MNps" 
		<< "\npv: " << buf.str() << endl;
}

void net_speedtest() {
	Net* n = new Net;
	zero_weights(n);
	rand_weights(n, 2);

	int16_t P1[SIZE_F1 * 2] = { };
	int64_t r = 0;

	cout << "Update test: 64M calls\n";

	// Update
	system_clock::time_point time_start = system_clock::now();
	milliseconds time = milliseconds(0);

	Bitboard pieces[3] = {};
	Piece sq[64] = { };
	pieces[EMPTY] = FullBoard;

	for (int j = 0; j < 1000000; j++) {
		for (Square s = A1; s < SQ_END; ++s) {
			Piece p = Piece(rng.get() & 3);
			if (p == MISC) { p = EMPTY; }

			update_L0(P1, s, sq[s], p, n);

			pieces[sq[s]] ^= SquareBoard[s];
			pieces[p]     ^= SquareBoard[s];
			sq[s] = p;
		}
	}

	system_clock::time_point time_now = system_clock::now();
	time = duration_cast<milliseconds>(time_now - time_start);

	cout << "elapsed time: " << time.count() << "ms"
		<< ", " << double(64000) / time.count() << " MOps" << endl;

	cout << "Evaluate test: 10M calls\n";

	// Evaluate
	time_start = system_clock::now();

	for (int i = 0; i < 10000000; i++) {
		for (int j = 0; j < SIZE_F1; j++) {
			uint64_t r = rng.get();
			P1[j++] = int16_t((r >> 0) & 127);
			P1[j++] = int16_t((r >> 8) & 127);
			P1[j++] = int16_t((r >> 16) & 127);
			P1[j]   = int16_t((r >> 24) & 127);
		}
		r += compute(P1, n, BLACK);
	}

	time_now = system_clock::now();
	time = duration_cast<milliseconds>(time_now - time_start);

	cout << "elapsed time: " << time.count() << "ms"
		<< ", " << double(10000) / time.count() << " MOps" 
		<< '\n' << r << endl;
}

void net_verify() {
	Net* n = new Net;
	zero_weights(n);
	rand_weights(n, 6);

	verify_SIMD(n);
}