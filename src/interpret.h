#ifndef INTERPRET_H
#define INTERPRET_H
#include <vector>
#include <string>
#include <iostream>
#include <deque>
#include <cstring>
#include "compile.h"

int interpret(std::vector<std::uint8_t>& method, const std::string& text, std::string& out);

#endif
