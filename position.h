#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>

#include "board.h"
#include "misc.h"
#include "network.h"

using namespace std;

typedef uint64_t Key;

struct Undo {
	// Informations needed to undo a move. Stored in a stack
	Undo* prev;
	Square s;
	Bitboard captured;

	// For eval
	alignas(32)
	int16_t accumulator[2 * SIZE_F1];
};

class Position {
private:
	Undo* undo_stack;
	Undo root;

	Piece squares[64];

	Bitboard pieces[3];
	int empty_count;
	Color side_to_move;

	Net* net;

	void pop_stack();
	void clear_stack();

	Bitboard index_captures(Square s, Piece p);

	void place(Piece p, Square s);
	void remove(Piece p, Square s);

	void rebuild();

public:
	Position(Net* n);
	~Position();

	void verify();

	inline Key get_key() { return 
		side_to_move ? hash_128i(~pieces[EMPTY], pieces[BLACK_P]) 
		: hash_128i(~pieces[EMPTY], pieces[WHITE_P]); }
	Color get_side() { return side_to_move; }
	Piece get_piece(Square s) { return squares[s]; }
	inline Bitboard get_pieces(Piece p) { return pieces[p]; }
	inline int get_count_empty() { return empty_count; }
	inline int get_count(Piece p) { return popcount(pieces[p]); }
	Bitboard get_occupied() { return ~pieces[EMPTY]; }
	inline Undo* get_stack() { return undo_stack; }
	inline int16_t* get_accumulator() { return undo_stack->accumulator; }
	inline void set_accumulator() { compute_L0(undo_stack->accumulator, squares, net); }
	inline Net* get_net() { return net; }
	inline void get_eval(int* dst) { compute(dst, undo_stack->accumulator, net, side_to_move); }

	// functions for tuning
	inline void set_net(Net* n) { net = n; }
	void set_squares();

	void show();
	void set(string fen);
	void do_move(Square s, Undo* new_undo);
	void undo_move();
	void do_null_move(Undo* new_undo);
	void undo_null_move();
	void do_move_wrap(Square s, Undo* new_undo);
	void do_move_fast(Square s, Bitboard* captures);
	void do_null_fast();
	void undo_move_fast(Square s, Bitboard* captures);
	void undo_null_fast();

	Position& operator=(const Position& board);

	friend ostream& operator<<(ostream& os, Position& pos);
};

ostream& operator<<(ostream& os, Position& pos);

#endif
