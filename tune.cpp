#include "tune.h"

constexpr int LOSS_SMOOTH = 8192;
constexpr int THREAD_LOOP = 8;

void convert_to_float(Net_train* dst, Net* src) 
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
	dst->L3_b = src->L3_b;

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
	dst->L3_b = src->L3_b;

	src->m.unlock();
}

void add_float(Net_train* dst, Net_train* src) {

	src->m.lock();
	dst->m.lock();

	for (int i = 0; i < SIZE_F0 * SIZE_F1; i += 8) {
		__m256 dst_ = _mm256_load_ps(dst->L0_a + i);
		__m256 src_ = _mm256_load_ps(src->L0_a + i);
		dst_ = _mm256_add_ps(dst_, src_);
		dst_ = _mm256_max_ps(dst_, _mm256_set1_ps(-16383.0f));
		dst_ = _mm256_min_ps(dst_, _mm256_set1_ps(16383.0f));
		_mm256_store_ps(dst->L0_a + i, dst_);
	}
	for (int i = 0; i < SIZE_F1; i += 8) {
		__m256 dst_ = _mm256_load_ps(dst->L0_b + i);
		__m256 src_ = _mm256_load_ps(src->L0_b + i);
		dst_ = _mm256_add_ps(dst_, src_);
		dst_ = _mm256_max_ps(dst_, _mm256_set1_ps(-16383.0f));
		dst_ = _mm256_min_ps(dst_, _mm256_set1_ps(16383.0f));
		_mm256_store_ps(dst->L0_b + i, dst_);
	}
	for (int i = 0; i < SIZE_F1 * SIZE_F2; i += 8) {
		__m256 dst_ = _mm256_load_ps(dst->L1_a + i);
		__m256 src_ = _mm256_load_ps(src->L1_a + i);
		dst_ = _mm256_add_ps(dst_, src_);
		dst_ = _mm256_max_ps(dst_, _mm256_set1_ps(-127.0f));
		dst_ = _mm256_min_ps(dst_, _mm256_set1_ps(127.0f));
		_mm256_store_ps(dst->L1_a + i, dst_);
	}
	for (int i = 0; i < SIZE_F2; i += 8) {
		__m256 dst_ = _mm256_load_ps(dst->L1_b + i);
		__m256 src_ = _mm256_load_ps(src->L1_b + i);
		dst_ = _mm256_add_ps(dst_, src_);
		dst_ = _mm256_max_ps(dst_, _mm256_set1_ps(-16383.0f));
		dst_ = _mm256_min_ps(dst_, _mm256_set1_ps(16383.0f));
		_mm256_store_ps(dst->L1_b + i, dst_);
	}
	for (int i = 0; i < SIZE_F2 * SIZE_F3; i += 8) {
		__m256 dst_ = _mm256_load_ps(dst->L2_a + i);
		__m256 src_ = _mm256_load_ps(src->L2_a + i);
		dst_ = _mm256_add_ps(dst_, src_);
		dst_ = _mm256_max_ps(dst_, _mm256_set1_ps(-127.0f));
		dst_ = _mm256_min_ps(dst_, _mm256_set1_ps(127.0f));
		_mm256_store_ps(dst->L2_a + i, dst_);
	}
	for (int i = 0; i < SIZE_F3; i += 8) {
		__m256 dst_ = _mm256_load_ps(dst->L2_b + i);
		__m256 src_ = _mm256_load_ps(src->L2_b + i);
		dst_ = _mm256_add_ps(dst_, src_);
		dst_ = _mm256_max_ps(dst_, _mm256_set1_ps(-16383.0f));
		dst_ = _mm256_min_ps(dst_, _mm256_set1_ps(16383.0f));
		_mm256_store_ps(dst->L2_b + i, dst_);
	}
	for (int i = 0; i < SIZE_F3; i += 8) {
		__m256 dst_ = _mm256_load_ps(dst->L3_a + i);
		__m256 src_ = _mm256_load_ps(src->L3_a + i);
		dst_ = _mm256_add_ps(dst_, src_);
		dst_ = _mm256_max_ps(dst_, _mm256_set1_ps(-32767.0f));
		dst_ = _mm256_min_ps(dst_, _mm256_set1_ps(32767.0f));
		_mm256_store_ps(dst->L3_a + i, dst_);
	}

	dst->L3_b += src->L3_b;
	dst->L3_b = dst->L3_b > (1 << EVAL_BITS) ? (1 << EVAL_BITS) :
		dst->L3_b < -(1 << EVAL_BITS) ? -(1 << EVAL_BITS) :
		dst->L3_b;

	dst->m.unlock();
	src->m.unlock();
}

