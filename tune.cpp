#include "tune.h"

void convert_to_double(Net_train* dst, Net* src) 
{
	dst->m.lock();

	for (int i = 0; i < SIZE_F0 * SIZE_F1; i++) {
		dst->L0_a[i] = src->L0_a[i];
	}
	for (int i = 0; i < SIZE_F1; i++) {
		dst->L0_b[i] = src->L0_b[i];
	}
	for (int i = 0; i < SIZE_F1 * SIZE_F2; i++) {
		dst->L1_a[i] = src->L1_a[i];
	}
	for (int i = 0; i < SIZE_F2; i++) {
		dst->L1_b[i] = src->L1_b[i];
	}
	for (int i = 0; i < SIZE_F2 * SIZE_F3; i++) {
		dst->L2_a[i] = src->L2_a[i];
	}
	for (int i = 0; i < SIZE_F3; i++) {
		dst->L2_b[i] = src->L2_b[i];
	}
	for (int i = 0; i < SIZE_F3; i++) {
		dst->L3_a[i] = src->L3_a[i];
	}

	dst->m.unlock();
}

void convert_to_int(Net* dst, Net_train* src) 
{
	src->m.lock();

	for (int i = 0; i < SIZE_F0 * SIZE_F1; i++) {
		dst->L0_a[i] = src->L0_a[i];
	}
	for (int i = 0; i < SIZE_F1; i++) {
		dst->L0_b[i] = src->L0_b[i];
	}
	for (int i = 0; i < SIZE_F1 * SIZE_F2; i++) {
		dst->L1_a[i] = src->L1_a[i];
	}
	for (int i = 0; i < SIZE_F2; i++) {
		dst->L1_b[i] = src->L1_b[i];
	}
	for (int i = 0; i < SIZE_F2 * SIZE_F3; i++) {
		dst->L2_a[i] = src->L2_a[i];
	}
	for (int i = 0; i < SIZE_F3; i++) {
		dst->L2_b[i] = src->L2_b[i];
	}
	for (int i = 0; i < SIZE_F3; i++) {
		dst->L3_a[i] = src->L3_a[i];
	}

	src->m.unlock();
}

void copy_double(Net_train* dst, Net_train* src) {
	
	src->m.lock();
	dst->m.lock();

	for (int i = 0; i < SIZE_F0 * SIZE_F1; i++) {
		dst->L0_a[i] = src->L0_a[i];
	}
	for (int i = 0; i < SIZE_F1; i++) {
		dst->L0_b[i] = src->L0_b[i];
	}
	for (int i = 0; i < SIZE_F1 * SIZE_F2; i++) {
		dst->L1_a[i] = src->L1_a[i];
	}
	for (int i = 0; i < SIZE_F2; i++) {
		dst->L1_b[i] = src->L1_b[i];
	}
	for (int i = 0; i < SIZE_F2 * SIZE_F3; i++) {
		dst->L2_a[i] = src->L2_a[i];
	}
	for (int i = 0; i < SIZE_F3; i++) {
		dst->L2_b[i] = src->L2_b[i];
	}
	for (int i = 0; i < SIZE_F3; i++) {
		dst->L3_a[i] = src->L3_a[i];
	}

	dst->m.unlock();
	src->m.unlock();
}

void add_double(Net_train* dst, Net_train* src) {

	src->m.lock();
	dst->m.lock();

	for (int i = 0; i < SIZE_F0 * SIZE_F1; i++) {
		dst->L0_a[i] += src->L0_a[i];
		dst->L0_a[i] = dst->L0_a[i] > 127 ? 127.0 :
			dst->L0_a[i] < -127 ? -127.0 : dst->L0_a[i];
	}
	for (int i = 0; i < SIZE_F1; i++) {
		dst->L0_b[i] += src->L0_b[i];
		dst->L0_b[i] = dst->L0_b[i] > 127 ? 127.0 :
			dst->L0_b[i] < -127 ? -127.0 : dst->L0_b[i];
	}
	for (int i = 0; i < SIZE_F1 * SIZE_F2; i++) {
		dst->L1_a[i] += src->L1_a[i];
		dst->L1_a[i] = dst->L1_a[i] > 127 ? 127.0 :
			dst->L1_a[i] < -127 ? -127.0 : dst->L1_a[i];
	}
	for (int i = 0; i < SIZE_F2; i++) {
		dst->L1_b[i] += src->L1_b[i];
		dst->L1_b[i] = dst->L1_b[i] > 16383 ? 16383.0 :
			dst->L1_b[i] < -16383 ? -16383.0 : dst->L1_b[i];
	}
	for (int i = 0; i < SIZE_F2 * SIZE_F3; i++) {
		dst->L2_a[i] += src->L2_a[i];
		dst->L2_a[i] = dst->L2_a[i] > 127 ? 127.0 :
			dst->L2_a[i] < -127 ? 127.0 : dst->L2_a[i];
	}
	for (int i = 0; i < SIZE_F3; i++) {
		dst->L2_b[i] += src->L2_b[i];
		dst->L2_b[i] = dst->L2_b[i] > 16383 ? 16383.0 :
			dst->L2_b[i] < -16383 ? -16383.0 : dst->L2_b[i];
	}
	for (int i = 0; i < SIZE_F3; i++) {
		dst->L3_a[i] += src->L3_a[i];
		dst->L3_a[i] = dst->L3_a[i] > 16383 ? 16383.0 :
			dst->L3_a[i] < -16383 ? -16383.0 : dst->L3_a[i];
	}

	dst->m.unlock();
	src->m.unlock();
}

