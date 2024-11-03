#include "book.h"

#include <queue>
#include <vector>

constexpr uint64_t mask_upper56 = ~(~0ULL >> 56);
constexpr uint64_t mask_lower08 = ~0ULL >> 56;
constexpr uint64_t mask_lower11 = ~0ULL >> 53;
constexpr int BOOK_WINDOW_PROBE = 6 << (EVAL_BITS - 6);
constexpr int BOOK_WINDOW_WRITE = 3 << (EVAL_BITS - 6);

inline BookEntry make_entry(Key key, Square nmove) {
	return (key & mask_upper56) | uint64_t(nmove);
}

inline Square probe_entry(BookEntry* t, Key key, uint64_t mask)
{
	BookEntry entry = *(t + (key & mask));
	return (key & mask_upper56) == (entry & mask_upper56) ?
		Square(entry & mask_lower08) : SQ_END;
}

Square probe_entry(BookEntry* book, Position* board) {
	return probe_entry(book, board->get_key(), mask_lower11);
}

struct ExtendedEntry {
	uint32_t moves1;
	uint32_t moves2;
	uint32_t moves3;
	int eval;
};

constexpr uint64_t mask_lower32 = (1ULL << 32) - 1;
constexpr uint64_t mask_lower16 = (1ULL << 16) - 1;

void pack_moves(ExtendedEntry* dst, Square* src) {
	uint64_t lower = 0;
	for (int i = 0; i < 8; i++) {
		lower |= uint64_t(src[i] & 63) << (i * 6);
	}
	uint64_t upper = 0;
	for (int i = 8; i < 16; i++) {
		upper |= uint64_t(src[i] & 63) << ((i - 8) * 6);
	}

	dst->moves1 = uint32_t(lower & mask_lower32);
	dst->moves2 = uint32_t(((upper & mask_lower16) << 16) |
		               (lower >> 32));
	dst->moves3 = uint32_t(upper >> 16);
}

void unpack_moves(Square* dst, ExtendedEntry* src) {
	uint64_t lower = uint64_t(src->moves1) | (uint64_t(src->moves2) << 32);
	for (int i = 0; i < 8; i++) {
		dst[i] = Square(lower & 63);
		lower >>= 6;
	}
	uint64_t upper = uint64_t(src->moves2) | (uint64_t(src->moves3) << 32);
	upper >>= 16;
	for (int i = 8; i < 16; i++) {
		dst[i] = Square(upper & 63);
		upper >>= 6;
	}
}

void view_ee(ExtendedEntry* entry) {
	Square moves[16] = { D4, D4, D4, D4, D4, D4, D4, D4,
						 D4, D4, D4, D4, D4, D4, D4, D4 };
	unpack_moves(moves, entry);
	cout << "e6";
	for (int i = 0; i < 16; i++) {
		if (moves[i] == D4) { break; }
		cout << ' ' << moves[i];
	}
	cout << ' ' << entry->eval << " (" << Square(entry->eval) << ")" << endl;
}

struct ExtendedBook {
	vector<ExtendedEntry>* list;
	queue<ExtendedEntry>* queue;
	mutex m;
};

ExtendedEntry* load_book(uint64_t* len, string filename) {
	uintmax_t fs = std::filesystem::file_size(filename);
	*len = fs / sizeof(ExtendedEntry);
	ExtendedEntry* dst = new ExtendedEntry[*len];
	load_file((char*)dst, filename, fs);
	return dst;
}

