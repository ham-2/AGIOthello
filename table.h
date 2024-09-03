#ifndef TABLE_INCLUDED
#define TABLE_INCLUDED

#include <cstring>
#include <thread>
#include <mutex>

#include "position.h"

using namespace std;

extern int num_threads;

class spinlock {
	std::atomic_flag locked = ATOMIC_FLAG_INIT;
public:
	void lock() {
		while (locked.test_and_set(std::memory_order_acquire)) { ; }
	}
	void unlock() {
		locked.clear(std::memory_order_release);
	}
};

struct TTEntry {
	Key key;
	int table_sn;
	Square nmove;
	short depth;
	short eval;
	spinlock m;
	Key pad;
};

struct TT {
	TTEntry* table;
		
	static size_t TT_MB_SIZE;
	static uint64_t TT_LENGTH;
	static int tt_sn;

	TT();
	~TT();

	int probe(Key key, TTEntry* probe);
	void register_entry(Key key, int depth, int eval, Square move);
	TTEntry* backup_entry(Key key);
	void write_entry(TTEntry* ptr);
	void clear_entry(Key key);
	void clear();
	void change_size(size_t new_size);
	void increment() { tt_sn++; }
};
	

extern TT Main_TT;

#endif