void backpropagate(Net_train* dst, Net_train* src, Position* board,
	int score_true, double* loss, double learning_rate)
{

	alignas(32)
	int16_t P1[SIZE_F1];
	int16_t P2_RAW[SIZE_F2];
	int16_t P2[SIZE_F2];
	int16_t P3_RAW[SIZE_F3];
	int16_t P3[SIZE_F3];
	int64_t P_RAW;
	int P; // score_curr

	double dPdP3R[SIZE_F3];
	double dPdP2R[SIZE_F2];
	double dPdP1R[SIZE_F1];

	int16_t *acc = board->get_accumulator();
	Net* n = board->get_net();
	
	ReLUClip_L0(P1, acc, board->get_side());
	compute_layer(P2_RAW, P1, n->L1_a, n->L1_b);
	ReLUClip_L1(P2, P2_RAW);
	compute_layer(P3_RAW, P2, n->L2_a, n->L2_b);
	ReLUClip_L2(P3, P3_RAW);
	compute_L3(&P_RAW, P3, n);
	ReLUClip_L3(&P, &P_RAW);

	// -dE/dP = true - curr
	double _coeff = learning_rate * (score_true - P);
	*loss = double(abs(score_true - P));

	// dP/dP_RAW
	_coeff *= P_RAW > (1 << EVAL_BITS) ? 0.1 :
		P_RAW < -(1 << EVAL_BITS) ? 0.1 :
		1.0;

	for (int i = 0; i < SIZE_F3; i++) {
		dst->L3_a[i] += _coeff * P3[i];
	}

	for (int i = 0; i < SIZE_F3; i++) {
		dPdP3R[i] = P3_RAW[i] > 16383 ? 0.1 :
			P3_RAW[i] < 0 ? 0.1 : 1.0;

		dPdP3R[i] *= n->L3_a[i];

		dst->L2_b[i] += _coeff * dPdP3R[i];
	}

	for (int j = 0; j < SIZE_F2; j++) {
		for (int i = 0; i < SIZE_F3; i++) {
			dst->L2_a[i + j * SIZE_F3] += _coeff * dPdP3R[i] * P2[j];
		}
	}

	for (int i = 0; i < SIZE_F2; i++) {
		double dP2dP2R = P2_RAW[i] > 127 ? 0.1 :
			P2_RAW[i] < 0 ? 0.1 : 1.0;

		dPdP2R[i] = 0;
		for (int k = 0; k < SIZE_F3; k++) {
			dPdP2R[i] += dPdP3R[k] * n->L2_a[k + i * SIZE_F3];
		}
		dPdP2R[i] *= dP2dP2R / 256;

		dst->L1_b[i] += _coeff * dPdP2R[i];
	}

	for (int j = 0; j < SIZE_F1; j++) {
		for (int i = 0; i < SIZE_F2; i++) {
			dst->L1_a[i + j * SIZE_F2] += _coeff * dPdP2R[i] * P1[j];
		}
	}
	
	if (board->get_side()) { acc += 32; }

	for (int i = 0; i < SIZE_F1; i++) {
		double dP1dP1R = acc[i] > 127 ? 0.1 :
			acc[i] < 0 ? 0.1 : 1.0;

		dPdP1R[i] = 0;
		for (int k = 0; k < SIZE_F2; k++) {
			dPdP1R[i] += dPdP2R[k] * n->L1_a[k + i * SIZE_F3];
		}
		dPdP1R[i] *= dP1dP1R / 256;

		dst->L0_b[i] += _coeff * dPdP1R[i];
	}

	if (!(board->get_side())) {
		for (int j = 0; j < SIZE_F0 / 2; j++) {
			for (int i = 0; i < SIZE_F1; i++) {
				dst->L0_a[2 * i + j * 2 * SIZE_F1    ] += _coeff * dPdP1R[i]
					* (board->get_piece(Square(j)) == BLACK_P);
				dst->L0_a[2 * i + j * 2 * SIZE_F1 + 1] += _coeff * dPdP1R[i]
					* (board->get_piece(Square(j)) == WHITE_P);
			}
		}
	}
	else {
		for (int j = 0; j < SIZE_F0 / 2; j++) {
			for (int i = 0; i < SIZE_F1; i++) {
				dst->L0_a[2 * i + j * 2 * SIZE_F1    ] += _coeff * dPdP1R[i]
					* (board->get_piece(Square(j)) == WHITE_P);
				dst->L0_a[2 * i + j * 2 * SIZE_F1 + 1] += _coeff * dPdP1R[i]
					* (board->get_piece(Square(j)) == BLACK_P);
			}
		}
	}
}

