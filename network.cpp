#include "network.h"

// Read / Write / Modify

void load_weights(Net* net, std::string filename)
{
	char filename_buf[256];
	filename = getcwd_wrap(filename_buf, 256) + filename;

	std::cout << "Loading weights from \"" << filename << "\"\n";
	std::ifstream input(filename, std::ios::binary);

	input.read((char*)net, sizeof(Net));

	if (input.fail() || (input.peek() != EOF)) {
		std::cout << "Failed to load weights" << std::endl;
	}
	else {
		std::cout << "Loaded weights from \"" << filename << "\"" << std::endl;
	}

	input.close();
}

void save_weights(Net* net, std::string filename)
{
	char filename_buf[256];
	filename = getcwd_wrap(filename_buf, 256) + filename;

	std::cout << "Saving weights to \"" << filename << "\"\n";
	std::ofstream output(filename, std::ios::binary);
	
	output.write((char*)net, sizeof(Net));

	if (output.fail()) {
		std::cout << "Failed to save weights" << std::endl;
	}
	else {
		std::cout << "Saved weights to \"" << filename << "\"" << std::endl;
	}

	output.close();
}

void write_weights(Net* net) {
	std::cout << "\n{";
	size_t s = 0;
	while (true) {
		std::cout << "0x" << std::hex << (uint64_t)(*((uint64_t*)(net)+s));
		if ((++s) < sizeof(Net) / sizeof(uint64_t)) {
			std::cout << ", ";
		}
		else { break; }
	}
	std::cout << "};";
}

void rand_weights(Net* net, int bits) {
	if (bits < 0) { return; }
	bits = bits > 7 ? 7 : bits;

	uint8_t mask = uint8_t(~0) >> (8 - bits);

	for (int i = 0; i < SIZE_F0 * SIZE_F1; i++) {
		net->L0_a[i] += (rng.get() & mask) - (rng.get() & mask);
	}
	for (int i = 0; i < SIZE_F1 * SIZE_F2; i++) {
		net->L1_a[i] += (rng.get() & mask) - (rng.get() & mask);
	}
	for (int i = 0; i < SIZE_F2 * SIZE_F3; i++) {
		net->L2_a[i] += (rng.get() & mask) - (rng.get() & mask);
	}
	for (int i = 0; i < SIZE_F3; i++) {
		net->L3_a[i] += (rng.get() & mask) - (rng.get() & mask);
	}
}

void set_material(Net* net) {
	zero_weights(net);
	for (int s = A1; s < SQ_END; s++) {
		for (int i = 0; i < SIZE_F1; i += 2) {
			net->L0_a[s * SIZE_F1 + i                ] = 1 * (1 << SHIFT_L1);
			net->L0_a[s * SIZE_F1 + i + 1 + L0_OFFSET] = 1 * (1 << SHIFT_L1);
		}
	}
	for (int i = 0; i < SIZE_F1; i += 2) {
		for (int j = 0; j < SIZE_F2; j += 2) {
			net->L1_a[j + 0 + (i + 0) * 32] = 2 * (1 << SHIFT_L2) / SIZE_F1;
			net->L1_a[j + 1 + (i + 1) * 32] = 2 * (1 << SHIFT_L2) / SIZE_F1;
		}
	}
	for (int i = 0; i < SIZE_F2; i += 2) {
		for (int j = 0; j < SIZE_F3; j += 2) {
			net->L2_a[j + 0 + (i + 0) * 32] = 2 * (1 << SHIFT_L3);
			net->L2_a[j + 1 + (i + 1) * 32] = 2 * (1 << SHIFT_L3);
		}
	}
	// after L2: SIZE_F2 * count each
	for (int i = 0; i < SIZE_F3; i += 2) {
		net->L3_a[i + 0] =  (1 << (EVAL_BITS - 6 - 9));
		net->L3_a[i + 1] = -(1 << (EVAL_BITS - 6 - 9));
	}
}

void _sub_L0(int16_t* dst, int addr, Net* n) {
#ifdef _AVX256_
	for (int i = 0; i < SIZE_F1; i += 16) {
		__m256i dst_ = _mm256_load_si256((__m256i*)(dst + i));
		__m256i src_ = _mm256_cvtepi8_epi16(_mm_load_si128((__m128i*)(n->L0_a + addr + i)));
		dst_ = _mm256_subs_epi16(dst_, src_);
		_mm256_store_si256((__m256i*)(dst + i), dst_);
	}
#else
	for (int i = 0; i < SIZE_F1; i++) {
		dst[i] -= n->L0_a[addr + i];
	}
#endif
}