void backpropagate(Net_train* dst, Position* board,
	int score_true, double* loss, double learning_rate)
{

	alignas(32)
	int16_t P1[SIZE_F1];
	int16_t P2_RAW[SIZE_F2];
	int16_t P2[SIZE_F2];
	int16_t P3_RAW[SIZE_F3];
	int16_t P3[SIZE_F3];
	int64_t P;

	float dPdP3R[SIZE_F3];
	float dPdP2R[SIZE_F2];
	float dPdP1R[SIZE_F1];

	int16_t *acc = board->get_accumulator() + (board->get_side() ? SIZE_F1 : 0);
	Net* n = board->get_net();

	ReLUClip<SIZE_F1, SHIFT_L0, MAX_L1>(P1, acc);
	compute_layer<SIZE_F2, SIZE_F1>(P2_RAW, P1, n->L1_a, n->L1_b);
	ReLUClip<SIZE_F2, SHIFT_L1, MAX_L2>(P2, P2_RAW);
	compute_layer<SIZE_F3, SIZE_F2>(P3_RAW, P2, n->L2_a, n->L2_b);
	ReLUClip<SIZE_F3, SHIFT_L2, MAX_L3>(P3, P3_RAW);
	compute_L3(&P, P3, n);

	// -dE/dP = true - curr
	float _coeff = learning_rate * (score_true - P);

	// clip: only for loss
	P = P > EVAL_ALL ? EVAL_ALL : P < EVAL_NONE ? EVAL_NONE : P;
	*loss = 1.0 * (score_true - P) * (score_true - P);

	dst->L3_b += _coeff;

	__m256 _coeff256 = _mm256_set1_ps(_coeff);

	for (int i = 0; i < SIZE_F3; i += 8) {
		__m128i __src16 = _mm_load_si128((__m128i*)(P3 + i));
		__m256i __src32 = _mm256_cvtepi16_epi32(__src16);
		__m256 _src = _mm256_cvtepi32_ps(__src32);
		__m256 _dst = _mm256_load_ps(dst->L3_a + i);
		
		_src = _mm256_mul_ps(_coeff256, _src);
		_dst = _mm256_add_ps(_dst, _src);
		_mm256_store_ps(dst->L3_a + i, _dst);
	}

	for (int i = 0; i < SIZE_F3; i ++) {
		dPdP3R[i] = P3_RAW[i] > (MAX_L3 << SHIFT_L2) ? 0.0 :
			P3_RAW[i] < 0 ? 0.0 : 1.0;

		dPdP3R[i] *= n->L3_a[i] / (1 << SHIFT_L2);

		dst->L2_b[i] += _coeff * dPdP3R[i];
	}

	for (int j = 0; j < SIZE_F2; j++) {
		for (int i = 0; i < SIZE_F3; i++) {
			dst->L2_a[i + j * SIZE_F3] += _coeff * dPdP3R[i] * P2[j];
		}
	}

	for (int i = 0; i < SIZE_F2; i++) {
		double dP2dP2R = P2_RAW[i] > (MAX_L2 << SHIFT_L1) ? 0.0 :
			P2_RAW[i] < 0 ? 0.0 : 1.0;

		dPdP2R[i] = 0;
		for (int k = 0; k < SIZE_F3; k++) {
			dPdP2R[i] += dPdP3R[k] * n->L2_a[k + i * SIZE_F3];
		}
		dPdP2R[i] *= dP2dP2R / (1 << SHIFT_L1);

		dst->L1_b[i] += _coeff * dPdP2R[i];
	}

	for (int j = 0; j < SIZE_F1; j++) {
		for (int i = 0; i < SIZE_F2; i++) {
			dst->L1_a[i + j * SIZE_F2] += _coeff * dPdP2R[i] * P1[j];
		}
	}

	for (int i = 0; i < SIZE_F1; i++) {
		double dP1dP1R = acc[i] > (MAX_L1 << SHIFT_L0) ? 0.0 :
			acc[i] < 0 ? 0.0 : 1.0;

		dPdP1R[i] = 0;
		for (int k = 0; k < SIZE_F2; k++) {
			dPdP1R[i] += dPdP2R[k] * n->L1_a[k + i * SIZE_F2];
		}
		dPdP1R[i] *= dP1dP1R / (1 << SHIFT_L0);

		dst->L0_b[i] += _coeff * dPdP1R[i];
	}

	Piece us = board->get_side() ? WHITE_P : BLACK_P;

	for (Square s = A1; s < SQ_END; ++s) {
		if (board->get_piece(s) == EMPTY) {	continue; }

		int addr = board->get_piece(s) == us ?
			s * SIZE_F1 : s * SIZE_F1 + L0_OFFSET;

		for (int i = 0; i < SIZE_F1; i += 8) {
			__m256 _src = _mm256_load_ps(dPdP1R + i);
			__m256 _dst = _mm256_load_ps(dst->L0_a + addr + i);

			_src = _mm256_mul_ps(_coeff256, _src);
			_dst = _mm256_add_ps(_dst, _src);
			_mm256_store_ps(dst->L0_a + addr + i, _dst);
		}
	}
}

