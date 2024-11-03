#ifndef NETWORK_INCLUDED
#define NETWORK_INCLUDED

#include "board.h"
#include "misc.h"

constexpr int SIZE_F0 = 128;
constexpr int SIZE_F1 = 128;
constexpr int SIZE_F2 = 64;
constexpr int SIZE_F3 = 64;
constexpr int SIZE_OUT = 2;

constexpr int SHIFT_L1 = 4;
constexpr int SHIFT_L2 = 6;
constexpr int SHIFT_L3 = 6;

constexpr int MAX_L1 = 255;
constexpr int MAX_L2 = 255;
constexpr int MAX_L3 = (1 << 15) - 1;

constexpr int L0_OFFSET = SIZE_F1 * SQ_END;

constexpr int EVAL_BITS = 16;

struct Net {

	alignas(32) 
	int8_t L0_a[SIZE_F0 * SIZE_F1];
	int16_t L0_b[SIZE_F1];

	int8_t L1_a[SIZE_F1 * SIZE_F2];
	int16_t L1_b[SIZE_F2];

	int8_t L2_a[SIZE_F2 * SIZE_F3];
	int16_t L2_b[SIZE_F3];

	int16_t L3_a[SIZE_F3 * SIZE_OUT];
	int32_t L3_b[SIZE_OUT];

};

inline void zero_weights(Net* net) { memset(net, 0, sizeof(Net)); }
void rand_weights_all(Net* net, int bits);
void rand_weights_1(Net* net, int mm);

inline int load_weights(Net* net, std::string filename) {
	return load_file((char*)net, filename, sizeof(Net));
}
inline void save_weights(Net* net, std::string filename) {
	return save_file((char*)net, filename, sizeof(Net));
}
void get_stats(Net* net);

void compute_L0(int16_t* dst_b, Bitboard* pieces, Net* n);
void update_L0(int16_t* dst, Square s, Piece from, Piece to, Net* n);
template <int S, int shift, int max>
void ReLUClip(int16_t* dst, int16_t* src);
template <int size_dst, int size_src>
void compute_layer(int16_t* dst, int16_t* src,
	               int8_t* a, int16_t* b);
void compute_L3(int* dst, int16_t* src, Net* n);

void compute(int* dst, int16_t* src, Net* n, Color side_to_move);

#endif