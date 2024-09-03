#include <cstdint>

#include "benchmark.h"

using namespace std;
using namespace std::chrono;

uint64_t _perft(Position* board, int depth) {
	if (depth == 0) {
		return 1ULL;
	}
	else {
		uint64_t value = 0;
		MoveList ml;
		ml.generate(*board);
		for (Square* s = ml.list; s < ml.end; s++) {	
			Undo u;
			board->do_move(*s, &u);
			value += _perft(board, depth - 1);
			board->undo_move(*s);
		}
		if (ml.list == ml.end) {
			if (board->get_passed()) {
				return 1ULL;
			}
			else {
				Undo u;
				board->do_null_move(&u);
				value += _perft(board, depth - 1);
				board->undo_null_move();
			}
		}
		return value;
	}
}

void perft(Position* board, int depth) {
	system_clock::time_point time_start = system_clock::now();
	milliseconds time = milliseconds(0);

	uint64_t value = 0;
	MoveList ml;
	ml.generate(*board);
	for (Square* s = ml.list; s < ml.end; s++) {
		Undo u;
		board->do_move(*s, &u);
		uint64_t sub = _perft(board, depth - 1);
		cout << *s << " " << sub << endl;
		value += sub;
		board->undo_move(*s);
	}
	cout << value << '\n';

	system_clock::time_point time_now = system_clock::now();
	time = duration_cast<milliseconds>(time_now - time_start);
	cout << "took " << time.count() << "ms, " <<
		double(value) / time.count() / 1000 << " MNps" << endl;
}