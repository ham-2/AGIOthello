#ifndef POSITION_INCLUDED
#define POSITION_INCLUDED

#include <string>

#include "board.h"
#include "misc.h"

using namespace std;

typedef uint64_t Key;

struct Undo {
	Key key;

	// Informations needed to undo a move. Stored in a stack
	Undo* prev;
	Bitboard captured;

	// Other useful informations
	bool pass;
	bool del;
};

class Position {
private:
	Undo* undo_stack;

	Piece squares[64];

	Bitboard pieces[3];
	int piece_count[3];
	uint16_t files[8];
	uint16_t ranks[8];
	uint16_t ldiags[15];
	uint16_t rdiags[15];
	Color side_to_move;

	void pop_stack();
	void clear_stack();

	void set_rays(Piece p, Square s);
	Bitboard index_captures(Square s);

	void place(Piece p, Square s);
	void remove(Piece p, Square s);

	void rebuild();

public:
	Position();

	static void init();

	void verify();

	inline Key get_key() { return undo_stack->key; }
	Color get_side() { return side_to_move; }
	Piece get_piece(Square s) { return squares[s]; }
	inline Bitboard get_pieces(Piece p) { return pieces[p]; }
	inline int get_count(Piece p) { return piece_count[p]; }
	Bitboard get_occupied() { return ~pieces[EMPTY]; }
	inline bool get_passed() { return undo_stack->pass; }

	// functions for movegen
	inline uint16_t get_files(int i) { return files[i]; }
	inline uint16_t get_ranks(int i) { return ranks[i]; }
	inline uint16_t get_ldiags(int i) { return ldiags[i]; }
	inline uint16_t get_rdiags(int i) { return rdiags[i]; }


	void show();
	void set(string fen);
	void do_move(Square s, Undo* new_undo);
	void undo_move(Square s);
	void do_null_move(Undo* new_undo);
	void undo_null_move();
	void do_move_wrap(Square s, Undo* new_undo);
	void undo_move_wrap(Square s);

	Position& operator=(const Position board);

	friend ostream& operator<<(ostream& os, Position& pos);
};

ostream& operator<<(ostream& os, Position& pos);

#endif
