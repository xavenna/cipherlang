#ifndef TOKEN_H
#define TOKEN_H
#include <vector>
#include <string>
#include <xcipher.hpp>


struct Token {
	enum Type {
		//keywords
		Var,
		Const,
		Load,
		Apply,
		Write,
		To,
		From,

		Operation,
		Transform,
		Identifier,
		File_Operation,
		String_Constant,
		Directive,

		End_Statement
	};
	Type type;
	std::string name;

	std::vector<std::string> args;

};

std::string tokenInfo(Token);
void tokenizeString(const std::vector<std::string>& script, std::vector<Token>& tlist);

bool tokenizeLine(const std::string& line, std::vector<Token>& tlist);

typedef std::vector<Token> Statement;
#endif
