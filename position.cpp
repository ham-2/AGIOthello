#include <sstream>
#include <bitset>
#include <memory.h>

#include "position.h"

// For Zobrist Hashing
Key piece_keys[3][64] = { };
Key side_to_move_key;

void Position::init() {
	//set Zobrist keys at startup
	PRNG generator = PRNG(9235123129483259312ULL);

	for (Piece p : { BLACK_P, WHITE_P }) {
		for (int j = 0; j < 64; j++) {
			piece_keys[p][j] = Key(generator.get());
		}
	}

	side_to_move_key = Key(generator.get());
}

void Position::pop_stack() {
	Undo* t = undo_stack;
	undo_stack = undo_stack->prev;
	if (t->del) { delete t; }
}

void Position::clear_stack() {
	while (undo_stack->prev != nullptr) { pop_stack(); }
}

void Position::set_rays(Piece p, Square s) {
	int file = get_file(s);
	int rank = get_rank(s);
	int ldiag = get_ldiag(s);
	int rdiag = get_rdiag(s);
	int num[3] = { 0, 256, 257 };

	files[file] &= Masks[rank];
	files[file] |= num[p] << rank;
	ranks[rank] &= Masks[file];
	ranks[rank] |= num[p] << file;

	ldiags[ldiag] &= Masks[get_ldiag_idx(s)];
	ldiags[ldiag] |= num[p] << get_ldiag_idx(s);
	rdiags[rdiag] &= Masks[get_rdiag_idx(s)];
	rdiags[rdiag] |= num[p] << get_rdiag_idx(s);
}

Bitboard Position::index_captures(Square s) {
	// Assumes the square is empty
	int file = get_file(s);
	int rank = get_rank(s);
	int ldiag = get_ldiag(s);
	int rdiag = get_rdiag(s);
	
	uint16_t f_idx = files[file] | 1 << rank;
	uint16_t r_idx = ranks[rank] | 1 << file;
	uint16_t ld_idx = ldiags[ldiag] | 1 << get_ldiag_idx(s);
	uint16_t rd_idx = rdiags[rdiag] | 1 << get_rdiag_idx(s);

	// need to flip idx's when white
	if (side_to_move) {
		f_idx ^= (f_idx >> 8);
		r_idx ^= (r_idx >> 8);
		ld_idx ^= (ld_idx >> 8);
		rd_idx ^= (rd_idx >> 8);
	}

	uint8_t file_c = Captures[f_idx];
	uint8_t rank_c = Captures[r_idx];
	uint8_t ldiag_c = Captures[ld_idx];
	uint8_t rdiag_c = Captures[rd_idx];

	return to_file(file_c, file) | to_rank(rank_c, rank) |
		to_ldiag(ldiag_c, ldiag) | to_rdiag(rdiag_c, rdiag);
}

// place/remove/move functions - dont handle key xor
inline void Position::place(Piece p, Square s) {
	squares[s] = p;
	pieces[p] ^= SquareBoard[s];
	pieces[EMPTY] ^= SquareBoard[s];

	set_rays(p, s);

	piece_count[EMPTY]--;
	piece_count[p]++;
}

inline void Position::remove(Piece p, Square s) {
	squares[s] = EMPTY;
	pieces[p] ^= SquareBoard[s];
	pieces[EMPTY] ^= SquareBoard[s];

	set_rays(EMPTY, s);

	piece_count[EMPTY]++;
	piece_count[p]--;
}

void Position::rebuild() { // computes others from squares data.
	for (int i = 0; i < 3; i++) { pieces[i] = 0; piece_count[i] = 0; }
	for (int i = 0; i < 8; i++) { files[i] = 0; ranks[i] = 0; }
	for (int i = 0; i < 15; i++) { ldiags[i] = 0; rdiags[i] = 0; }
	
	// set EMPTYs to full
	piece_count[EMPTY] = 64;
	pieces[EMPTY] = FullBoard;

	for (Square s = A1; s < SQ_END; ++s) {
		Piece p = squares[s];
		place(p, s);

		undo_stack->key ^= piece_keys[p][s];
	}
}

Position::Position(Net* n) {
	memset(this, 0, sizeof(Position));
	undo_stack = new Undo;
	memset(undo_stack, 0, sizeof(Undo));
	undo_stack->prev = nullptr;
	net = n;
}

