#include "book.h"

constexpr uint64_t mask_lower56 = ~0ULL >> 8;
constexpr int BOOK_WINDOW = 6 << (EVAL_BITS - 6);

inline uint64_t make_entry(Key key, Square nmove) {
	return (key & mask_lower56) | (uint64_t(nmove) << 56);
}

inline Square read_entry(uint64_t entry) {
	return Square(entry >> 56);
}

Square probe_entry(Key key)
{
	return GAME_END;
}

int _build_book(SearchParams* sp, int ply) 
{
	Position* board = sp->board;
	atomic<bool>* stop = sp->stop;
	TT* table = sp->table;

	sp->table->increment();
	MoveList legal_moves;
	legal_moves.generate(*board);

	Key root_key = board->get_key();

	Square s;
	Square nmove = SQ_END;
	int comp_eval;
	int new_eval = EVAL_INIT;
	int reduction;

	int i = 0;
	while ((i < legal_moves.length()) && !(*stop)) {
		s = *(legal_moves.list + i);

		// Do move
		Bitboard c;
		board->do_move(s, &c);
		TTEntry probe_m;

		int r = 1;
		while (r <= 16) {
			comp_eval = -alpha_beta(sp, &probe_m,
				r, -BOOK_WINDOW, BOOK_WINDOW);
			r++;
		}

		if (comp_eval > new_eval) {
			new_eval = comp_eval;
			nmove = s;
		}

		board->undo_move(s, &c);
		i++;
	}
}

void build_book(int depth)
{

}
