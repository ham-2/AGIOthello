#include "network.h"

// Read / Write / Modify

void load_weights(Net* net, std::string filename)
{
	char filename_buf[256];
	filename = getcwd_wrap(filename_buf, 256) + filename;

	std::cout << "Loading weights from \"" << filename << "\"\n";
	std::ifstream input(filename, std::ios::binary);

	input.read((char*)net->L0_a, SIZE_F0 * SIZE_F1);
	input.read((char*)net->L0_b, SIZE_F1);
	input.read((char*)net->L1_a, SIZE_F1 * SIZE_F2);
	input.read((char*)net->L1_b, SIZE_F2 * 2);
	input.read((char*)net->L2_a, SIZE_F2 * SIZE_F3 * 2);
	input.read((char*)net->L2_b, SIZE_F3 * 4);
	input.read((char*)net->L3_a, SIZE_F3);

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
	output.write((char*)net->L0_b, SIZE_F1);
	output.write((char*)net->L1_a, SIZE_F1 * SIZE_F2);
	output.write((char*)net->L1_b, SIZE_F2 * 2);
	output.write((char*)net->L2_a, SIZE_F2 * SIZE_F3 * 2);
	output.write((char*)net->L2_b, SIZE_F3 * 4);
	output.write((char*)net->L3_a, SIZE_F3);

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

void compute_L0(int* dst, Piece* squares, Net* n) {
	for (int i = 0; i < SIZE_F1; i++) {
		// Black
		dst[i] = n->L0_b[i];
		// White
		dst[i + SIZE_F1] = n->L0_b[i];
	}

	for (int i = 0; i < half_F01; i++) {
		// Black
		dst[i & (SIZE_F1 - 1)] += is_black(squares[i >> 5]) * (n->L0_a)[i << 1];
		dst[i & (SIZE_F1 - 1)] += is_white(squares[i >> 5]) * (n->L0_a)[(i << 1) + 1];
		// White
		dst[i & (SIZE_F1 - 1) + SIZE_F1] += is_white(squares[i >> 5]) * (n->L0_a)[i << 1];
		dst[i & (SIZE_F1 - 1) + SIZE_F1] += is_black(squares[i >> 5]) * (n->L0_a)[(i << 1) + 1];
	}
}

void update_L0(int* dst, Square s, Piece from, Piece to, Net* n) {
	for (int i = 0; i < 32; i++) {
		// Black
		dst[i] -= is_black(from) * (n->L0_a)[(int(s) << 5) + i];
		dst[i] += is_black(to) * (n->L0_a)[(int(s) << 5) + i];
		dst[i] -= is_white(from) * (n->L0_a)[(int(s) << 5) + i + 1];
		dst[i] += is_white(to) * (n->L0_a)[(int(s) << 5) + i + 1];
		// White
		dst[i + SIZE_F1] -= is_white(from) * (n->L0_a)[(int(s) << 5) + i];
		dst[i + SIZE_F1] += is_white(to) * (n->L0_a)[(int(s) << 5) + i];
		dst[i + SIZE_F1] -= is_black(from) * (n->L0_a)[(int(s) << 5) + i + 1];
		dst[i + SIZE_F1] += is_black(to) * (n->L0_a)[(int(s) << 5) + i + 1];
	}
}

void ReLUClip_L0(int* dst, int* src, Color side_to_move) {
	if (side_to_move) { src += SIZE_F1; }
	for (int i = 0; i < SIZE_F1; i++) {
		dst[i] = src[i] < 0 ? 0 :
			src[i] > 127 ? 127 : src[i];
	}
}

void ReLUClip_L1(int* src) {
	for (int i = 0; i < SIZE_F2; i++) {
		src[i] = src[i] < 0 ? 0 :
			src[i] > 16383 ? 16383 : src[i];
	}
}

void ReLUClip_L2(int* src) {
	for (int i = 0; i < SIZE_F3; i++) {
		src[i] = src[i] < 0 ? 0 :
			src[i] > 2097151 ? 2097151 : src[i];
	}
}

constexpr int sde = SIZE_F2 / 4;

void compute_L1(int* dst, int* src, Net* n) {
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
		__m256i l1w1_ = _mm256_load_si256((__m256i*)(n->L1_a + j * SIZE_F2));
		__m256i l1w2_ = _mm256_load_si256((__m256i*)(n->L1_a + (j + 1) * SIZE_F2));
		__m256i s1 = _mm256_maddubs_epi16(src_, _mm256_unpackhi_epi8(l1w1_, l1w2_));
		__m256i s2 = _mm256_maddubs_epi16(src_, _mm256_unpacklo_epi8(l1w1_, l1w2_));
		dst1_ = _mm256_adds_epi16(dst1_, s1);
		dst2_ = _mm256_adds_epi16(dst2_, s2);
	}

	_mm256_store_si256((__m256i*)(buf + 0), dst2_);
	_mm256_store_si256((__m256i*)(buf + 16), dst1_);

	for (int i = 0; i < sde; i++) {
		dst[i          ] = int(buf[i          ]) + n->L1_b[i          ];
		dst[i +     sde] = int(buf[i + 2 * sde]) + n->L1_b[i +     sde];
		dst[i + 2 * sde] = int(buf[i +     sde]) + n->L1_b[i + 2 * sde];
		dst[i + 3 * sde] = int(buf[i + 3 * sde]) + n->L1_b[i + 3 * sde];
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

constexpr int sde2 = SIZE_F3 / 8;

void compute_L2(int* dst, int* src, Net* n) {
#ifdef _AVX256_

	__m256i dst1_;
	__m256i dst2_;
	__m256i dst3_;
	__m256i dst4_;
	int32_t buf[SIZE_F2];

	for (int i = 0; i < SIZE_F3; i += 16) {

	dst1_ = _mm256_setzero_si256();
	dst2_ = _mm256_setzero_si256();
	dst3_ = _mm256_setzero_si256();
	dst4_ = _mm256_setzero_si256();

	for (int j = 0; j < SIZE_F2; j += 2) {
		__m256i src1_ = _mm256_set1_epi16(uint16_t(*(src + j)));
		__m256i src2_ = _mm256_set1_epi16(uint16_t(*(src + j + 1)));
		__m256i src_ = _mm256_unpackhi_epi16(src1_, src2_);
		__m256i l1w1_ = _mm256_load_si256((__m256i*)(n->L2_a + j * SIZE_F2));
		__m256i l1w2_ = _mm256_load_si256((__m256i*)(n->L2_a + (j + 1) * SIZE_F2));
		__m256i l1w3_ = _mm256_load_si256((__m256i*)(n->L2_a + j * SIZE_F2 + SIZE_F2 / 2));
		__m256i l1w4_ = _mm256_load_si256((__m256i*)(n->L2_a + (j + 1) * SIZE_F2 + SIZE_F2 / 2));
		__m256i s1 = _mm256_madd_epi16(src_, _mm256_unpackhi_epi16(l1w1_, l1w2_));
		__m256i s2 = _mm256_madd_epi16(src_, _mm256_unpacklo_epi16(l1w1_, l1w2_));
		__m256i s3 = _mm256_madd_epi16(src_, _mm256_unpackhi_epi16(l1w3_, l1w4_));
		__m256i s4 = _mm256_madd_epi16(src_, _mm256_unpacklo_epi16(l1w3_, l1w4_));
		dst1_ = _mm256_add_epi32(dst1_, s1);
		dst2_ = _mm256_add_epi32(dst2_, s2);
		dst3_ = _mm256_add_epi32(dst3_, s3);
		dst4_ = _mm256_add_epi32(dst4_, s4);
	}

	_mm256_store_si256((__m256i*)(buf + 0), dst2_);
	_mm256_store_si256((__m256i*)(buf + 8), dst1_);
	_mm256_store_si256((__m256i*)(buf + 16), dst4_);
	_mm256_store_si256((__m256i*)(buf + 24), dst3_);

	}

	for (int i = 0; i < sde2; i++) {
		dst[i           ] = buf[i           ] + n->L2_b[i           ];
		dst[i +     sde2] = buf[i + 2 * sde2] + n->L2_b[i +     sde2];
		dst[i + 2 * sde2] = buf[i +     sde2] + n->L2_b[i + 2 * sde2];
		dst[i + 3 * sde2] = buf[i + 3 * sde2] + n->L2_b[i + 3 * sde2];
	}
	for (int i = 4 * sde2; i < 5 * sde2; i++) {
		dst[i           ] = buf[i           ] + n->L2_b[i           ];
		dst[i +     sde2] = buf[i + 2 * sde2] + n->L2_b[i +     sde2];
		dst[i + 2 * sde2] = buf[i +     sde2] + n->L2_b[i + 2 * sde2];
		dst[i + 3 * sde2] = buf[i + 3 * sde2] + n->L2_b[i + 3 * sde2];
	}

#else

	for (int i = 0; i < SIZE_F3; i++) {
		dst[i] = int(n->L2_b[i]);
	}

	for (int i = 0; i < SIZE_F2; i++) {
		for (int j = 0; j < SIZE_F3; j++) {
			dst[j] += src[i] * n->L2_a[i * SIZE_F2 + j];
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


int compute(int* src, Net* n, Color side_to_move) {
	int r;
	alignas(32)
	int P1[SIZE_F1];
	int P2[SIZE_F2];
	int P3[SIZE_F3];
	
	ReLUClip_L0(P1, src, side_to_move);
	compute_L1(P2, P1, n);
	ReLUClip_L1(P2);
	compute_L2(P3, P2, n);
	ReLUClip_L2(P3);
	compute_L3(&r, P3, n);

	return r;
}


void compute_L1_fallback(int* dst, int* src, Net* n) {
	for (int j = 0; j < SIZE_F2; j++) {
		dst[j] = int(n->L1_b[j]);
	}

	for (int i = 0; i < SIZE_F1; i++) {
		for (int j = 0; j < SIZE_F2; j++) {
			dst[j] += src[i] * n->L1_a[i * SIZE_F2 + j];
		}
	}
}

void compute_L2_fallback(int* dst, int* src, Net* n) {
	for (int i = 0; i < SIZE_F3; i++) {
		dst[i] = int(n->L2_b[i]);
	}

	for (int i = 0; i < SIZE_F2; i++) {
		for (int j = 0; j < SIZE_F3; j++) {
			dst[j] += src[i] * n->L2_a[i * SIZE_F2 + j];
		}
	}
}

void verify_SIMD(Net* n) {
	alignas(32)
	int P1[SIZE_F1];
	int P2[SIZE_F2];
	int P2_F[SIZE_F2];
	int P3[SIZE_F3];
	int P3_F[SIZE_F3];

	for (int i = 0; i < SIZE_F1; i++) {
		P1[i] = short(rng.get() >> 57);
		std::cout << P1[i] << " ";
	}

	ReLUClip_L0(P1, P1, BLACK);
	compute_L1(P2, P1, n);
	ReLUClip_L1(P2);
	compute_L1_fallback(P2_F, P1, n);
	ReLUClip_L1(P2_F);

	int p2_err = 0;
	for (int i = 0; i < SIZE_F2; i++) {
		if (P2[i] != P2_F[i]) { p2_err++; }
	}
	std::cout << "\n P2 error: " << p2_err << std::endl;

	
	compute_L2(P3, P2, n);
	//ReLUClip_L2(P3);
	compute_L2_fallback(P3_F, P2_F, n);
	//ReLUClip_L2(P3_F);

	int p3_err = 0;
	for (int i = 0; i < SIZE_F3; i++) {
		if (P3[i] != P3_F[i]) { p3_err++; }
	}
	std::cout << "\n P3 error: " << p3_err << std::endl;
}