void _build_book(Thread* t, ExtendedBook* book, int target_depth)
{
	TT* table = new TT;
	SearchParams sp = {
		t->board, &Threads.stop, table, t->step
	};

	while (!*sp.stop) {
		book->m.lock();
		while (book->queue->size() == 0 && (!*sp.stop)) {
			book->m.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			book->m.lock();
		}
		if (*sp.stop) { book->m.unlock(); break; }
		ExtendedEntry entry = book->queue->front();
		book->queue->pop();
		book->m.unlock();

		table->clear();
		Square moves[16] = { D4, D4, D4, D4, D4, D4, D4, D4,
							 D4, D4, D4, D4, D4, D4, D4, D4 };
		unpack_moves(moves, &entry);
		Position* board = sp.board;
		Bitboard c;
		board->set(startpos_fen);
		board->do_move(E6, &c);
		int depth = 0;
		while (depth < 16 && moves[depth] != D4) {
			board->do_move(moves[depth], &c);
			depth++;
		}

		MoveList ml;
		ml.generate(*board);

		Key root_key = board->get_key();

		Square s;
		Square nmove = D4;
		int comp_eval;
		int new_eval = EVAL_INIT;
		int evals[33];

		for (int i = 0; i < ml.length(); i++) {
			s = ml.list[i];

			// Do move
			Bitboard c;
			board->do_move(s, &c);
			TTEntry probe_m = {};

			int r = 1;
			while (r <= 11) {
				comp_eval = -alpha_beta(&sp,
					&probe_m, r,
					-BOOK_WINDOW_PROBE,
					BOOK_WINDOW_PROBE);
				r++;

				if (r > 8 && probe_m.type == -1) { break; }
			}
			evals[i] = comp_eval;

			if (comp_eval > new_eval) {
				new_eval = comp_eval;
				nmove = s;
			}

			board->undo_move(s, &c);
		}

		if (depth < target_depth &&
			new_eval > -BOOK_WINDOW_PROBE &&
			new_eval < BOOK_WINDOW_PROBE) {
			int probe_cutoff = max(-BOOK_WINDOW_PROBE, new_eval - BOOK_WINDOW_PROBE);

			for (int i = 0; i < ml.length(); i++) {
				if (evals[i] < probe_cutoff) { continue; }
				s = ml.list[i];
				moves[depth] = s;
				ExtendedEntry next = {};
				pack_moves(&next, moves);
				book->m.lock();
				book->queue->push(next);
				book->m.unlock();
			}
		}

		entry.eval = new_eval;
		book->m.lock();
		if (*sp.stop) {
			book->queue->push(entry);
		}
		else {
			book->list->push_back(entry);
		}
		book->m.unlock();
	}

	delete table;
}

void continue_bigbook(int num_threads, bool c, int depth)
{
	vector<Thread*> threads;
	ExtendedBook book = {};
	book.list = new vector<ExtendedEntry>;
	book.queue = new queue<ExtendedEntry>;
	
	if (c) {
		uint64_t len;
		ExtendedEntry* q = load_book(&len, "queue.tob");
		for (uint64_t i = 0; i < len; i++) {
			book.queue->push(q[i]);
		}
		cout << "Queue: " << len << '\n';
		ExtendedEntry* l = load_book(&len, "bigbook.tob");
		for (uint64_t i = 0; i < len; i++) {
			book.list->push_back(l[i]);
		}
		cout << "Book: " << len << endl;
		free(q);
		free(l);
	}
	else {
		Square moves[16] = { D4, D4, D4, D4, D4, D4, D4, D4,
					         D4, D4, D4, D4, D4, D4, D4, D4 };
		ExtendedEntry ie = {};
		pack_moves(&ie, moves);
		book.queue->push(ie);
	}

	Threads.stop = false;

	for (int i = 0; i < num_threads; i++) {
		Thread* new_thread = new Thread(i, Threads.n);
		threads.push_back(new_thread);
		new_thread->t = new thread(_build_book, new_thread, &book, depth);
	}

	for (int i = 0; i < num_threads; i++) {
		threads[i]->t->join();
		delete threads[i]->t;
	}

	save_file((char*)book.list->data(), "bigbook.tob",
		sizeof(ExtendedEntry) * book.list->size());
	book.list->clear();
	while (!book.queue->empty()) {
		book.list->push_back(book.queue->front());
		book.queue->pop();
	}
	save_file((char*)book.list->data(), "queue.tob",
		sizeof(ExtendedEntry) * book.list->size());
}

ExtendedEntry* index_bigbook(ExtendedEntry* book, uint64_t book_length, ExtendedEntry* p) {
	for (uint64_t i = 0; i < book_length; i++) {
		if (book[i].moves1 != p->moves1) { continue; }
		if (book[i].moves2 != p->moves2) { continue; }
		if (book[i].moves3 != p->moves3) { continue; }
		return book + i;
	}
	return nullptr;
}

void view_book(string filename) {
	uint64_t len = 0;
	ExtendedEntry* book = load_book(&len, filename);
	for (uint64_t i = 0; i < len; i++) {
		view_ee(book + i);
	}
	free(book);
}