int _solve_learning(Position* board)
{

	MoveList legal_moves;
	legal_moves.generate(*board);

	if (legal_moves.list == legal_moves.end) {

		if (board->get_passed()) {
			int bc = popcount(board->get_pieces(BLACK_P));
			int wc = popcount(board->get_pieces(WHITE_P));
			return board->get_side() ? wc - bc : bc - wc;
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
		int new_eval = EVAL_INIT;

		for (int i = 0; i < legal_moves.length(); i++) {
			s = legal_moves.list[i];

			// Do move
			Undo u;
			board->do_move(s, &u);

			int comp_eval = -_solve_learning(board);

			if (comp_eval > new_eval) {
				new_eval = comp_eval;
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
			board->do_null_fast(u);
			u->del = true;
			return 1;
		}
	}

	else {
		Square s = legal_moves.list[rng.get() % legal_moves.length()];
		Undo* u = new Undo;
		board->do_move_fast(s, u);
		u->del = true;
		return 1;
	}
}

int _play_best(Position* board, int find_depth)
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

		Undo* u = new Undo;
		board->do_move(nmove, u);
		u->del = true;

		return 1;
	}
}

const char* startpos_fen_ = "8/8/8/3@O3/3O@3/8/8/8 b";

void _do_learning_thread(Net_train* src,
	Position* board, atomic<bool>* stop, 
	atomic<double>* loss, atomic<uint64_t>* games,
	int find_depth, double lr, PRNG p) 
{
	Net tmp;
	Net_train dst_;
	board->set_net(&tmp);

	double loss_total, loss_curr, loss_base;
	atomic<double>* loss_base_dst = loss + 32;

	while (!(*stop)) {
		convert_to_int(&tmp, src);
		memset(&dst_, 0, sizeof(Net_train));

		for (int rep = 0; rep < THREAD_LOOP; rep++) {
			board->set(startpos_fen_);
			int depth = 0;

			while ((depth < 60 - find_depth) &&
				_play_rand(board, &p) > 0) {
				depth++;
			}
			board->set_squares();
			board->set_accumulator();
			int depth_begin = depth;

			while (_play_best(board, find_depth) > 0) {
				depth++;
			}
			int depth_end = depth;

			int score_true = _solve_learning(board);

			// WDL training
			score_true = score_true > 0 ? EVAL_ALL :
				score_true < 0 ? EVAL_NONE : 0;

			loss_total = 0;
			loss_base = 0;
			// skip last null move
			if (depth > depth_begin) { 
				score_true = -score_true;
				board->undo_move();
			}

			while (true) {
				backpropagate(&dst_, board,
					score_true, &loss_curr, lr);

				loss_total += loss_curr;

				int score_base = get_material(*board);
				score_base = score_base > 0 ? EVAL_ALL :
					score_base < 0 ? EVAL_NONE : 0;
				loss_base += double(score_true - score_base) * (score_true - score_base);

				score_true = -score_true;
				depth--;
				if (depth > depth_begin) { board->undo_move(); }
				else { break; }
			}
			loss_total /= (depth_end - depth_begin + 1);
			loss_base  /= (depth_end - depth_begin + 1);

			*loss = loss_total + (*loss) * (LOSS_SMOOTH - 1) / LOSS_SMOOTH;
			*loss_base_dst = loss_base + (*loss_base_dst) * (LOSS_SMOOTH - 1) / LOSS_SMOOTH;
			
			(*games)++;
		}

		add_float(src, &dst_);
	}

}

