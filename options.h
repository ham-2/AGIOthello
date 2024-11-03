#ifndef OPTIONS_INCLUDED
#define OPTIONS_INCLUDED

#include <sstream>

#include "search.h"
#include "table.h"
#include "threads.h"

constexpr int HASH_MAX = 1024;

void print_option();

void set_option(std::istringstream& ss);

#endif