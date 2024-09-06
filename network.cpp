#include "network.h"

// Read / Write / Modify

void load_weights(Net* net, std::string filename)
{
	char filename_buf[256];
	filename = getcwd_wrap(filename_buf, 256) + filename;

	std::cout << "Loading weights from \"" << filename << "\"\n";
	std::ifstream input(filename, std::ios::binary);

	input.read((char*)net->L0_a, SIZE_F0 * SIZE_F1);
	input.read((char*)net->L0_b, SIZE_F1 * 2);
	input.read((char*)net->L1_a, SIZE_F1 * SIZE_F2);
	input.read((char*)net->L1_b, SIZE_F2 * 2);
	input.read((char*)net->L2_a, SIZE_F2 * SIZE_F3);
	input.read((char*)net->L2_b, SIZE_F3 * 2);
	input.read((char*)net->L3_a, SIZE_F3 * 2);

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
	
	output.write((char*)net->L0_a, SIZE_F0 * SIZE_F1);
	output.write((char*)net->L0_b, SIZE_F1 * 2);
	output.write((char*)net->L1_a, SIZE_F1 * SIZE_F2);
	output.write((char*)net->L1_b, SIZE_F2 * 2);
	output.write((char*)net->L2_a, SIZE_F2 * SIZE_F3);
	output.write((char*)net->L2_b, SIZE_F3 * 2);
	output.write((char*)net->L3_a, SIZE_F3 * 2);

	if (output.fail()) {
		std::cout << "Failed to save weights" << std::endl;
	}
	else {
		std::cout << "Saved weights to \"" << filename << "\"" << std::endl;
	}

	output.close();
}

void rand_weights(Net* net, int bits) {
	if (bits < 0) { return; }
	bits = bits > 7 ? 7 : bits;

	uint8_t mask = uint8_t(~0) >> (8 - bits);

	for (int i = 0; i < SIZE_F0 * SIZE_F1; i++) {
		net->L0_a[i] += (rng.get() & mask) - (rng.get() & mask);
	}
	for (int i = 0; i < SIZE_F1; i++) {
		net->L0_b[i] += (rng.get() & mask) - (rng.get() & mask);
	}
	for (int i = 0; i < SIZE_F1 * SIZE_F2; i++) {
		net->L1_a[i] += (rng.get() & mask) - (rng.get() & mask);
	}
	for (int i = 0; i < SIZE_F2; i++) {
		net->L1_b[i] += (rng.get() & mask) - (rng.get() & mask);
	}
	for (int i = 0; i < SIZE_F2 * SIZE_F3; i++) {
		net->L2_a[i] += (rng.get() & mask) - (rng.get() & mask);
	}
	for (int i = 0; i < SIZE_F3; i++) {
		net->L2_b[i] += (rng.get() & mask) - (rng.get() & mask);
	}
	for (int i = 0; i < SIZE_F3; i++) {
		net->L3_a[i] += (rng.get() & mask) - (rng.get() & mask);
	}
}

// Evaluation

constexpr int half_F01 = SIZE_F0 * SIZE_F1 / 2;

inline static int is_black(Piece p) { return int(p == BLACK_P); }
inline static int is_white(Piece p) { return int(p == WHITE_P); }

void compute_L0(int16_t* dst, Piece* squares, Net* n) {
	for (int i = 0; i < SIZE_F1; i++) {
		// Black
		dst[i] = n->L0_b[i];
		// White
		dst[i + SIZE_F1] = n->L0_b[i];
	}

	for (int i = 0; i < half_F01; i++) {
		// Black
		dst[i & (SIZE_F1 - 1)] += is_black(squares[i >> 5]) * (n->L0_a)[(i << 1)];
		dst[i & (SIZE_F1 - 1)] += is_white(squares[i >> 5]) * (n->L0_a)[(i << 1) + 1];
		// White
		dst[(i & (SIZE_F1 - 1)) + SIZE_F1] += is_white(squares[i >> 5]) * (n->L0_a)[(i << 1)];
		dst[(i & (SIZE_F1 - 1)) + SIZE_F1] += is_black(squares[i >> 5]) * (n->L0_a)[(i << 1) + 1];
	}
}

