#ifndef COMPILE_H
#define COMPILE_H
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include "token.h"
#include "util.h"

#define VAR_OFFSET 0
#define CONST_OFFSET 256
#define SPECIAL_VAR_OFFSET 768

#define NUM_SPECIAL_VARS 11

#define MAX_CONST_LEN 256
#define CONST_HEADER_WIDTH 2

#define HEADER_LEN 16

struct BinaryInstruction {
  std::uint8_t opcode;
  std::uint16_t first;
  std::uint16_t second;
  std::uint16_t third;
  std::uint8_t argc;
  std::vector<std::uint16_t> args;

  std::vector<std::uint8_t> binary();
};

struct Ident {
  bool isVar;
  std::uint16_t number;
};

struct ProtoInstruction {
  enum Type {
	Load      = 0,
	Apply     = 1,
	Operation = 2,
	Nil      = -1
  };
  Type type;
  std::string source;
  std::string second_source;
  std::string target;
  std::string value;
  std::vector<std::string> args;

};

int compileMethod(std::vector<Token>& tlist, std::vector<std::uint8_t>& method);
int createHeader(std::vector<std::uint8_t>& header, int argmin, int argmax, int numConsts, unsigned textLen, unsigned varNum);



int organizeTokens(const std::vector<Token>& tlist, std::vector<Statement>& statements);
int locateVars(const std::vector<Statement>& statements, std::vector<std::string>& variables, std::vector<std::string>& constants, std::map<std::string, std::string>& constvals);

int resolveReferences(std::vector<ProtoInstruction>& instructions, std::vector<std::string>& variables, std::vector<std::string>& constants, std::map<std::string, std::string>& constvals);
bool updateReference(const std::vector<std::string>& identList, std::string& ident, int offset);

int consolidate(std::vector<ProtoInstruction>& instructions, const std::vector<Statement>& statements);


int binarify(const std::vector<ProtoInstruction>& instructions, std::vector<BinaryInstruction>& bvec);

int generate_text(std::vector<std::uint8_t>& text, const std::vector<BinaryInstruction>& bvec);
int generate_const(std::vector<std::uint8_t>& table, const std::map<std::string, std::string>& value_hash, std::vector<std::string> constants);


int assemble(std::vector<std::uint8_t>& method, const std::vector<std::uint8_t>& head, const std::vector<std::uint8_t>& text, const std::vector<std::uint8_t>& data);

int getSpecialVarNum(const std::string& name);
bool isValidSpecialVar(const std::string& name);
#endif
