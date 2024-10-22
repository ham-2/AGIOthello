#include <sstream>
#include <bitset>
#include <memory.h>

#include "position.h"

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

template <Piece from, Piece to>
void Position::update_() {
	Bitboard b = acc_state[from] & pieces[to];
	while (b) {
		Square s = pop_lsb(&b);
		update_L0(accumulator, s, from, to, net);
	}
}

void Position::set_state() {
	update_<EMPTY, BLACK_P>();
	update_<EMPTY, WHITE_P>();
	update_<BLACK_P, EMPTY>();
	update_<BLACK_P, WHITE_P>();
	update_<WHITE_P, EMPTY>();
	update_<WHITE_P, BLACK_P>();

	for (int i = 0; i < 3; i++) { acc_state[i] = pieces[i]; }
}

Position::Position(Net* n) {
	memset(this, 0, sizeof(Position));
	net = n;
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
			os << print_piece(pos.get_piece(sq)) << '|';
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

	Net* temp2 = net;
	memset(this, 0, sizeof(Position));
	net = temp2;
	pieces[EMPTY] = FullBoard;
	empty_count = 64;

	// set pieces from rank 8.
	ss >> noskipws >> c;
	Square sq = A1;
	Piece p;
	while (c != ' ') {
		if (isdigit(c)) {
			sq += (c - '0');
		}
		else if (parse_piece(c, p)) {
			if (p != EMPTY) {
				pieces[p] ^= SquareBoard[sq];
				pieces[EMPTY] ^= SquareBoard[sq];
				empty_count--;
			}
			++sq;
		}
		ss >> c;
	}
		
	// white / black to move
	ss >> c;
	side_to_move = c == 'w' ? WHITE : BLACK;
	ss >> c;

	// write other data
	set_accumulator();
}

void Position::do_move_wrap(Square s, Bitboard* captures) {
	if (s == NULL_MOVE) { pass(); }
	else { do_move(s, captures); }
}

void Position::undo_move_wrap(Square s, Bitboard* captures) {
	if (s == NULL_MOVE) { pass(); }
	else { undo_move(s, captures); }
}

void Position::do_move(Square s, Bitboard* captures) 
{
	Piece p = side_to_move ? WHITE_P : BLACK_P;
	*captures = index_captures(s, p);
	
	pieces[~p] ^= *captures;
	pieces[p] ^= *captures ^ SquareBoard[s];
	pieces[EMPTY] ^= SquareBoard[s];
	empty_count--;
	side_to_move = ~side_to_move;
}

void Position::undo_move(Square s, Bitboard* captures)
{
	Piece p = side_to_move ? WHITE_P : BLACK_P; // Piece to place again

	pieces[~p] ^= *captures ^ SquareBoard[s];
	pieces[p] ^= *captures;
	pieces[EMPTY] ^= SquareBoard[s];
	empty_count++;
	side_to_move = ~side_to_move;
}

Position& Position::operator=(const Position& board) {
	Net* temp = net;
	memcpy(this, &board, sizeof(Position));
	net = temp;
	return *this;
}