int _solve_learning(Position* board) {
	// Move generation
	MoveList legal_moves;
	legal_moves.generate(*board);

	// No legal moves
	if (legal_moves.list == legal_moves.end) {

		if (board->get_passed()) {
			return get_material(*board);
		}

		else {
			Undo u;
			board->do_null_move(&u);
			int after_pass = -_solve_learning(board);
			board->undo_null_move();
			return after_pass;
		}
	}

	// Legal moves
	else {
		Square s;
		Square nmove = NULL_MOVE;
		int comp_eval;
		int new_eval = EVAL_INIT;

		for (int i = 0; i < legal_moves.length(); i++) {
			s = legal_moves.list[i];

			// Do move
			Undo u;
			board->do_move(s, &u);

			comp_eval = -_solve_learning(board);

			if (comp_eval > new_eval) {
				new_eval = comp_eval;
				nmove = s;
			}

			board->undo_move();
		}

		return new_eval;
	}
}

int _play_rand(Position* board, PRNG* rng_) 
{	

	MoveList legal_moves;
	legal_moves.generate(*board);

	if (legal_moves.list == legal_moves.end) {

		if (board->get_passed()) {
			return 0;
		}

		else {
			Undo* u = new Undo;
			board->do_null_move(u);
			u->del = true;

			return 1;
		}
	}

	// Legal moves
	else {

		Square s = legal_moves.list[rng.get() % legal_moves.length()];

		Undo* u = new Undo;
		board->do_move(s, u);
		u->del = true;

		return 1;
	}
}

const char* startpos_fen_ = "8/8/8/3@O3/3O@3/8/8/8 b";

void _do_learning_thread(Net_train* src,
	Position* board, atomic<bool>* stop, 
	atomic<double>* loss, atomic<uint64_t>* games,
	double lr, PRNG p) 
{
	Net tmp;
	Net_train src_;
	Net_train dst_;

	int score[64];
	double loss_total, loss_curr;

	while (!(*stop)) {
		copy_double(&src_, src);
		convert_to_int(&tmp, &src_);
		board->set_net(&tmp);
		memset(&dst_, 0, sizeof(Net_train));


		board->set(startpos_fen_);
		int depth = 0;

		while (board->get_count(EMPTY) > 0 &&
			_play_rand(board, &p) > 0) { depth++; }

		int depth_end = depth;

		int score_true = _solve_learning(board);

		loss_total = 0;
		while (--depth > 0 && (depth_end - depth) < 2) {
			backpropagate(&dst_, &src_, board,
				score_true, &loss_curr, lr);

			loss_total += loss_curr;

			board->undo_move();
		}
		loss_total /= (depth_end - depth);

		while (--depth > 0) { board->undo_move(); }

		*loss = loss_total + (*loss) * (LOSS_SMOOTH - 1) / LOSS_SMOOTH;
		(*games)++;

		add_double(src, &dst_);
	}

}

void do_learning(Net* dst, Net* src, int64_t games, int threads, double lr) {

	PRNG rng_0(3245356235923498ULL);

	Net_train curr;
	convert_to_double(&curr, src);

	atomic<double> loss[64];
	atomic<uint64_t> games_(0);
	thread thread_[64];
	Position* boards[64];

	double loss_;

	using namespace std::chrono;

	system_clock::time_point time_start = system_clock::now();
	milliseconds time = milliseconds(0);

	Threads.stop = false;

	for (int i = 0; i < threads; i++) {
		boards[i] = new Position(nullptr);
		
		thread_[i] = thread(
			_do_learning_thread,
			&curr, boards[i], &(Threads.stop), 
			loss + i, &games_,
			lr, PRNG(rng_0.get())
		);
	}

	std::cout << std::scientific;

	std::cout
		<< "Tuning with: " << threads << " Threads\n"
		<< "Learning rate: " << std::setw(12) << lr << '\n'
		<< "Max Games: " << games << '\n' << std::endl;

	while (!(Threads.stop) && (games == -1 || (games_ < games))) {
		loss_ = 0.0;
		for (int i = 0; i < threads; i++) {
			loss_ += loss[i];
		}
		loss_ /= threads;

		system_clock::time_point time_now = system_clock::now();
		time = duration_cast<milliseconds>(time_now - time_start);

		std::cout
			<< "Elapsed Time: " << std::setw(12) << time.count() << "ms // "
			<< "Games: " << std::setw(12) << games_ << " // "
			<< "Loss: " << loss_ 
			<< " (" << loss_ / LOSS_SMOOTH / (1 << 23) << " avg d)"
			<< std::endl;

		std::this_thread::sleep_for(milliseconds(3000));
	}

	for (int i = 0; i < threads; i++) {
		delete boards[i];

		thread_[i].join();
	}

	convert_to_int(dst, &curr);
	save_weights(dst, "temp.bin");
}