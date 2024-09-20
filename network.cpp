#include "network.h"

#include <bitset>

#include <cassert>

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
	input.read((char*)(&net->L3_b), 4);

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
	output.write((char*)(&net->L3_b), 4);

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

void set_weights(Net* net) {
	zero_weights(net);
	for (int s = A1; s < SQ_END; s++) {
		for (int i = 0; i < SIZE_F1; i += 2) {
			net->L0_a[65536 + s * SIZE_F1 + i] = 64;
			net->L0_a[67584 + s * SIZE_F1 + i + 1] = 64;
		}
	}
	for (int i = 0; i < 32; i += 2) {
		for (int j = 0; j < 32; j += 2) {
			net->L1_a[j + 0 + (i + 0) * 32] = 16;
			net->L1_a[j + 1 + (i + 1) * 32] = 16;
		}
	}
	for (int i = 0; i < 32; i += 2) {
		for (int j = 0; j < 32; j += 2) {
			net->L2_a[j + 0 + (i + 0) * 32] = 2;
			net->L2_a[j + 1 + (i + 1) * 32] = 2;
		}
	}
	for (int i = 0; i < 32; i += 2) {
		net->L3_a[i + 0] = 16384;
		net->L3_a[i + 1] = -16384;
	}
}

// Evaluation

// L0 idx
// DIR 1 (0)	[8192 * pair + 2048 * dir + sq * SIZE_F1]
// DIR 7 (1)	
// DIR 8 (2)	
// DIR 9 (3)	
//	EMPTY - US		(0)
//	EMPTY - THEM	(1)
//	US    - EMPTY	(2)
//	US    - US		(3)
//	US	  - THEM	(4)
//	THEM  - EMPTY	(5)
//	THEM  - US		(6)
//	THEM  - THEM	(7)

// US			[65536 + sq * SIZE_F1]
// THEM			[67584 + sq * SIZE_F1]


void compute_L0(int16_t* dst, Piece* squares, Bitboard* pieces, Net* n) {
	for (int i = 0; i < SIZE_F1; i++) {
		// Bias
		dst[i          ] = n->L0_b[i];
		dst[i + SIZE_F1] = n->L0_b[i];
	}

	for (int s = A1; s < SQ_END; s++) {
		if (squares[s] == BLACK_P) {
			for (int i = 0; i < SIZE_F1; i++) {
				dst[i          ] += n->L0_a[s * SIZE_F1 + i + 65536];
				dst[i + SIZE_F1] += n->L0_a[s * SIZE_F1 + i + 67584];
			}
		}

		else if (squares[s] == WHITE_P) {
			for (int i = 0; i < SIZE_F1; i++) {
				dst[i          ] += n->L0_a[s * SIZE_F1 + i + 67584];
				dst[i + SIZE_F1] += n->L0_a[s * SIZE_F1 + i + 65536];
			}
		}
	}

	for (int pair = 0; pair < 8; pair++) {

	Bitboard bn[4];
	int p1 = (pair + 1) / 3;
	int p2 = (pair + 1) % 3;
	int pair2 = ((p1 ^ 3) % 3) * 3 +
		((p2 ^ 3) % 3) - 1;

	bn[0] = shift<-1>(pieces[p2]) & pieces[p1];
	bn[1] = shift<-7>(pieces[p2]) & pieces[p1];
	bn[2] = shift<-8>(pieces[p2]) & pieces[p1];
	bn[3] = shift<-9>(pieces[p2]) & pieces[p1];

	for (int dir = 0; dir < 4; dir++) {
		while (bn[dir]) {
			Square s = pop_lsb(&bn[dir]);
			for (int i = 0; i < SIZE_F1; i++) {
				dst[i          ] += n->L0_a[8192 * pair  + 2048 * dir + s * SIZE_F1 + i];
				dst[i + SIZE_F1] += n->L0_a[8192 * pair2 + 2048 * dir + s * SIZE_F1 + i];
			}
		}
	}

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
		dst[i] -= n[addr + i];
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
		dst[i] += n[addr + i];
	}
#endif
}

