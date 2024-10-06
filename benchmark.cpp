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
			int score = get_material_eval(*Threads.board);
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
	rand_weights_all(n, 2);

	int16_t P1[SIZE_F1 * 2] = { };

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
	int v[2];

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
		compute(v, P1, n, BLACK);
	}

	time_now = system_clock::now();
	time = duration_cast<milliseconds>(time_now - time_start);

	cout << "elapsed time: " << time.count() << "ms"
		<< ", " << double(10000) / time.count() << " MOps" 
		<< '\n' << v[0] << ' ' << v[1] << endl;
}

int find_best(Position& board, int depth,
	int alpha, int beta) {
	if (depth < 1) { return eval(board); }

	MoveList legal_moves;
	legal_moves.generate(board);

	if (legal_moves.list == legal_moves.end) {
		if (board.get_passed()) {
			return get_material_eval(board);
		}

		else {
			Undo u;
			board.do_null_move(&u);
			int after_pass = -find_best(board, depth - 1, -beta, -alpha);
			board.undo_null_move();
			return after_pass;
		}
	}

	Square s;
	Square nmove = NULL_MOVE;
	int comp_eval;
	int new_eval = EVAL_INIT;

	for (int i = 0; i < legal_moves.length(); i++) {
		s = legal_moves.list[i];

		Undo u;
		board.do_move(s, &u);
		comp_eval = -find_best(board, depth - 1, -beta, -alpha);
		board.undo_move();

		if (comp_eval > new_eval) {
			new_eval = comp_eval;
			nmove = s;
			if (new_eval > alpha) { alpha = new_eval; }
			if (alpha > beta) { break; }
		}
	}
	return new_eval;
}

int find_g(Position& board, int depth,
	int alpha, int beta) {
	if (depth < 1) { return get_material_eval(board); }

	MoveList legal_moves;
	legal_moves.generate(board);

	if (legal_moves.list == legal_moves.end) {
		if (board.get_passed()) {
			return get_material_eval(board);
		}

		else {
			Undo u;
			board.do_null_move(&u);
			int after_pass = -find_g(board, depth - 1, -beta, -alpha);
			board.undo_null_move();
			return after_pass;
		}
	}

	Square s;
	Square nmove = NULL_MOVE;
	int comp_eval;
	int new_eval = EVAL_INIT;

	for (int i = 0; i < legal_moves.length(); i++) {
		s = legal_moves.list[i];

		Undo u;
		board.do_move(s, &u);
		comp_eval = -find_g(board, depth - 1, -beta, -alpha);
		board.undo_move();

		if (comp_eval > new_eval) {
			new_eval = comp_eval;
			nmove = s;
			if (new_eval > alpha) { alpha = new_eval; }
			if (alpha > beta) { break; }
		}
	}
	return new_eval;
}

int play_r(Position& board1, int sd, bool rand) {
	MoveList legal_moves;
	legal_moves.generate(board1);
	if (legal_moves.list == legal_moves.end) {
		if (board1.get_passed()) { return get_material_eval(board1); }
		Undo u1;
		board1.do_null_move(&u1);
		int r = -play_r(board1, sd, !rand);
		board1.undo_null_move();
		return r;
	}
	else {
		Square s;
		Square nmove = NULL_MOVE;
		int comp_eval;
		int new_eval = EVAL_INIT;

		if (rand) {
			nmove = legal_moves.list[rng.get() % legal_moves.length()];
		}
		else {
			for (int i = 0; i < legal_moves.length(); i++) {
				s = legal_moves.list[i];
				Undo u;
				board1.do_move(s, &u);
				comp_eval = -find_best(board1, sd, EVAL_MIN, -new_eval);
				if (comp_eval > new_eval) {
					new_eval = comp_eval;
					nmove = s;
				}
				board1.undo_move();
			}
		}
		Undo u1;
		board1.do_move(nmove, &u1);
		int r = -play_r(board1, sd, !rand);
		board1.undo_move();
		return r;
	}
}

