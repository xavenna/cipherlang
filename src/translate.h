#ifndef TRANSLATE_H
#define TRANSLATE_H
#include <xcipher.hpp>
#include <fstream>
#include <iostream>
#include "compile.h"

int transformText(const std::string&, const std::string&, const std::string&, const std::string&, bool);


int convertToMethod(std::vector<std::uint8_t>& output, const std::string& input);
#endif
