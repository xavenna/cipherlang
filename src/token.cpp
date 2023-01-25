#include <iostream>
#include "token.h"
#include "util.h"


std::string tokenInfo(Token t) {
  std::string out;
  switch(t.type) {
	case Token::Var:
	  out = "Var";
	  break;
	case Token::Const:
	  out = "Const";
	  break;
	case Token::Load:
	  out = "Load";
	  break;
	case Token::Apply:
	  out = "Apply";
	  break;
	case Token::Write:
	  out = "Write";
	  break;
	case Token::To:
	  out = "To";
	  break;
	case Token::From:
	  out = "From";
	  break;
	case Token::Operation:
	  out = "Operation:"+t.name+";";
	  for(auto x : t.args) {
        out += x+',';
	  }
	  break;
	case Token::Transform:
	  out = "Transform:"+t.name+";";
	  for(auto x : t.args) {
        out += x+',';
	  }
	  break;
	case Token::Identifier:
	  out = "Identifier:"+t.name;
	  break;
	case Token::File_Operation:
	  out = "File Operation:"+t.name;
	  break;
	case Token::String_Constant:
	  out = "String Constant:\""+t.name+"\"";
	  break;
	case Token::Directive:
	  out = "Directive:"+t.name;
	  break;
	case Token::End_Statement:
	  out = "End Statement";
	  break;
  }
  return out;
}

void tokenizeString(const std::vector<std::string>& script, std::vector<Token>& tlist) {


	// do some things first

	unsigned lineCounter = 0;
	for(auto line : script) {
		//check the size of the line
		if(line.size() == 0) {
			continue;
		}

		if(line[0] == '#') {
			//comment
			continue;
		}
		else {
			//tokenize line
			tokenizeLine(line, tlist);
		}
	}

}

bool tokenizeLine(const std::string& line, std::vector<Token>& tlist) {
	std::vector<std::string> elements;
	parse(line, elements);
	//convert each element to a token
	//determine which type of token each element is
	Token t;
	for(auto x : elements) {
		if(x.size() > 0) { //a token of size 1 shouldn't be possible, but i'd rather be safe than sorry
			if(x[0] == '_') {
				//a special variable
				t.type = Token::Identifier;
				t.name = x;
				//don't omit the underscore, as it is part of the var name
			} else if(x[0] == '$') {
				//operation
				if(x.size() == 1) {
					std::cout << "Error: Invalid Token "<< x << '\n';
					return false;
				}
				t.type = Token::Operation;
				t.name = x.substr(1);
			} else if(x[0] == '"') {
				//string constant
				t.type = Token::String_Constant;
				t.name = x.substr(1,x.size()-2);
			} else if(x[0] == '!') {
				//file operation
				if(x.size() == 1) {
					std::cout << "Error: Invalid Token "<< x << '\n';
					return false;
				}
				t.type = Token::File_Operation;
				t.name = x.substr(1);
			} else if(x[0] == '%') {
				//directive
				t.type = Token::Directive;
				t.name = x.substr(1);

			} else if(x.back() == '>') {
				//transform
				// ensure form is correct
				if(x.find('<') == std::string::npos || x[0] == '<') {
					//bad form, error
					std::cout << "Error: Invalid Token "<< x << '\n';
					return false;
				}
				t.type = Token::Transform;
				t.name = x.substr(0, x.find('<'));
				x = x.substr(x.find('<'));
				x = x.substr(1, x.size()-2);
				genericParse(x, t.args, ",");
			} else if(x == "var") {  // these are all keywords, with no arguments
				t.type = Token::Var;
			} else if(x == "const") {
				t.type = Token::Const;
			} else if(x == "load") {
				t.type = Token::Load;
			} else if(x == "write") {
				t.type = Token::Write;
			} else if(x == "apply") {
				t.type = Token::Apply;
			} else if(x == "to") {
				t.type = Token::To;
			} else if(x == "from") {
				t.type = Token::From;
			} else {
				//attempt to determine type of token based on contents of text
				//it is most likely an identifier
				t.type = Token::Identifier;
				t.name = x;
			}
			tlist.push_back(t);
		}
	}
	if(elements.size() != 0) {
		tlist.push_back(Token());
		tlist.back().type = Token::End_Statement;
	}
	//most error checking is handled in later stages
	return true;
}
