#include "tune.h"

constexpr int LOSS_SMOOTH = (1 << 12);
constexpr int THREAD_LOOP = 1;
constexpr int SOLVE_DEPTH = 9;

constexpr int AUTOSAVE_S = 1200;

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
	for (int i = 0; i < SIZE_F3 * SIZE_OUT; i++) {
		dst->L3_a[i] = src->L3_a[i];
	}
	for (int i = 0; i < SIZE_OUT; i++) {
		dst->L3_b[i] = src->L3_b[i];
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
	for (int i = 0; i < SIZE_F3 * SIZE_OUT; i++) {
		dst->L3_a[i] = src->L3_a[i];
	}
	for (int i = 0; i < SIZE_OUT; i++) {
		dst->L3_b[i] = src->L3_b[i];
	}

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
	for (int i = 0; i < SIZE_F3 * SIZE_OUT; i += 8) {
		__m256 dst_ = _mm256_load_ps(dst->L3_a + i);
		__m256 src_ = _mm256_load_ps(src->L3_a + i);
		dst_ = _mm256_add_ps(dst_, src_);
		dst_ = _mm256_max_ps(dst_, _mm256_set1_ps(-32767.0f));
		dst_ = _mm256_min_ps(dst_, _mm256_set1_ps(32767.0f));
		_mm256_store_ps(dst->L3_a + i, dst_);
	}
	for (int i = 0; i < SIZE_OUT; i++) {
		dst->L3_b[i] += src->L3_b[i];
	}

	dst->m.unlock();
	src->m.unlock();
}

template <int size_3, int size_2, int max_2, int shift_2>
void back_b_(__m256 c, float* d_3, float* d_2,
	int16_t* p2r, int8_t* a_2, float* b_1) {

	for (int i = 0; i < size_2; i++) {
		if (p2r[i] > (max_2 << shift_2) || p2r[i] < 0) { 
			d_2[i] = 0;
			continue;
		}

		float temp[8];
		__m256 d_2_ = _mm256_setzero_ps();
		for (int j = 0; j < size_3; j += 8) {
			__m256 d_3_ = _mm256_load_ps(d_3 + j);
			__m256 a_2_ = _mm256_cvtepi32_ps(
				_mm256_cvtepi8_epi32(
				_mm_load_si128((__m128i*)(a_2 + j + i * size_3))));
			d_2_ = _mm256_add_ps(d_2_, _mm256_mul_ps(d_3_, a_2_));
		}
		d_2_ = _mm256_hadd_ps(d_2_, _mm256_setzero_ps());
		d_2_ = _mm256_hadd_ps(d_2_, _mm256_setzero_ps());
		d_2_ = _mm256_hadd_ps(d_2_, _mm256_setzero_ps());
		_mm256_store_ps(temp, d_2_);
		d_2[i] = temp[0] / (1 << shift_2);
	}

	c = _mm256_mul_ps(c, _mm256_set1_ps(128.0));
	for (int i = 0; i < size_2; i += 8) {
		__m256 d_2_ = _mm256_load_ps(d_2 + i);
		d_2_ = _mm256_mul_ps(d_2_, c);
		__m256 b_1_ = _mm256_load_ps(b_1 + i);
		b_1_ = _mm256_add_ps(b_1_, d_2_);
		_mm256_store_ps(b_1 + i, b_1_);
	}
}

template <int size_3, int size_2>
void back_a_(__m256 c, float* d_3, float* a_2, int16_t* p2) {
	for (int i = 0; i < size_2; i++) {
		__m256 p2_ = _mm256_set1_ps(float(p2[i]));
		for (int j = 0; j < size_3; j += 8) {
			__m256 d_3_ = _mm256_load_ps(d_3 + j);
			d_3_ = _mm256_mul_ps(d_3_, p2_);
			d_3_ = _mm256_mul_ps(d_3_, c);
			__m256 a2_ = _mm256_load_ps(a_2 + j + i * size_3);
			a2_ = _mm256_add_ps(a2_, d_3_);
			_mm256_store_ps(a_2 + j + i * size_3, a2_);
		}
	}
}

