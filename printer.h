#ifndef PRINTER_INCLUDED
#define PRINTER_INCLUDED

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include "board.h"
#include "eval.h"
#include "search.h"
#include "table.h"
#include "threads.h"

using namespace std;

constexpr int PRINT_MIN_MS = 250;

void printer(float time, atomic<bool>* stop, condition_variable* cv);

#endif