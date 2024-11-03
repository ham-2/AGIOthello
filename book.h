#ifndef BOOK_INCLUDED
#define BOOK_INCLUDED

#include <filesystem>

#include "alphabeta.h"
#include "threads.h"

typedef uint64_t BookEntry;

Square probe_entry(BookEntry* book, Position* board);

void continue_bigbook(int threads, bool c, int depth);

void view_book(string filename);
void prune_book();

#endif