void do_learning(Net* dst, Net* src, uint64_t games, int threads, int find_depth, double lr) {

	PRNG rng_0(3245356235923498ULL);

	Net_train curr;
	convert_to_float(&curr, src);

	atomic<double> loss[64];
	atomic<uint64_t> games_(0);
	thread thread_[64];
	Position* boards[64];

	double loss_, loss_base_;

	using namespace std::chrono;

	system_clock::time_point time_start = system_clock::now();
	milliseconds time = milliseconds(0);

	Threads.stop = false;

	for (int i = 0; i < threads; i++) {
		boards[i] = new Position(nullptr);
		
		thread_[i] = thread(
			_do_learning_thread,
			&curr, boards[i], &(Threads.stop), 
			loss + i, &games_, find_depth,
			lr, PRNG(rng_0.get())
		);
	}

	std::cout << std::setprecision(1);

	std::cout
		<< "Learning with: " << threads << " Threads\n"
		<< "Learning rate: " << std::scientific << lr << '\n'
		<< "Max Games: " << games << '\n' 
		<< "Depth: " << find_depth << '\n' << std::endl;

	std::cout << std::setprecision(2);

	while (!(Threads.stop) && (games_ < games)) {
		std::this_thread::sleep_for(milliseconds(3000));
		loss_ = 0.0;
		loss_base_ = 0.0;
		for (int i = 0; i < threads; i++) {
			loss_ += loss[i];
			loss_base_ += loss[i + 32];
		}
		loss_ /= threads;
		loss_base_ /= threads;

		system_clock::time_point time_now = system_clock::now();
		time = duration_cast<milliseconds>(time_now - time_start);

		std::cout
			<< "Time: " << std::setw(7) << time.count() / 1000 << "s // "
			<< "Games: " << std::setw(7) << std::fixed << float(games_ / 1000) / 1000 << "M // "
			<< "Acc: " << std::setw(5) << 100 - sqrt(loss_ / LOSS_SMOOTH) * 50 / EVAL_ALL << "% // "
			<< "Acc_Baseline: " << std::setw(5) << 100 - sqrt(loss_base_ / LOSS_SMOOTH) * 50 / EVAL_ALL << "% // "
			<< "Loss: " << std::scientific << loss_
			<< std::endl;
	}

	for (int i = 0; i < threads; i++) {
		delete boards[i];

		thread_[i].join();
	}

	convert_to_int(dst, &curr);
	save_weights(dst, "temp.bin");
}