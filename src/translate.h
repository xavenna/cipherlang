#ifndef TRANSLATE_H
#define TRANSLATE_H
#include <xcipher.hpp>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "compile.h"

namespace fs = std::filesystem;

int transformText(const std::string&, const std::string&, const std::string&, const std::string&, bool);


int convertToMethod(std::vector<std::uint8_t>& output, const std::string& input);
#endif
