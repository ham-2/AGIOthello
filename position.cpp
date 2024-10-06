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

template <int T>
inline constexpr Bitboard add_captures(Square s, Bitboard us, Bitboard them)
{
	Bitboard cand = SquareBoard[s];
	if (T > 0) {
		cand |= them & (cand << T);
		them &=        (them << T);
		cand |= them & (cand << (2 * T));
		them &=        (them << (2 * T));
		cand |= them & (cand << (4 * T));
	}
	else {
		cand |= them & (cand >> -T);
		them &=        (them >> -T);
		cand |= them & (cand >> (2 * -T));
		them &=        (them >> (2 * -T));
		cand |= them & (cand >> (4 * -T));
	}
	cand ^= SquareBoard[s];
	return (shift<T>(cand) & us) ? cand : EmptyBoard;
}

Bitboard Position::index_captures(Square s, Piece p) {
#ifdef _BMI2_
	int file = get_file(s);
	int rank = get_rank(s);
	int ldiag = get_ldiag(s);
	int rdiag = get_rdiag(s);

#ifdef _BIGC_
	// Assumes the square is empty
	Bitboard upper = ~pieces[EMPTY]; // occupied bits
	Bitboard lower = pieces[~p] | SquareBoard[s]; // them bits + set(misc)

	uint16_t f_idx  = extract_bits(upper, FileBoard[file]) << 8;
	uint16_t r_idx  = extract_bits(upper, RankBoard[rank]) << 8;
	uint16_t ld_idx = extract_bits(upper, LDiagBoard[ldiag]) << 8;
	uint16_t rd_idx = extract_bits(upper, RDiagBoard[rdiag]) << 8;

	f_idx  |= extract_bits(lower, FileBoard[file]);
	r_idx  |= extract_bits(lower, RankBoard[rank]);
	ld_idx |= extract_bits(lower, LDiagBoard[ldiag]);
	rd_idx |= extract_bits(lower, RDiagBoard[rdiag]);

	uint8_t file_c = Captures[f_idx];
	uint8_t rank_c = Captures[r_idx];
	uint8_t ldiag_c = Captures[ld_idx];
	uint8_t rdiag_c = Captures[rd_idx];

	return to_file(file_c, file) | to_rank(rank_c, rank)
	| to_ldiag(ldiag_c, ldiag) | to_rdiag(rdiag_c, rdiag);

#else

	Bitboard captures = EmptyBoard;
	Bitboard ue = ~pieces[~p];
	Bitboard us = pieces[p];
	int lidx = get_lidx(s);
	int ridx = get_ridx(s);

	uint8_t a, b, c;

	a = Captures[rank][extract_bits(ue, FileBoard[file])];
	b = Captures[rank][extract_bits(us, FileBoard[file])];
	c = (a & b) ^ Captures[rank][a ^ b];
	captures |= to_file(c, file);

	a = Captures[file][extract_bits(ue, RankBoard[rank])];
	b = Captures[file][extract_bits(us, RankBoard[rank])];
	c = (a & b) ^ Captures[file][a ^ b];
	captures |= to_rank(c, rank);

	a = Captures[lidx][extract_bits(ue, LDiagBoard[ldiag])];
	b = Captures[lidx][extract_bits(us, LDiagBoard[ldiag])];
	c = (a & b) ^ Captures[lidx][a ^ b];
	captures |= to_ldiag(c, ldiag);

	a = Captures[ridx][extract_bits(ue, RDiagBoard[rdiag])];
	b = Captures[ridx][extract_bits(us, RDiagBoard[rdiag])];
	c = (a & b) ^ Captures[ridx][a ^ b];
	captures |= to_rdiag(c, rdiag);
	
	return captures;
#endif

#else
	Bitboard captures = EmptyBoard;

	captures |= add_captures<-9>(s, pieces[p], pieces[~p] & ~FileBoard[7]);
	captures |= add_captures<-8>(s, pieces[p], pieces[~p]);
	captures |= add_captures<-7>(s, pieces[p], pieces[~p] & ~FileBoard[0]);
	captures |= add_captures<-1>(s, pieces[p], pieces[~p] & ~FileBoard[7]);
	captures |= add_captures< 1>(s, pieces[p], pieces[~p] & ~FileBoard[0]);
	captures |= add_captures< 7>(s, pieces[p], pieces[~p] & ~FileBoard[7]);
	captures |= add_captures< 8>(s, pieces[p], pieces[~p]);
	captures |= add_captures< 9>(s, pieces[p], pieces[~p] & ~FileBoard[0]);
	
	return captures;
#endif
}