int play_g(Position& board1, int sd, bool rand) {
	MoveList legal_moves;
	legal_moves.generate(board1);
	if (legal_moves.list == legal_moves.end) {
		if (board1.get_passed()) { return get_material_eval(board1); }
		Undo u1;
		board1.do_null_move(&u1);
		int r = -play_g(board1, sd, !rand);
		board1.undo_null_move();
		return r;
	}
	else {
		Square s;
		Square nmove = NULL_MOVE;
		int comp_eval;
		int new_eval = EVAL_INIT;
		for (int i = 0; i < legal_moves.length(); i++) {
			s = legal_moves.list[i];
			Undo u;
			board1.do_move(s, &u);
			if (rand) {
				comp_eval = -find_g(board1, sd, EVAL_MIN, -new_eval);
			}
			else {
				comp_eval = -find_best(board1, sd, EVAL_MIN, -new_eval);
			}
			if (comp_eval > new_eval) {
				new_eval = comp_eval;
				nmove = s;
			}
			board1.undo_move();
		}
		Undo u1;
		board1.do_move(nmove, &u1);
		int r = -play_g(board1, sd, !rand);
		board1.undo_move();
		return r;
	}
}

int play_n(Position& board1, Position& board2, int sd) {

	MoveList legal_moves;
	legal_moves.generate(board1);
	if (legal_moves.list == legal_moves.end) {
		if (board1.get_passed()) { return get_material_eval(board1); }
		Undo u1, u2;
		board1.do_null_move(&u1);
		board2.do_null_move(&u2);
		int r = -play_n(board2, board1, sd);
		board1.undo_null_move();
		board2.undo_null_move();
		return r;
	}
	else {
		if (board1.get_count(EMPTY) < 9) {
			return find_best(board1, 64, EVAL_MIN, EVAL_MAX);
		}

		Square s;
		Square nmove = NULL_MOVE;
		int comp_eval;
		int new_eval = EVAL_INIT;
		for (int i = 0; i < legal_moves.length(); i++) {
			s = legal_moves.list[i];
			Undo u;
			board1.do_move(s, &u);
			comp_eval = -find_best(board1, sd, EVAL_MIN, -new_eval);
			if (comp_eval > new_eval) {
				new_eval = comp_eval;
				nmove = s;
			}
			board1.undo_move();
		}
		Undo u1, u2;
		board1.do_move(nmove, &u1);
		board2.do_move(nmove, &u2);
		int r = -play_n(board2, board1, sd);
		board1.undo_move();
		board2.undo_move();
		return r;
	}
}

void test_thread(Net* n1, Net* n2, 
	int type, int games, int depth_start, int depth_search,
	int* res) 
{
	Position board1(n1);
	Position board2(n2);

	int win  = 0;
	int draw = 0;
	int loss = 0;

	system_clock::time_point time_start = system_clock::now();
	milliseconds time = milliseconds(0);

	for (int i = 0; i < games; i++) {
		board1.set(startpos_fen);

		for (int j = 0; j < depth_start; j++) {
			MoveList legal_moves;
			legal_moves.generate(board1);
			if (legal_moves.list == legal_moves.end) {
				if (board1.get_passed()) { break; }
				board1.do_null_fast();
			}
			else {
				Square m = legal_moves.list[rng.get() % legal_moves.length()];
				board1.do_move_fast(m);
			}
		}

		if (board1.get_passed()) {
			MoveList legal_moves;
			legal_moves.generate(board1);
			if (legal_moves.list == legal_moves.end) {
				i--;
				continue;
			}
		}

		board1.set_squares();
		board1.set_accumulator();

		int score1, score2;

		switch (type) {
		case 0: {
			score1 = play_r(board1, depth_search - 1, false);
			score2 = play_r(board1, depth_search - 1, true);
			break;
		}
		case 1: {
			score1 = play_g(board1, depth_search - 1, false);
			score2 = play_g(board1, depth_search - 1, true);
			break;
		}
		case 2: {
			board2 = board1;
			board2.set_accumulator();

			score1 = play_n(board1, board2, depth_search - 1);
			score2 = play_n(board2, board1, depth_search - 1);
			break;
		}
		}

		score1 = score1 > 0 ? 1 : score1 < 0 ? -1 : 0;
		score2 = score2 > 0 ? -1 : score2 < 0 ? 1 : 0;
		if (score1 + score2 == 0) { res[1]++; }
		else if (score1 + score2 > 0) { res[0]++; }
		else { res[2]++; }
		board1.set(startpos_fen);
	}

	res[0] += win;
	res[1] += draw;
	res[2] += loss;
}


