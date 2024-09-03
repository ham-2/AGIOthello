#ifndef SEARCH_INCLUDED
#define SEARCH_INCLUDED

#include <cmath>
#include <sstream>
#include <algorithm>

#include "eval.h"
#include "position.h"
#include "threads.h"

using namespace std;

void get_time(istringstream& ss, Color c, float& time, int& max_ply);

void search_start(Thread* t, float time, int max_ply);

extern bool stop_if_mate;
extern bool ponder;
extern atomic<bool> ponder_continue;

#endif
