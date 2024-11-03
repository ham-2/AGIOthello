#include <vector>

#ifdef _WIN64
#include <windows.h>
#include <memoryapi.h>
#else
#include <stdlib.h>
#endif

#include "table.h"

using namespace std;

	int num_threads = 1;
	
	void* table_malloc(size_t size) {
#ifdef _WIN64
		return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
		return aligned_alloc(4096, size);
#endif
	}

	void table_free(void* table) {
#ifdef _WIN64
		VirtualFree(table, 0, MEM_RELEASE);
#else
		free(table);
#endif
	}

size_t TT::TT_MB_SIZE = 1;
uint64_t TT::TT_LENGTH = TT::TT_MB_SIZE * 1024 * 1024 / sizeof(TTEntry);

uint64_t SIZE_NUM = (uint64_t)(TT::TT_LENGTH - 1);

TT Main_TT;
int TT::tt_sn;

TT::TT() {
	table = static_cast<TTEntry*>(table_malloc(TT_LENGTH * sizeof(TTEntry)));
}

TT::~TT() {
	table_free(table);
}

int TT::probe(Key key, TTEntry* probe) {
	TTEntry* entry_ptr = table + (key & SIZE_NUM);
	entry_ptr->m.lock();
	memcpy(probe, entry_ptr, offsetof(TTEntry, m));
	entry_ptr->m.unlock();
	return 0;
}

void TT::register_entry(Key key, int eval, Square move, uint8_t depth, int8_t type) {
	TTEntry* entry_ptr = table + (key & SIZE_NUM);
	int r = depth + (type == 0 ? 4 : 0);
	entry_ptr->m.lock();
	if (depth >= entry_ptr->depth ||
		tt_sn > entry_ptr->table_sn) {
		entry_ptr->key = key;
		entry_ptr->eval = eval;
		entry_ptr->nmove = move;
		entry_ptr->table_sn = tt_sn;
		entry_ptr->depth = depth;
		entry_ptr->type = type;
	}
	entry_ptr->m.unlock();
	return;
}

void TT::write_entry(TTEntry* ptr) {
	TTEntry* entry_ptr = table + (ptr->key & SIZE_NUM);
	entry_ptr->m.lock();
	if (ptr->depth >= entry_ptr->depth ||
		tt_sn > entry_ptr->table_sn) {
		memcpy(entry_ptr, ptr, sizeof(TTEntry) - offsetof(TTEntry, m));
	}
	entry_ptr->m.unlock();
}

void TT::clear_entry(Key key) {
	TTEntry* entry_ptr = table + (key & SIZE_NUM);
	entry_ptr->m.lock();
	entry_ptr->key = 0;
	entry_ptr->depth = 0;
	entry_ptr->eval = 0;
	entry_ptr->nmove = Square(0);
	entry_ptr->table_sn = 0;
	entry_ptr->type = 0;
	entry_ptr->m.unlock();
	return;
}

void TT::clear() {
	tt_sn = 1;
	std::memset(table, 0, TT_LENGTH * sizeof(TTEntry));
}

void TT::change_size(size_t new_size) {
	table_free(table);
	TT_MB_SIZE = new_size;
	TT_LENGTH = TT_MB_SIZE * 1024 * 1024 / sizeof(TTEntry);
	SIZE_NUM = (uint64_t)(TT::TT_LENGTH - 1);
	table = static_cast<TTEntry*>(table_malloc(TT_LENGTH * sizeof(TTEntry)));
}

void TT::check_full()
{
	uint64_t count = 0;
	for (int i = 0; i < TT_LENGTH; i++) {
		if ((table + i)->key != 0) { count++; }
	}

	cout << "TTFull: " << double(count) * 100 / TT_LENGTH << "% "
		<< '(' << count << '/' << TT_LENGTH << ')' << endl;
}

void getpv(ostream& os, Position* board) {
	string pv;
	TTEntry probe = {};
	Main_TT.probe(board->get_key(), &probe);

	if (probe.key == board->get_key() &&
		probe.nmove != SQ_END &&
		probe.nmove != GAME_END) {
		Bitboard c;
		board->do_move_wrap(probe.nmove, &c);
		os << probe.nmove << " ";
		getpv(os, board);
		board->undo_move_wrap(probe.nmove, &c);
	}
}