int _correct_eval(SearchParams* sp, 
	               ExtendedEntry* src, uint64_t len,
	               ExtendedEntry* curr) {
	Position* board = sp->board;
	Square moves[16] = { D4, D4, D4, D4, D4, D4, D4, D4,
					     D4, D4, D4, D4, D4, D4, D4, D4 };
	unpack_moves(moves, curr);
	int depth = 0;
	while (depth < 16 && moves[depth] != D4) { depth++; }
	ExtendedEntry* curr_b = index_bigbook(src, len, curr);
	if (curr_b == nullptr) { return -EVAL_INIT; }
	ExtendedEntry next = {};

	int new_eval = EVAL_INIT;
	if (depth < 16) {
		MoveList ml;
		ml.generate(*board);
		for (Square* s = ml.list; s < ml.end; s++) {
			Bitboard c;
			board->do_move(*s, &c);
			moves[depth] = *s;
			pack_moves(&next, moves);
			int comp_eval = -_correct_eval(sp, src, len, &next);
			if (comp_eval > new_eval) {
				new_eval = comp_eval;
			}
			board->undo_move(*s, &c);
		}
	}

	if (new_eval == EVAL_INIT) { return curr_b->eval; }
	curr_b->eval = new_eval;
	return new_eval;
}

void _prune_book(SearchParams* sp,
	vector<ExtendedEntry>* dst, 
	ExtendedEntry* src, uint64_t len,
	ExtendedEntry* curr,
	bool so_b, bool so_w) 
{
	Position* board = sp->board;
	Square moves[16] = { D4, D4, D4, D4, D4, D4, D4, D4,
						 D4, D4, D4, D4, D4, D4, D4, D4 };
	unpack_moves(moves, curr);
	int depth = 0;
	while (depth < 16 && moves[depth] != D4) { depth++; }
	ExtendedEntry* curr_b = index_bigbook(src, len, curr);
	if (curr_b == nullptr) { return; }
	
	bool so = (board->get_side() == BLACK && (!so_b)) ||
		      (board->get_side() == WHITE && (!so_w));
	ExtendedEntry next = {};

	Square nmove = SQ_END;
	int new_eval = EVAL_INIT;
	if (depth < 16) {
		MoveList ml;
		ml.generate(*board);
		for (Square* s = ml.list; s < ml.end; s++) {
			Bitboard c;
			board->do_move(*s, &c);
			moves[depth] = *s;
			pack_moves(&next, moves);
			ExtendedEntry* next_b = index_bigbook(src, len, &next);
			int comp_eval = next_b == nullptr ? EVAL_INIT : -next_b->eval;
			if (comp_eval > new_eval) {
				new_eval = comp_eval;
				nmove = *s;
			}
			if (comp_eval == curr_b->eval) {
				_prune_book(sp, dst, src, len, &next, so_b, so_w);
			}
			else if (board->get_side() == BLACK && so_w == false) {
				_prune_book(sp, dst, src, len, &next, true, false);
			}
			else if (board->get_side() == WHITE && so_b == false) {
				_prune_book(sp, dst, src, len, &next, false, true);
			}
			board->undo_move(*s, &c);
		}
	}

	if (new_eval == EVAL_INIT) { return; }
	curr_b->eval = int(nmove);
	dst->push_back(*curr_b);
}

void prune_book() {
	uint64_t book_len = 0;
	ExtendedEntry* book = load_book(&book_len, "bigbook.tob");
	SearchParams sp = {
		Threads.board, &(Threads.stop), &Main_TT, 1
	};

	Bitboard c;
	Threads.board->do_move(E6, &c);

	Square moves[16] = { D4, D4, D4, D4, D4, D4, D4, D4,
					     D4, D4, D4, D4, D4, D4, D4, D4 };
	ExtendedEntry ie = {};
	pack_moves(&ie, moves);
	_correct_eval(&sp, book, book_len, &ie);
	save_file((char*)book, "book_corrected.tob", sizeof(ExtendedEntry) * book_len);
	vector<ExtendedEntry> pruned;
	_prune_book(&sp, &pruned, book, book_len, &ie, false, false);
	Threads.board->undo_move(E6, &c);
	cout << "length: " << pruned.size() << endl;

	save_file((char*)pruned.data(), "book_pruned.tob", sizeof(ExtendedEntry) * pruned.size());
	free(book);
}