void backpropagate(Net_train* dst, Position* board,
	int score_true, atomic<double>* loss, double learning_rate)
{
	alignas(32)
	int16_t P1[SIZE_F1];
	int16_t P2_RAW[SIZE_F2];
	int16_t P2[SIZE_F2];
	int16_t P3_RAW[SIZE_F3];
	int16_t P3[SIZE_F3];
	int64_t P[SIZE_OUT];
	int64_t PS;

	float mg = float(board->get_count(EMPTY)) / 64;
	float eg = 1 - mg;

	float dPdP3R[SIZE_F3];
	float dPdP2R[SIZE_F2];
	float dPdP1R[SIZE_F1];

	int16_t *acc = board->get_accumulator() + (board->get_side() ? SIZE_F1 : 0);
	Net* n = board->get_net();

	ReLUClip<SIZE_F1, SHIFT_L1, MAX_L1>(P1, acc);
	compute_layer<SIZE_F2, SIZE_F1>(P2_RAW, P1, n->L1_a, n->L1_b);
	ReLUClip<SIZE_F2, SHIFT_L2, MAX_L2>(P2, P2_RAW);
	compute_layer<SIZE_F3, SIZE_F2>(P3_RAW, P2, n->L2_a, n->L2_b);
	ReLUClip<SIZE_F3, SHIFT_L3, MAX_L3>(P3, P3_RAW);
	compute_L3(P, P3, n);
	PS = P[0] * mg + P[1] * eg;

	// -dE/dP = true - curr
	float _coeff = learning_rate * (score_true - PS);

	// clip: only for loss
	PS = PS > EVAL_ALL ? EVAL_ALL : PS < EVAL_NONE ? EVAL_NONE : PS;
	double loss_ = double(score_true - PS) * (score_true - PS);
	*loss = loss_ + (*loss) * (LOSS_SMOOTH - 1) / LOSS_SMOOTH;

	dst->L3_b[0] += _coeff * mg * (1 << 8);
	dst->L3_b[1] += _coeff * eg * (1 << 8);

	__m256 _coeff256 = _mm256_set1_ps(_coeff);

	for (int i = 0; i < SIZE_F3; i++) {
		dst->L3_a[0 + i * SIZE_OUT] += _coeff * mg * P3[i];
		dst->L3_a[1 + i * SIZE_OUT] += _coeff * eg * P3[i];

		dPdP3R[i] = P3_RAW[i] > (MAX_L3 << SHIFT_L3) ? 0.0 :
			P3_RAW[i] < 0 ? 0.0 :
			mg * n->L3_a[i] / (1 << SHIFT_L3) +
			eg * n->L3_a[i + SIZE_F3] / (1 << SHIFT_L3);

		dst->L2_b[i] += _coeff * dPdP3R[i] * (1 << 8);
	}

	back_a_<SIZE_F3, SIZE_F2>(_coeff256, dPdP3R, dst->L2_a, P2);

	back_b_<SIZE_F3, SIZE_F2, MAX_L2, SHIFT_L2>
		(_coeff256, dPdP3R, dPdP2R, P2_RAW, n->L2_a, dst->L1_b);

	back_a_<SIZE_F2, SIZE_F1>(_coeff256, dPdP2R, dst->L1_a, P1);

	back_b_<SIZE_F2, SIZE_F1, MAX_L1, SHIFT_L1>
		(_coeff256, dPdP2R, dPdP1R, acc, n->L1_a, dst->L0_b);

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

struct PBS {
	Net_train* dst_;
	atomic<double>* loss_curr;
	double lr;
	PRNG* r;
};

int _play_best(Position* board1, Position* board2, int find_depth, bool side, PBS* p)
{
	MoveList legal_moves;
	legal_moves.generate(*board1);
	int score_true;

	if (board1->get_count(EMPTY) < SOLVE_DEPTH) { find_depth = SOLVE_DEPTH; }

	if (legal_moves.list == legal_moves.end) {
		if (board1->get_passed()) { 
			// WDL training
			score_true = get_material_eval(*board1);
			score_true = score_true > 0 ? EVAL_ALL :
				score_true < 0 ? EVAL_NONE : 0;
			return score_true;
		}
		Undo u1, u2;
		board1->do_null_move(&u1);
		board2->do_null_move(&u2);
		score_true = -_play_best(board1, board2, find_depth, !side, p);
		board1->undo_null_move();
		board2->undo_null_move();
	}

	else {
		Square s;
		Square nmove = NULL_MOVE;
		int comp_eval;
		int new_eval = EVAL_INIT;
		Position* currboard = side ? board1 : board2;

		for (int i = 0; i < legal_moves.length(); i++) {
			s = legal_moves.list[i];
			Undo u;
			currboard->do_move(s, &u);
			comp_eval = -find_best(*currboard, find_depth,
				EVAL_MIN, -new_eval);

			if (comp_eval > new_eval) {
				new_eval = comp_eval;
				nmove = s;
			}
			currboard->undo_move();
		}

		Undo u1, u2;
		board1->do_move(nmove, &u1);
		board2->do_move(nmove, &u2);
		score_true = -_play_best(board1, board2, find_depth, !side, p);
		board1->undo_move();
		board2->undo_move();
	}

	backpropagate(p->dst_, board1,
		score_true, 
		p->loss_curr, p->lr);

	return score_true;
}

void _do_learning_thread(Net_train* src,
	Position* board, Net* pool, int* wdl, atomic<bool>* stop, 
	atomic<double>* loss, atomic<uint64_t>* games,
	int find_depth, int rand_depth, double lr, PRNG* p) 
{
	Net tmp;
	Net_train dst_;
	board->set_net(&tmp);

	Position* board2[POOL_SIZE];
	for (int i = 0; i < POOL_SIZE; i++) {
		board2[i] = new Position(pool + i);
	}

	while (!(*stop)) {
		convert_to_int(&tmp, src);
		memset(&dst_, 0, sizeof(Net_train));

		for (int rep = 0; rep < THREAD_LOOP; rep++) {
			for (int bidx = 0; bidx < POOL_SIZE; bidx++) {

				board->set(startpos_fen);
				int depth = 0;

				while ((depth < rand_depth) &&
					_play_rand(board, p) > 0) {
					depth++;
				}
				board->set_squares();
				board->set_accumulator();
				*board2[bidx] = *board;
				board2[bidx]->set_accumulator();

				PBS pb = { &dst_, loss, lr, p };
				int score1 =  _play_best(board, board2[bidx], find_depth, true, &pb);
				(*games)++;
				int score2 = -_play_best(board, board2[bidx], find_depth, false, &pb);
				(*games)++;

				int* win = wdl + 2 * bidx;
				int* draw = win + 1;

				if (score1 + score2  > 0) { (*win)++; }
				if (score1 + score2 == 0) { (*draw)++; }
			}
		}

		add_float(src, &dst_);
	}

}

void do_learning(Net* dst, Net* src, Net* pool, int* pool_wdl,
	uint64_t* time_curr, uint64_t* game_curr, uint64_t games,
	int threads, int find_depth, int rand_depth, double lr) {

	PRNG rng_0(3245356235923498ULL);

	Net_train curr;
	convert_to_float(&curr, src);

	atomic<double> loss[THREADS_MAX];
	atomic<uint64_t> games_(0);
	thread thread_[THREADS_MAX];
	Position* boards[THREADS_MAX];
	int save_next = AUTOSAVE_S;

	double loss_;

	using namespace std::chrono;

	system_clock::time_point time_start = system_clock::now();
	milliseconds time = milliseconds(0);

	Threads.stop = false;

	for (int i = 0; i < threads; i++) {
		boards[i] = new Position(nullptr);
		PRNG* n = new PRNG(rng_0.get());
		
		thread_[i] = thread(
			_do_learning_thread,
			&curr, boards[i], pool, pool_wdl + POOL_SIZE * 2 * i, & (Threads.stop),
			loss + i, &games_, find_depth, rand_depth,
			lr, n
		);
	}

	std::cout << std::setprecision(1);

	std::cout << '\n'
		<< "Learning rate: " << std::scientific << lr << '\n'
		<< "NGames: " << games << '\n'
		<< "From: " << rand_depth << '\n'
		<< "Depth: " << find_depth << '\n' << std::endl;

	std::cout << std::setprecision(2);

	while (!(Threads.stop)) {
		std::this_thread::sleep_for(milliseconds(3000));
		loss_ = 0.0;
		for (int i = 0; i < threads; i++) {
			loss_ += loss[i];
		}
		loss_ /= threads;

		system_clock::time_point time_now = system_clock::now();
		time = duration_cast<milliseconds>(time_now - time_start);

		double pos_ = (games_ / threads) * (61 - rand_depth);
		double div_by = (1.0 - pow(double(LOSS_SMOOTH - 1) / LOSS_SMOOTH, pos_)) * LOSS_SMOOTH;
		double accuracy = 100.0 - sqrt(loss_ / div_by) * 50 / EVAL_ALL;
		std::cout
			<< "Time: " << std::setw(7) << *time_curr + time.count() / 1000 << "s // "
			<< "Games: " << std::setw(7) << std::fixed << float((games_ + *game_curr) / 1000) / 1000 << "M // "
			<< "Acc: " << std::setw(5) << accuracy << "% // "
			<< "Loss: " << std::scientific << loss_
			<< std::endl;

		if (time.count() / 1000 > save_next) {
			convert_to_int(dst, &curr);
			save_weights(dst, "temp-" + to_string((*game_curr) / 1000) + "K.bin");
			save_next += AUTOSAVE_S;
		}

		if (games_ > games) { Threads.stop = true; break; }
	}

	for (int i = 0; i < threads; i++) {
		thread_[i].join();
		delete boards[i];
	}

	*time_curr += time.count() / 1000;
	*game_curr += games_;

	convert_to_int(dst, &curr);
	save_weights(dst, "temp.bin");
}

void do_learning_cycle(Net* dst, Net* src, uint64_t* game_switch,
	int threads, int* find_depth, int* rand_depth, double* lr, int cycles)
{
	int count = 0;
	uint64_t game_curr = 0;
	uint64_t time_curr = 0;

	Net pool[POOL_SIZE];
	int pool_wdl[THREADS_MAX * POOL_SIZE * 2];

	cout << "Initializing Pool...\n";
	for (int i = 0; i < POOL_SIZE; i++) {
		memcpy(pool + i, src, sizeof(Net));
		rand_weights(pool + i, 1);
	}

	while (true) {
		cout << "Playing Pool... ";
		memset(pool_wdl, 0, sizeof(pool_wdl));

		for (int i = 0; i < cycles; i++) {
			do_learning(dst, src, pool, pool_wdl,
				&time_curr, &game_curr, game_switch[i],
				threads, find_depth[i], rand_depth[i], lr[i]);
		}

		save_weights(dst, "ep" + to_string(count)
			+ "-" + to_string(game_curr / 1000) + "K.bin");

		cout << "Updating Pool... ";
		for (int i = 1; i < threads; i++) {
			for (int j = 0; j < POOL_SIZE; j++) {
				pool_wdl[2 * j + 0] += pool_wdl[2 * POOL_SIZE * i + 2 * j + 0];
				pool_wdl[2 * j + 1] += pool_wdl[2 * POOL_SIZE * i + 2 * j + 1];
			}
		}

		int least_score = -1;
		int least_idx = 0;
		for (int i = 0; i < POOL_SIZE; i++) {
			int curr_score = 2 * pool_wdl[i * 2 + 0] + pool_wdl[i * 2 + 1];
			if (curr_score > least_score) {
				least_score = curr_score;
				least_idx = i;
			}
		}

		memcpy(pool + least_idx, dst, sizeof(Net));
		cout << least_idx << endl;

		count++;
	}
}