void _add_L0(int16_t* dst, int addr, Net* n) {
#ifdef _AVX256_
	for (int i = 0; i < SIZE_F1; i += 16) {
		__m256i dst_ = _mm256_load_si256((__m256i*)(dst + i));
		__m256i src_ = _mm256_cvtepi8_epi16(_mm_load_si128((__m128i*)(n->L0_a + addr + i)));
		dst_ = _mm256_adds_epi16(dst_, src_);
		_mm256_store_si256((__m256i*)(dst + i), dst_);
	}
#else
	for (int i = 0; i < SIZE_F1; i++) {
		dst[i] += n->L0_a[addr + i];
	}
#endif
}

// Evaluation
// L0 idx: (dst = i)
// US   : sq * SIZE_F1 + i
// THEM : sq * SIZE_F1 + i + L0_OFFSET;

void compute_L0(int16_t* dst_b, Piece* squares, Net* n) {
	
	int16_t* dst_w = dst_b + SIZE_F1;
	
	for (int i = 0; i < SIZE_F1; i++) {
		// Bias
		dst_b[i] = n->L0_b[i];
		dst_w[i] = n->L0_b[i];
	}

	for (int s = A1; s < SQ_END; ++s) {
		if (squares[s] == BLACK_P) {
			_add_L0(dst_b, s * SIZE_F1            , n);
			_add_L0(dst_w, s * SIZE_F1 + L0_OFFSET, n);
		}
		else if (squares[s] == WHITE_P) {
			_add_L0(dst_b, s * SIZE_F1 + L0_OFFSET, n);
			_add_L0(dst_w, s * SIZE_F1            , n);
		}
	}
}

void update_L0(int16_t* dst_b, Square s, Piece from, Piece to, Net* n) {
	
	int16_t* dst_w = dst_b + SIZE_F1;

	if (from == BLACK_P) {
		_sub_L0(dst_b, s * SIZE_F1            , n);
		_sub_L0(dst_w, s * SIZE_F1 + L0_OFFSET, n);
	}
	else if (from == WHITE_P) {
		_sub_L0(dst_b, s * SIZE_F1 + L0_OFFSET, n);
		_sub_L0(dst_w, s * SIZE_F1            , n);
	}
	
	if (to == BLACK_P) {
		_add_L0(dst_b, s * SIZE_F1,             n);
		_add_L0(dst_w, s * SIZE_F1 + L0_OFFSET, n);
	}
	else if (to == WHITE_P) {
		_add_L0(dst_b, s * SIZE_F1 + L0_OFFSET, n);
		_add_L0(dst_w, s * SIZE_F1            , n);
	}
}

template <int S, int shift, int max>
void ReLUClip(int16_t* dst, int16_t* src) {
#ifdef _AVX256_
	for (int i = 0; i < S; i += 16) {
		__m256i src_ = _mm256_load_si256((__m256i*)(src + i));
		src_ = _mm256_srai_epi16(src_, shift);
		src_ = _mm256_max_epi16(src_, _mm256_setzero_si256());
		src_ = _mm256_min_epi16(src_, _mm256_set1_epi16(max));
		_mm256_store_si256((__m256i*)(dst + i), src_);
	}
#else
	for (int i = 0; i < S; i++) {
		dst[i] = src[i] >> shift;
		dst[i] = dst[i] < 0 ? 0 :
			dst[i] > max ? max : dst[i];
	}
#endif
}

template <int S, int shift, int max>
void ReLUClip_fallback(int16_t* dst, int16_t* src) {
	for (int i = 0; i < S; i++) {
		dst[i] = src[i] >> shift;
		dst[i] = dst[i] < 0 ? 0 :
			dst[i] > max ? max : dst[i];
	}
}

template <int size_dst, int size_src>
void compute_layer_fallback(int16_t* dst, int16_t* src,
	int8_t* a, int16_t* b)
{
	for (int i = 0; i < size_dst; i += 32) {
		int tmp[32] = { };

		for (int j = 0; j < size_src; j++) {
			for (int k = 0; k < 32; k++) {
				tmp[k] += src[j] * a[j * size_dst + i + k];
			}
		}

		for (int k = 0; k < 32; k++) {
			tmp[k] += b[i + k];
			dst[i + k] = tmp[k] > 32767 ? 32767 :
				tmp[k] < -32768 ? 32768 : tmp[k];
		}
	}
}