double test_net(int threads, int games, int depth_start, int depth_search, int type,
	 Net* n) 
{
	cout << "Testing with " << threads << " Threads, ";
	switch (type) {
	case 0: {
		cout << "vs Random \n";
		break;
	}
	case 1: {
		cout << "vs Greedy \n";
		break;
	}
	case 2: {
		cout << "vs Net \n";
		break;
	}
	}

	int win  = 0;
	int draw = 0;
	int loss = 0;
	int res[32][3] = {};
	thread* t[32];

	system_clock::time_point time_start = system_clock::now();
	milliseconds time = milliseconds(0);

	for (int i = 0; i < threads; i++) {
		int games_ = (i == 0) ? games - (games / threads) * (threads - 1) : (games / threads);
		t[i] = new thread(test_thread, Threads.n, n, type, games_, depth_start, depth_search, res[i]);
	}

	for (int i = 0; i < threads; i++) {
		t[i]->join();
		win += res[i][0];
		draw += res[i][1];
		loss += res[i][2];
	}

	system_clock::time_point time_now = system_clock::now();
	time = duration_cast<milliseconds>(time_now - time_start);

	double elo_max = log10(double(2 * games) - 1) * 400;

	double elo_diff = (win == games) ? elo_max :
		(loss == games) ? -elo_max : log10(double(games) / (0.01 + loss + double(draw) / 2) - 1) * 400;

	cout << "+" << win << " =" << draw << " -" << loss
		<< "\nelo " << int(elo_diff)
		<< "\ntime " << time.count() << "ms" << endl;

	return elo_diff;
}

void test_batch(string dir, int threads, int games, int depth_start, int depth_search) {
	Net n;
	string name[32];
	double elo_[32][32] = {};
	double elo[2][32] = {};

	int end = 0;
	dir = filesystem::current_path().string() + "/" + dir;
	for (const auto& entry : filesystem::directory_iterator(dir)) {
		if (entry.path().extension() != ".bin") { continue; }
		name[end] = entry.path().string();
		if (end++ >= 32) { break; }
	}
	
	for (int i = 0; i < end; i++) {
		cout << "Weight: " << name[i] << '\n';
		load_weights(Threads.n, name[i]);
		for (int opp = i + 1; opp < end; opp++) {
			load_weights(&n, name[opp]);
			double ed = test_net(threads, games, depth_start, depth_search, 2, &n);
			elo_[i][opp] = ed;
			elo_[opp][i] = -ed;
		}
		cout << endl;
	}

	cout << "Elo: \n";

	int idx_curr = 0;
	for (int it = 0; it < 16; it++) {
		for (int i = 0; i < end; i++) {
			elo[idx_curr][i] = 0;
			for (int j = 0; j < end; j++) {
				elo[idx_curr][i] += elo[idx_curr ^ 1][j] + elo_[i][j];
			}
			elo[idx_curr][i] /= end;
		}
		idx_curr ^= 1;
	}

	for (int i = 0; i < end; i++) {
		cout << filesystem::path(name[i]).filename() << ' ' << elo[idx_curr ^ 1][i] << '\n';
	}
	cout << endl;
}