// place/remove/move functions - dont handle key xor
inline void Position::place(Piece p, Square s) {
	squares[s] = p;
	pieces[p] ^= SquareBoard[s];
	pieces[EMPTY] ^= SquareBoard[s];

	piece_count[EMPTY]--;
	piece_count[p]++;
}

inline void Position::remove(Piece p, Square s) {
	squares[s] = EMPTY;
	pieces[p] ^= SquareBoard[s];
	pieces[EMPTY] ^= SquareBoard[s];

	piece_count[EMPTY]++;
	piece_count[p]--;
}

void Position::rebuild() { // computes others from squares data.
	for (int i = 0; i < 3; i++) { pieces[i] = 0; piece_count[i] = 0; }
	
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
	undo_stack->del = true;
	net = n;
}

Position::~Position()
{
	clear_stack();
	delete undo_stack;
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

	cout << *this << "\n\n" << testpos << endl;
}

void Position::set_squares() {
	Bitboard e = pieces[EMPTY];
	Bitboard b = pieces[BLACK_P];

	int sqc[3] = { };
	for (Square s = A1; s < SQ_END; ++s) {
		squares[s] = e & 1 ? EMPTY :
			b & 1 ? BLACK_P : WHITE_P;

		e >>= 1;
		b >>= 1;
		sqc[squares[s]]++;
	}

	piece_count[EMPTY]   = popcount(pieces[EMPTY]);
	piece_count[BLACK_P] = popcount(pieces[BLACK_P]);
	piece_count[WHITE_P] = popcount(pieces[WHITE_P]);
}

void Position::show() {
	cout << *this;
}

ostream& operator<<(ostream& os, Position& pos) {
	// Show board
	Square sq = A1;
	for (int i = 0; i < 8; i++) {
		os << i + 1 << '|';
		for (int j = 0; j < 8; j++) {
			os << print_piece(pos.squares[sq]) << '|';
			++sq;
		}
		os << "\n-+-+-+-+-+-+-+-+-+-\n";
	}
	os << "  a|b|c|d|e|f|g|h\n";

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
	compute_L0(undo_stack->accumulator, squares, net);

	rebuild();
}

void Position::do_move(Square s, Undo* new_undo) {
	memcpy(new_undo, undo_stack, sizeof(Undo));
	new_undo->s = s;
	new_undo->prev = undo_stack;
	new_undo->pass = false;
	new_undo->del = false;
	
	undo_stack = new_undo;

	// Handle captures
	Piece p = side_to_move ? WHITE_P : BLACK_P;
	Bitboard captures = index_captures(s, p);

	pieces[~p] ^= captures;
	pieces[p] ^= captures;

	new_undo->captured = captures;
	while (captures) {
		Square c = pop_lsb(&captures);
		
		update_L0(new_undo->accumulator, c, ~p, p, net);

		squares[c] = p;

		piece_count[p]++;
		piece_count[~p]--;

		new_undo->key ^= piece_keys[~p][c] ^ piece_keys[p][c];
	}

	// Place the new piece
	update_L0(new_undo->accumulator, s, EMPTY, p, net);

	place(p, s);
	new_undo->key ^= piece_keys[p][s];
	
	side_to_move = ~side_to_move;
	new_undo->key ^= side_to_move_key;
}

void Position::undo_move() {
	Square s = undo_stack->s;
	if (s == NULL_MOVE) { undo_null_move(); return; }

	Piece p = side_to_move ? WHITE_P : BLACK_P; // Piece to place again
	Bitboard captured = undo_stack->captured;

	pieces[~p] ^= captured;
	pieces[p] ^= captured;
	remove(~p, s);

	while (captured) {
		Square c = pop_lsb(&captured);
		squares[c] = p;

		piece_count[p]++;
		piece_count[~p]--;
	}

	side_to_move = ~side_to_move;
	pop_stack();
}

void Position::do_null_move(Undo* new_undo) {
	memcpy(new_undo, undo_stack, sizeof(Undo));
	new_undo->s = NULL_MOVE;
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

void Position::do_move_fast(Square s) {
	Piece p = side_to_move ? WHITE_P : BLACK_P;
	Bitboard captures = index_captures(s, p);

	pieces[~p] ^= captures;
	pieces[p] ^= captures;
	pieces[p] ^= SquareBoard[s];
	pieces[EMPTY] ^= SquareBoard[s];
	side_to_move = ~side_to_move;
}

void Position::do_null_fast() {
	side_to_move = ~side_to_move;
}

Position& Position::operator=(const Position& board) {
	Net* temp = net;
	clear_stack();
	Undo* t = undo_stack;
	memcpy(this, &board, sizeof(Position));
	undo_stack = t;
	memcpy(undo_stack, board.undo_stack, sizeof(Undo));
	undo_stack->prev = nullptr;
	undo_stack->del = true;
	net = temp;
	return *this;
}