#ifndef EVAL_INCLUDED
#define EVAL_INCLUDED

#include <atomic>

#include "position.h"
#include "movegen.h"
#include "table.h"
#include "misc.h"

using namespace std;

extern atomic<long> node_count;

constexpr int EVAL_END = (1 << (EVAL_BITS + 1)); // Not real value, set if certain win

constexpr int EVAL_ALL = (1 << EVAL_BITS); // +64 disks
constexpr int EVAL_NONE = -EVAL_ALL; // -64 disks
constexpr int EVAL_FAIL = EVAL_NONE - 1;
constexpr int EVAL_MAX = EVAL_ALL ^ EVAL_END;
constexpr int EVAL_MIN = EVAL_NONE ^ EVAL_END;
constexpr int EVAL_INIT = EVAL_MIN - 1;

int eval(Position& board);

inline bool is_mate(int eval) { return eval > EVAL_END || eval < -EVAL_END; }
inline int get_material(Position& board) {
	return board.get_side() ?
		(board.get_count(WHITE_P) - board.get_count(BLACK_P)) << (EVAL_BITS - 6) :
		(board.get_count(BLACK_P) - board.get_count(WHITE_P)) << (EVAL_BITS - 6);
}
inline int get_material_eval(Position& board) {
	int m = get_material(board);
	return m == 0 ? m : m ^ EVAL_END;
}

string eval_print(int eval);

#endif