void update_L0(int16_t* dst, Square s, Piece from, Piece to, Net* n) {
#ifdef _AVX256_
	
	__m256i src1_ = _mm256_set1_epi8(uint8_t(is_black(from)));
	__m256i src2_ = _mm256_set1_epi8(uint8_t(is_white(from)));
	__m256i l0w1_ = _mm256_load_si256((__m256i*)(n->L0_a + (int(s) << 6)));
	__m256i l0w2_ = _mm256_load_si256((__m256i*)(n->L0_a + (int(s) << 6) + 32));
	__m256i src_, dst1_, dst2_, dst3_, dst4_, mul_;

	dst1_ = _mm256_load_si256((__m256i*)(dst));
	dst2_ = _mm256_load_si256((__m256i*)(dst + 16));
	dst3_ = _mm256_load_si256((__m256i*)(dst      + SIZE_F1));
	dst4_ = _mm256_load_si256((__m256i*)(dst + 16 + SIZE_F1));

	src_ = _mm256_unpackhi_epi8(src1_, src2_);

	mul_ = _mm256_maddubs_epi16(src_, l0w1_);
	dst1_ = _mm256_subs_epi16(dst1_, mul_);
	mul_ = _mm256_maddubs_epi16(src_, l0w2_);
	dst2_ = _mm256_subs_epi16(dst2_, mul_);

	src_ = _mm256_unpackhi_epi8(src2_, src1_);

	mul_ = _mm256_maddubs_epi16(src_, l0w1_);
	dst3_ = _mm256_subs_epi16(dst3_, mul_);
	mul_ = _mm256_maddubs_epi16(src_, l0w2_);
	dst4_ = _mm256_subs_epi16(dst4_, mul_);

	src1_ = _mm256_set1_epi8(uint8_t(is_black(to)));
	src2_ = _mm256_set1_epi8(uint8_t(is_white(to)));

	src_ = _mm256_unpackhi_epi8(src1_, src2_);

	mul_ = _mm256_maddubs_epi16(src_, l0w1_);
	dst1_ = _mm256_adds_epi16(dst1_, mul_);
	mul_ = _mm256_maddubs_epi16(src_, l0w2_);
	dst2_ = _mm256_adds_epi16(dst2_, mul_);

	src_ = _mm256_unpackhi_epi8(src2_, src1_);

	mul_ = _mm256_maddubs_epi16(src_, l0w1_);
	dst3_ = _mm256_adds_epi16(dst3_, mul_);
	mul_ = _mm256_maddubs_epi16(src_, l0w2_);
	dst4_ = _mm256_adds_epi16(dst4_, mul_);

	_mm256_store_si256((__m256i*)(dst), dst1_);
	_mm256_store_si256((__m256i*)(dst + 16), dst2_);
	_mm256_store_si256((__m256i*)(dst + SIZE_F1), dst3_);
	_mm256_store_si256((__m256i*)(dst + 16 + SIZE_F1), dst4_);

#else

	for (int i = 0; i < SIZE_F1; i++) {
		// Black
		dst[i] -= is_black(from) * (n->L0_a)[(int(s) << 6) + 2 * i];
		dst[i] += is_black(to  ) * (n->L0_a)[(int(s) << 6) + 2 * i];
		dst[i] -= is_white(from) * (n->L0_a)[(int(s) << 6) + 2 * i + 1];
		dst[i] += is_white(to  ) * (n->L0_a)[(int(s) << 6) + 2 * i + 1];
		// White
		dst[i + SIZE_F1] -= is_white(from) * (n->L0_a)[(int(s) << 6) + 2 * i];
		dst[i + SIZE_F1] += is_white(to  ) * (n->L0_a)[(int(s) << 6) + 2 * i];
		dst[i + SIZE_F1] -= is_black(from) * (n->L0_a)[(int(s) << 6) + 2 * i + 1];
		dst[i + SIZE_F1] += is_black(to  ) * (n->L0_a)[(int(s) << 6) + 2 * i + 1];
	}

#endif
}

void ReLUClip_L0(int* dst, int16_t* src, Color side_to_move) {
	if (side_to_move) { src += SIZE_F1; }
	for (int i = 0; i < SIZE_F1; i++) {
		int tmp = src[i] >> 8;
		dst[i] = tmp < 0 ? 0 :
			tmp > 127 ? 127 : tmp;
	}
}

void ReLUClip_L1(int* src) {
	for (int i = 0; i < SIZE_F2; i++) {
		src[i] >>= 8;
		src[i] = src[i] < 0 ? 0 :
			src[i] > 127 ? 127 : src[i];
	}
}

void ReLUClip_L2(int* src) {
	for (int i = 0; i < SIZE_F3; i++) {
		src[i] = src[i] < 0 ? 0 :
			src[i] > 16383 ? 16383 : src[i];
	}
}

constexpr int sde = SIZE_F2 / 4;

