#ifndef COMPILE_H
#define COMPILE_H
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include "token.h"
#include "util.h"

struct BinaryInstruction {
  std::uint16_t opcode;
  std::uint16_t first;
  std::uint16_t second;

};

struct Ident {
  bool isVar;
  std::uint16_t number;
};

struct ProtoInstruction {
  std::uint16_t type;
  std::string name;
  std::string first;
  std::string second;

};

int compileMethod(std::vector<Token>& tlist, std::vector<std::uint8_t>& method);
int createHeader(std::vector<std::uint8_t>& header, int argmin, int argmax);


int consolidate(std::vector<ProtoInstruction>& instructions, const std::vector<Statement>& statements);

int organizeTokens(const std::vector<Token>& tlist, std::vector<Statement>& statements);
int locateVars(const std::vector<Statement>& statements, std::vector<std::string>& variables, std::vector<std::string>& constants, std::map<std::string, std::string>& constvals);

bool isValidSpecialVar(const std::string& name);
#endif