inline int _get_pair(Piece p1, Piece p2, Color c) {
	return c ? ((p1 ^ 3) % 3) * 3 + ((p2 ^ 3) % 3) - 1 :
		p1 * 3 + p2 - 1;
}

void _update_L0(int16_t* dst, int dir, Piece* squares,
	Square s, Piece from, Piece to, Net* n)
{
	if (dir < 0) {

	int offset = (-dir - 1) % 5;

	int pair = _get_pair(squares[s + dir], from, BLACK);
	if (pair > -1) {
		_sub_L0(dst          , 8192 * pair + 2048 * offset + (s + dir) * SIZE_F1, n);
		pair = _get_pair(squares[s + dir], from, WHITE);
		_sub_L0(dst + SIZE_F1, 8192 * pair + 2048 * offset + (s + dir) * SIZE_F1, n);
	}
	pair = _get_pair(squares[s + dir], to, BLACK);
	if (pair > -1) {
		_add_L0(dst, 8192 * pair + 2048 * offset + (s + dir) * SIZE_F1, n);
		pair = _get_pair(squares[s + dir], to, WHITE);
		_add_L0(dst + SIZE_F1, 8192 * pair + 2048 * offset + (s + dir) * SIZE_F1, n);
	}
	}
	else {

	int offset = (dir - 1) % 5;

	int pair = _get_pair(from, squares[s + dir], BLACK);
	if (pair > -1) {
		_sub_L0(dst          , 8192 * pair + 2048 * offset + s * SIZE_F1, n);
		pair = _get_pair(from, squares[s + dir], WHITE);
		_sub_L0(dst + SIZE_F1, 8192 * pair + 2048 * offset + s * SIZE_F1, n);
	}

	pair = _get_pair(to, squares[s + dir], BLACK);
	if (pair > -1) {
		_add_L0(dst, 8192 * pair + 2048 * offset + s * SIZE_F1, n);
		pair = _get_pair(to, squares[s + dir], WHITE);
		_add_L0(dst + SIZE_F1, 8192 * pair + 2048 * offset + s * SIZE_F1, n);
	}
	}
}

void update_L0(int16_t* dst, Square s, Piece* squares, Piece to, Net* n) {
	Piece from = squares[s];

	if (from == BLACK_P) {
		_sub_L0(dst          , s * SIZE_F1 + 65536, n);
		_sub_L0(dst + SIZE_F1, s * SIZE_F1 + 67584, n);
	}
	else if (from == WHITE_P) {
		_sub_L0(dst          , s * SIZE_F1 + 67584, n);
		_sub_L0(dst + SIZE_F1, s * SIZE_F1 + 65536, n);
	}

	if (to == BLACK_P) {
		_add_L0(dst          , s * SIZE_F1 + 65536, n);
		_add_L0(dst + SIZE_F1, s * SIZE_F1 + 67584, n);
	}
	else if (to == WHITE_P) {
		_add_L0(dst          , s * SIZE_F1 + 67584, n);
		_add_L0(dst + SIZE_F1, s * SIZE_F1 + 65536, n);
	}

	if (get_rank(s) != 0) {
		if (get_file(s) != 0) {
			_update_L0(dst, -9, squares, s, from, to, n);
		}

		_update_L0(dst, -8, squares, s, from, to, n);

		if (get_file(s) != 7) {
			_update_L0(dst, -7, squares, s, from, to, n);
		}
	}
	if (get_file(s) != 0) {
		_update_L0(dst, -1, squares, s, from, to, n);
	}

	if (get_file(s) != 7) {
		_update_L0(dst, 1, squares, s, from, to, n);
	}
	if (get_rank(s) != 7) {
		if (get_file(s) != 0) {
			_update_L0(dst, 7, squares, s, from, to, n);
		}

		_update_L0(dst, 8, squares, s, from, to, n);

		if (get_file(s) != 7) {
			_update_L0(dst, 9, squares, s, from, to, n);
		}
	}
}