void compute_layer(int* dst, int* src, int8_t* a, int16_t* b) {
#ifdef _AVX256_

	__m256i dst1_;
	__m256i dst2_;
	int16_t buf[SIZE_F2];

	dst1_ = _mm256_setzero_si256();
	dst2_ = _mm256_setzero_si256();

	for (int j = 0; j < SIZE_F1; j += 2) {
		__m256i src1_ = _mm256_set1_epi8(uint8_t(*(src + j)));
		__m256i src2_ = _mm256_set1_epi8(uint8_t(*(src + j + 1)));
		__m256i src_ = _mm256_unpackhi_epi8(src1_, src2_);
		__m256i l1w1_ = _mm256_load_si256((__m256i*)(a + j * SIZE_F2));
		__m256i l1w2_ = _mm256_load_si256((__m256i*)(a + (j + 1) * SIZE_F2));
		__m256i s1 = _mm256_maddubs_epi16(src_, _mm256_unpackhi_epi8(l1w1_, l1w2_));
		__m256i s2 = _mm256_maddubs_epi16(src_, _mm256_unpacklo_epi8(l1w1_, l1w2_));
		dst1_ = _mm256_adds_epi16(dst1_, s1);
		dst2_ = _mm256_adds_epi16(dst2_, s2);
	}

	_mm256_store_si256((__m256i*)(buf + 0), dst2_);
	_mm256_store_si256((__m256i*)(buf + 16), dst1_);

	for (int i = 0; i < sde; i++) {
		dst[i          ] = int(buf[i          ]) + b[i          ];
		dst[i +     sde] = int(buf[i + 2 * sde]) + b[i +     sde];
		dst[i + 2 * sde] = int(buf[i +     sde]) + b[i + 2 * sde];
		dst[i + 3 * sde] = int(buf[i + 3 * sde]) + b[i + 3 * sde];
	}

# else

	for (int j = 0; j < SIZE_F2; j++) {
		dst[j] = int(n->L1_b[j]);
	}

	for (int i = 0; i < SIZE_F1; i++) {
		for (int j = 0; j < SIZE_F2; j++) {
			dst[j] += src[i] * n->L1_a[i * SIZE_F2 + j];
		}
	}

#endif
}

void compute_L3(int* dst, int* src, Net* n) {
	*dst = 0;
	for (int i = 0; i < SIZE_F3; i++) {
		*dst += src[i] * n->L3_a[i];
	}
}


int compute(int16_t* src, Net* n, Color side_to_move) {
	int r;
	alignas(32)
	int P1[SIZE_F1];
	int P2[SIZE_F2];
	int P3[SIZE_F3];
	
	ReLUClip_L0(P1, src, side_to_move);
	compute_layer(P2, P1, n->L1_a, n->L1_b);
	ReLUClip_L1(P2);
	compute_layer(P3, P2, n->L2_a, n->L2_b);
	ReLUClip_L2(P3);
	compute_L3(&r, P3, n);

	return r;
}


void compute_layer_fallback(int* dst, int* src, int8_t* a, int16_t* b) {
	for (int j = 0; j < SIZE_F2; j++) {
		dst[j] = int(b[j]);
	}

	for (int i = 0; i < SIZE_F1; i++) {
		for (int j = 0; j < SIZE_F2; j++) {
			dst[j] += src[i] * a[i * SIZE_F2 + j];
		}
	}
}

void verify_SIMD(Net* n) {
	alignas(32)
	int16_t P1_ACC[SIZE_F1 * 2];
	int16_t P1_ACC_F[SIZE_F1 * 2];
	int P1[SIZE_F1];
	int P2[SIZE_F2];
	int P2_F[SIZE_F2];

	Piece sq[64] = { };
	

	for (int i = 0; i < 32; i++) {
		P1_ACC[i] = n->L0_b[i];
		P1_ACC[i + 32] = n->L0_b[i];
	}

	for (int i = 0; i < 64; i++) {
		Piece p = Piece(rng.get() & 3);
		if (p == MISC) { p = EMPTY; }
		sq[i] = p;
		update_L0(P1_ACC, Square(i), EMPTY, p, n);
	}

	compute_L0(P1_ACC_F, sq, n);

	int p1_err = 0;
	for (int i = 0; i < SIZE_F1 * 2; i++) {
		if (P1_ACC[i] != P1_ACC_F[i]) { 
			p1_err++;
			std::cout << '\n' << P1_ACC[i] << ' ' << P1_ACC_F[i];
		}
	}
	std::cout << "\nupdate_L0 error: " << p1_err << std::endl;

	ReLUClip_L0(P1, P1_ACC, BLACK);
	compute_layer(P2, P1, n->L1_a, n->L1_b);
	ReLUClip_L1(P2);
	compute_layer_fallback(P2_F, P1, n->L1_a, n->L1_b);
	ReLUClip_L1(P2_F);

	int p2_err = 0;
	for (int i = 0; i < SIZE_F2; i++) {
		if (P2[i] != P2_F[i]) { p2_err++; }
	}
	std::cout << "\ncompute_Layer error: " << p2_err << std::endl;
}