template <int size_dst, int size_src>
void compute_layer(int16_t* dst,
	int16_t* src,
	int8_t* a, int16_t* b)
{
#ifdef _AVX256_

	for (int i = 0; i < size_dst; i += 32) {
		__m256i dst1_ = _mm256_setzero_si256();
		__m256i dst2_ = _mm256_setzero_si256();

	for (int j = 0; j < size_src; j += 2) {
		__m256i src1_ = _mm256_set1_epi8(uint8_t(*(src + j)));
		__m256i src2_ = _mm256_set1_epi8(uint8_t(*(src + j + 1)));
		__m256i src_ = _mm256_unpackhi_epi8(src1_, src2_);
		__m256i l1w1_ = _mm256_load_si256((__m256i*)(a +  j      * size_dst + i));
		__m256i l1w2_ = _mm256_load_si256((__m256i*)(a + (j + 1) * size_dst + i));
		__m256i s1 = _mm256_maddubs_epi16(src_, _mm256_unpackhi_epi8(l1w1_, l1w2_));
		__m256i s2 = _mm256_maddubs_epi16(src_, _mm256_unpacklo_epi8(l1w1_, l1w2_));
		dst1_ = _mm256_adds_epi16(dst1_, s1);
		dst2_ = _mm256_adds_epi16(dst2_, s2);
	}

	_mm256_store_si256((__m256i*)(dst + i), _mm256_adds_epi16(
		_mm256_permute2f128_si256(dst2_, dst1_, 0b00100000),
		_mm256_load_si256((__m256i*)(b + i))
	));
	_mm256_store_si256((__m256i*)(dst + i + 16), _mm256_adds_epi16(
		_mm256_permute2f128_si256(dst2_, dst1_, 0b00110001),
		_mm256_load_si256((__m256i*)(b + i + 16))
	));
	}

#else

	compute_layer_fallback<size_dst, size_src>(dst, src, a, b);

#endif
}

void compute_L3(int64_t* dst, int16_t* src, Net* n) {
	for (int j = 0; j < SIZE_OUT; j++) {
		dst[j] = n->L3_b[j];
	}
	for (int i = 0; i < SIZE_F3; i++) {
	for (int j = 0; j < SIZE_OUT; j++) {
		dst[j] += int64_t(src[i]) * n->L3_a[j + SIZE_OUT * i];
	}
	}
}


void compute(int* dst, int16_t* src, Net* n, Color side_to_move) {

	alignas(32)
	int16_t P1[SIZE_F1];
	int16_t P2[SIZE_F2];
	int16_t P3[SIZE_F3];
	int64_t P4[SIZE_OUT];
	
	ReLUClip<SIZE_F1, SHIFT_L1, MAX_L1>(P1, src + (side_to_move ? SIZE_F1 : 0));
	compute_layer<SIZE_F2, SIZE_F1>(P2, P1, n->L1_a, n->L1_b);
	ReLUClip<SIZE_F2, SHIFT_L2, MAX_L2>(P2, P2);
	compute_layer<SIZE_F3, SIZE_F2>(P3, P2, n->L2_a, n->L2_b);
	ReLUClip<SIZE_F3, SHIFT_L3, MAX_L3>(P3, P3);
	compute_L3(P4, P3, n);

	for (int i = 0; i < SIZE_OUT; i++) {
		dst[i] = int(P4[i]);
	}
}

void verify_SIMD(Net* n) {

	alignas(32)
	int16_t P1_ACC[SIZE_F1 * 2];
	int16_t P1_ACC_F[SIZE_F1 * 2];
	int16_t P1[SIZE_F1];
	int16_t P2[SIZE_F2];
	int16_t P2_F[SIZE_F2];

	Piece sq[64] = { };
	
	for (int i = 0; i < SIZE_F1; i++) {
		P1_ACC[i          ] = n->L0_b[i];
		P1_ACC[i + SIZE_F1] = n->L0_b[i];
	}

	Bitboard pieces[3] = {};
	pieces[EMPTY] = FullBoard;

	for (int i = 0; i < 64; i++) {
		Square s = Square(i % 64);
		Piece p = Piece(rng.get() & 3);
		if (p == MISC) { p = EMPTY; }

		pieces[sq[s]] ^= SquareBoard[s];
		pieces[p] ^= SquareBoard[s];

		update_L0(P1_ACC, s, sq[s], p, n);
		sq[s] = p;
	}

	compute_L0(P1_ACC_F, sq, n);

	int p1_err = 0;
	for (int i = 0; i < SIZE_F1 * 2; i++) {
		if (P1_ACC[i] != P1_ACC_F[i]) { p1_err++; }
	}

	std::cout << "update_L0 error: " << p1_err << std::endl;

	ReLUClip<SIZE_F1, SHIFT_L1, MAX_L1>(P1, P1_ACC);
	compute_layer<SIZE_F2, SIZE_F1>(P2, P1, n->L1_a, n->L1_b);
	compute_layer_fallback<SIZE_F2, SIZE_F1>(P2_F, P1, n->L1_a, n->L1_b);

	int p2_err = 0;
	for (int i = 0; i < SIZE_F2; i++) {
		if (P2[i] != P2_F[i]) { p2_err++; }
	}

	std::cout << "compute_Layer error: " << p2_err << std::endl;
}