void Position::verify() {
	// Verify all data are consistant.
	Position testpos(net);
	for (int i = 0; i < 64; i++) { testpos.squares[i] = squares[i]; }
	testpos.rebuild();

	Bitboard test = EmptyBoard;
	for (int i = 0; i < 3; i++) {
		if (test & pieces[i]) {
			cout << "Pieces overlap detected at " << i << endl;
		}
		test |= pieces[i];

		if (testpos.pieces[i] != pieces[i]) {
			cout << "Pieces inconsistent at " << i << endl;
		}

		if (i != 0 && testpos.piece_count[i] != piece_count[i]) {
			cout << "Piece count inconsistent at " << i << endl;
		}
	}

	for (int i = 0; i < 8; i++) { 
		if (testpos.files[i] != files[i]) {
			cout << "Files inconsistent at " << i << endl;
		}
		if (testpos.ranks[i] != ranks[i]) {
			cout << "Ranks inconsistent at " << i << endl;
		}
	}

	for (int i = 0; i < 15; i++) { 
		if (testpos.ldiags[i] != ldiags[i]) {
			cout << "Ldiags inconsistent at " << i << endl;
		}
		if (testpos.rdiags[i] != rdiags[i]) {
			cout << "Rdiags inconsistent at " << i << endl;
		}
	}

	cout << *this << "\n\n" << testpos << endl;
}

void Position::show() {
	cout << *this;
}

ostream& operator<<(ostream& os, Position& pos) {
	// Show board
	Square sq = A8;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			os << print_piece(pos.squares[sq]) << " ";
			++sq;
		}
		sq += (-16);
		os << "\n";
	}

	if (pos.side_to_move) { os << "White to move"; }
	else { os << "Black to move"; }
	os << "\n";

	// Key
	os << "Key: " << pos.get_key() << "\n";

	return os;
}

void Position::set(string fen) {
	char c;
	istringstream ss(fen);

	// first clear stack
	clear_stack();
	Undo* temp = undo_stack;
	Net* temp2 = net;
	memset(this, 0, sizeof(Position));
	undo_stack = temp;
	net = temp2;
	memset(undo_stack, 0, sizeof(Undo));

	// set pieces from rank 8.
	ss >> noskipws >> c;
	Square sq = A8;
	Piece p;
	while (c != ' ') {
		if (isdigit(c)) {
			sq += (c - '0');
		}
		else if (c == '/') {
			sq += (-16);
		}
		else if (parse_piece(c, p)) {
			squares[sq] = p;
			++sq;
		}
		ss >> c;
	}
		
	// white / black to move
	ss >> c;
	side_to_move = c == 'w' ? WHITE : BLACK;
	if (side_to_move == WHITE) { undo_stack->key ^= side_to_move_key; }
	ss >> c;

	// write other data
	undo_stack->captured = EmptyBoard;
	rebuild();
}

void Position::do_move(Square s, Undo* new_undo) {
	memcpy(new_undo, undo_stack, sizeof(Undo));
	new_undo->prev = undo_stack;
	new_undo->del = false;
	
	undo_stack = new_undo;

	// Handle captures
	Bitboard captures = index_captures(s);
	Piece p = side_to_move ? WHITE_P : BLACK_P;
	
	pieces[~p] ^= captures;
	pieces[p] ^= captures;

	new_undo->captured = captures;
	while (captures) {
		Square c = pop_lsb(&captures);
		
		squares[c] = p;
		set_rays(p, c);
		update_L0(new_undo->accumulator, s, ~p, p, net);

		piece_count[p]++;
		piece_count[~p]--;

		new_undo->key ^= piece_keys[~p][c] ^ piece_keys[p][c];
	}

	// Place the new piece
	place(p, s);
	update_L0(new_undo->accumulator, s, EMPTY, p, net);
	new_undo->key ^= piece_keys[p][s];
	
	side_to_move = ~side_to_move;
	new_undo->key ^= side_to_move_key;
}

void Position::undo_move(Square s) {
	Piece p = side_to_move ? WHITE_P : BLACK_P; // Piece to place again
	Bitboard captured = undo_stack->captured;

	pieces[~p] ^= captured;
	pieces[p] ^= captured;
	remove(~p, s);

	while (captured) {
		Square c = pop_lsb(&captured);
		squares[c] = p;
		set_rays(p, c);

		piece_count[p]++;
		piece_count[~p]--;
	}

	side_to_move = ~side_to_move;
	pop_stack();
}

void Position::do_null_move(Undo* new_undo) {
	memcpy(new_undo, undo_stack, sizeof(Undo));
	new_undo->prev = undo_stack;
	new_undo->pass = true;
	new_undo->del = false;

	undo_stack = new_undo;

	side_to_move = ~side_to_move;
	new_undo->key ^= side_to_move_key;

	new_undo->captured = EMPTY;
}

void Position::undo_null_move() {
	side_to_move = ~side_to_move;
	pop_stack();
}

void Position::do_move_wrap(Square s, Undo* new_undo) {
	if (s == NULL_MOVE) { do_null_move(new_undo); }
	else { do_move(s, new_undo); }
}

void Position::undo_move_wrap(Square s) {
	if (s == NULL_MOVE) { undo_null_move(); }
	else { undo_move(s); }
}

Position& Position::operator=(const Position board) {
	clear_stack();
	delete undo_stack;
	memcpy(this, &board, sizeof(Position));
	undo_stack = new Undo;
	memcpy(undo_stack, board.undo_stack, sizeof(Undo));
	undo_stack->prev = nullptr;
	return *this;
}