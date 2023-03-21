#ifndef INTERPRET_H
#define INTERPRET_H
#include <vector>
#include <string>
#include <iostream>
#include <deque>
#include <cstring>
#include "compile.h"
#include "transform.h"

int interpret(std::vector<std::uint8_t>& method, const std::string& text, std::string& out);

int applyOperation(std::string& result, const std::string& tempVar, const std::string& secondVar, const std::string& type);

int applyTransform(std::string& input, const std::string& type, const std::vector<std::string>& args);


std::uint16_t getConstOffset(const std::vector<std::uint8_t>& method, std::uint16_t off, std::uint16_t num);
#endif
