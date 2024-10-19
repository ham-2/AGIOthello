#include "options.h"

using namespace std;

void print_option() {
	Threads.acquire_cout();
	cout << "option name Threads type spin default 1 min 1 max " << THREADS_MAX << '\n'
		<< "option name Hash type spin default 1 min 1 max 64 \n"
		<< endl;
	Threads.release_cout();
}

void set_option(istringstream& ss) {
	string word;
	ss >> skipws >> word;
	if (word == "name") {
		ss >> word;
		if (word == "Threads") {
			ss >> word;
			if (word == "value") {
				int new_threads;
				ss >> new_threads;
				Threads.set_threads(new_threads);
			}
		}

		else if (word == "Hash") {
			ss >> word;
			if (word == "value") {
				int new_size;
				ss >> new_size;
				if (new_size < 1) { new_size = 1; }
				else {
					while ((new_size & (new_size - 1)) != 0) { new_size--; }
					if (new_size > 64) { new_size = 64; }
				}
				Main_TT.change_size((size_t)(new_size));
			}
		}
	}
}