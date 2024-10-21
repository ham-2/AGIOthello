#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>

#include "board.h"
#include "misc.h"
#include "network.h"

using namespace std;

typedef uint64_t Key;

class Position {
private:
	Bitboard pieces[3];
	int empty_count;
	Color side_to_move;

	// For eval
	alignas(32)
	int16_t accumulator[2 * SIZE_F1];
	Net* net;
	Bitboard acc_state[3];

	Bitboard index_captures(Square s, Piece p);

	template <Piece from, Piece to>
	void update_();
	void set_state();

public:
	Position(Net* n);

	inline Key get_key() { return 
		side_to_move ? hash_128i(~pieces[EMPTY], pieces[BLACK_P]) 
		: hash_128i(~pieces[EMPTY], pieces[WHITE_P]); }
	Color get_side() { return side_to_move; }
	inline Piece get_piece(Square s) { 
		return pieces[EMPTY] & SquareBoard[s] ? EMPTY :
			pieces[BLACK_P] & SquareBoard[s] ? BLACK_P : WHITE_P;
	}
	inline Bitboard get_pieces(Piece p) { return pieces[p]; }
	inline int get_count_empty() { return empty_count; }
	inline int get_count(Piece p) { return popcount(pieces[p]); }
	Bitboard get_occupied() { return ~pieces[EMPTY]; }
	inline int16_t* get_accumulator() { return accumulator; }
	inline void set_accumulator() { 
		compute_L0(accumulator, pieces, net);
		for (int i = 0; i < 3; i++) { acc_state[i] = pieces[i]; }
	}
	inline Net* get_net() { return net; }
	inline void get_eval(int* dst) { 
		set_state();
		compute(dst, accumulator, net, side_to_move);
	}

	// functions for tuning
	inline void set_net(Net* n) { net = n; }

	void show();
	void set(string fen);
	void do_move_wrap(Square s, Bitboard* captures);
	void undo_move_wrap(Square s, Bitboard* captures);
	void do_move(Square s, Bitboard* captures);
	void undo_move(Square s, Bitboard* captures);
	inline void pass() { side_to_move = ~side_to_move; }

	Position& operator=(const Position& board);

	friend ostream& operator<<(ostream& os, Position& pos);
};

ostream& operator<<(ostream& os, Position& pos);

#endif