void ReLUClip_L0(int16_t* dst, int16_t* src, Color side_to_move) {
	if (side_to_move) { src += SIZE_F1; }
	for (int i = 0; i < SIZE_F1; i++) {
		dst[i] = src[i] >> SHIFT_L0;
		dst[i] = dst[i] < 0 ? 0 :
			dst[i] > 127 ? 127 : dst[i];
	}
}

void ReLUClip_L1(int16_t* dst, int16_t* src) {
	for (int i = 0; i < SIZE_F2; i++) {
		dst[i] = src[i] >> SHIFT_L1;
		dst[i] = dst[i] < 0 ? 0 :
			dst[i] > 127 ? 127 : dst[i];
	}
}

void ReLUClip_L2(int16_t* dst, int16_t* src) {
	for (int i = 0; i < SIZE_F3; i++) {
		dst[i] = src[i] < 0 ? 0 :
			src[i] > 16383 ? 16383 : src[i];
	}
}

void compute_layer_fallback(int16_t* dst, int16_t* src, int8_t* a, int16_t* b) {

	int tmp[SIZE_F2] = { };
	for (int i = 0; i < SIZE_F1; i++) {
		for (int j = 0; j < SIZE_F2; j++) {
			tmp[j] += src[i] * a[i * SIZE_F2 + j];
		}
	}

	for (int j = 0; j < SIZE_F2; j++) {
		tmp[j] += b[j];
		dst[j] = tmp[j] > 32767 ? 32767 :
			tmp[j] < -32768 ? 32768 : tmp[j];
	}
}

void compute_layer(int16_t* dst, int16_t* src, int8_t* a, int16_t* b) {
#ifdef _AVX256_

	__m256i dst1_;
	__m256i dst2_;

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

	_mm256_store_si256((__m256i*)(dst + 0), _mm256_adds_epi16(
		_mm256_permute2f128_si256(dst2_, dst1_, 0b00100000),
		_mm256_load_si256((__m256i*)(b))
	));
	_mm256_store_si256((__m256i*)(dst + 16), _mm256_adds_epi16(
		_mm256_permute2f128_si256(dst2_, dst1_, 0b00110001),
		_mm256_load_si256((__m256i*)(b + 16))
	));

# else

	compute_layer_fallback(dst, src, a, b);

#endif
}

void compute_L3(int64_t* dst, int16_t* src, Net* n) {
	*dst = n->L3_b;
	for (int i = 0; i < SIZE_F3; i++) {
		*dst += int64_t(src[i]) * n->L3_a[i];
	}
}


int compute(int16_t* src, Net* n, Color side_to_move) {

	alignas(32)
	int16_t P1[SIZE_F1];
	int16_t P2[SIZE_F2];
	int16_t P3[SIZE_F3];
	int64_t P4;
	
	ReLUClip_L0(P1, src, side_to_move);
	compute_layer(P2, P1, n->L1_a, n->L1_b);
	ReLUClip_L1(P2, P2);
	compute_layer(P3, P2, n->L2_a, n->L2_b);
	ReLUClip_L2(P3, P3);
	compute_L3(&P4, P3, n);

	assert(P4 > -196609);

	return int(P4);
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

		update_L0(P1_ACC, s, sq, p, n);
		sq[s] = p;
	}

	compute_L0(P1_ACC_F, sq, pieces, n);

	int p1_err = 0;
	for (int i = 0; i < SIZE_F1 * 2; i++) {
		if (P1_ACC[i] != P1_ACC_F[i]) { p1_err++; }
	}

	std::cout << "update_L0 error: " << p1_err << std::endl;

	ReLUClip_L0(P1, P1_ACC, BLACK);
	compute_layer(P2, P1, n->L1_a, n->L1_b);
	compute_layer_fallback(P2_F, P1, n->L1_a, n->L1_b);

	int p2_err = 0;
	for (int i = 0; i < SIZE_F2; i++) {
		if (P2[i] != P2_F[i]) { 
			std::cout << P2[i] << ' ' << P2_F[i] << '\n';
			p2_err++;
		}
	}

	std::cout << "compute_Layer error: " << p2_err << std::endl;
}