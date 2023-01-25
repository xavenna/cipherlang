#include "compile.h"


int compileMethod(std::vector<Token>& tlist, std::vector<std::uint8_t>& method) {
	std::cout << "Compiling Method" << std::endl;

  //check for directives, ensure argmin and argmax
  int argmin=-1;
  int argmax=-1;
  std::vector<std::string> directives;
  for(unsigned i=0;i<tlist.size();i++) {
	if(tlist[i].type == Token::Directive) {
      //handle directives
	  //parse tlist[i].name
	  genericParse(tlist[i].name, directives, ",");
	  tlist.erase(tlist.begin()+i);
	  i--;
	}
  }

  //handle directives
  for(auto x : directives) {
	//should be of the form: name=value
	if(x.size() < 3) {
	  std::cout << "Error: directive is of insufficient size\n";
	  return 1;
	}
	if(x.find('=') == std::string::npos) {
	  std::cout << "Error: directive is missing an equal sign\n";
	  return 1;
	}
	if(x[0] == '=') { 
	  std::cout << "Error: directive is missing a name\n";
	}
	if(x.back() == '=') {
	  std::cout << "Error: directive is missing a key\n";
	}
    std::string name = x.substr(0,x.find('='));
	std::string value = x.substr(x.find('=')+1);

	if(name == "argmin") {
	  //set argmin
	  if(argmin != -1) {
	    std::cout << "Error: repeated directive 'argmin'\n";
		return 1;
	  }
	  if(!isNum(value)) {
		std::cout << "Error: key for argmin must be an integer\n";
		return 1;
	  }
	  int s = std::stoi(value);
	  if(s < 0) {
		std::cout << "Error: key for argmin must be positive\n";
		return 1;
	  }
	  argmin = s;
	}
	else if(name == "argmax") {
	  if(argmax != -1) {
	    std::cout << "Error: repeated directive 'argmax'\n";
		return 1;
	  }
	  if(!isNum(value)) {
		std::cout << "Error: key for argmax must be an integer\n";
		return 1;
	  }
	  int s = std::stoi(value);
	  if(s < 0) {
		std::cout << "Error: key for argmax must be positive\n";
		return 1;
	  }
	  argmax = s;

	}
	else {
	  std::cout << "Error: unrecognized directive '"<<name<<"'\n";
	  return 1;
	}
	
  }
  if(argmin < 0 || argmax < 0) {
    std::cout << "Error: argmin and argmax not specified\n";
	return 1;
  }


  //parse into expressions
  //make sure each statement is well formed
  std::vector<Statement> statements;
  if(organizeTokens(tlist, statements) != 0) {
	return 1;
  }

  // use each statement from 'statements' to create an instruction
  for(auto x : statements) {
    for(auto y : x) {
      //std::cout << tokenInfo(y) << ", ";
	}
	//std::cout << '\n';
  }

  //ensure each statement is valid in meaning

  //analyze variables and constants
  std::vector<std::string> variables;
  std::vector<std::string> constants;
  std::map<std::string, std::string> constvals;
  //search for all variable and const tokens and create an index
  if(locateVars(statements, variables, constants, constvals) != 0) { //error,
	return -1;
  }


  //transform each statement into one (or more) proto-instructions  
  std::vector<ProtoInstruction> instructions;
  if(consolidate(instructions, statements) != 0) {
	return -1;
  }

  // // //break identifiers into multiple instructions, if possible

  //replacing variables and constants with an appropriate reference

  //all identifiers will be replaced with a type and number, referring to their position
  //in the appropriate table

  //convert the proto-instructions into binary instructions

  //create header
  if(createHeader(method, argmin, argmax) != 0) {
	//error
	return -1;
  }

  //construct the text section of the binary method
  
  //construct the constants table

  //assemble into a complete method file
  return 0;
}


int locateVars(const std::vector<Statement>& statements, std::vector<std::string>& variables, std::vector<std::string>& constants, std::map<std::string, std::string>& constvals) {
  //in each statement, check for variable/const declarations and usage
  //when a var/const is declared, check if the identifier has been used. if so, error
  //otherwise, add it to the appropriate table.
  //when a var is used, check if it has been declared. if not, error
  //when a const is used, check if it has been declared. If not, error
  //when a const is used, check if it being modified. if so, error

	//TODO: Reserve names of special variables

  for(auto x : statements) {
    //check for var, const tokens (to find declarations)
	if(x.size() == 0) {
	  //empty statement, disregard
      continue;
	}
	if(x[0].type == Token::Var) {
	  //line is a variable declaration
	  //x.size() should equal 2
	  if(x.size() != 2) {
        std::cout << "Error: malformed variable declaration statement\n";
		return 1;
	  }
	  if(x[1].type != Token::Identifier) {
        std::cout << "Error: malformed variable declaration statement\n";
		return 1;
	  }
	  //x[1].name is the variable's name
	  if(x[1].name[0] == '_') {
		std::cout << "Error: the '_' prefix for variable names is reserved\n";
		return 1;
	  }
	  if(std::find(variables.begin(), variables.end(), x[1].name) != variables.end()) {
        std::cout << "Error: repeated variable name\n";
		return 1;
	  }
	  if(std::find(constants.begin(), constants.end(), x[1].name) != constants.end()) {
        std::cout << "Error: name has already been used for a constant\n";
		return 1;
	  }
	  variables.push_back(x[1].name);
	}
	else if(x[0].type == Token::Const) {
	  //line is a constant declaration
	  if(x.size() != 3) {
		std::cout << "Error: malformed constant declaration statement\n";
		return 1;
	  }
	  if(x[1].type != Token::Identifier || x[2].type != Token::String_Constant) {
		std::cout << "Error: malformed constant declaration statement\n";
		return 1;
	  }
	  //x[1].name is the constant's name
	  //x[2].name is the constant's value
	  if(std::find(constants.begin(), constants.end(), x[1].name) != constants.end()) {
        std::cout << "Error: repeated constant name\n";
		return 1;
	  }
	  if(std::find(variables.begin(), variables.end(), x[1].name) != variables.end()) {
        std::cout << "Error: name has already been used for a variable\n";
		return 1;
	  }
	  constants.push_back(x[1].name);
	  constvals.emplace(x[1].name, x[2].name);
	}
	else {
	  //check line to see if variables are used
	  for(auto y : x) {
		//search for identifier tokens
		if(y.type == Token::Identifier) {
		  //check if y.name is an existing variable or constant name
		  //if not, error
		  if(std::find(constants.begin(), constants.end(), y.name) == constants.end()) {
		    if(std::find(variables.begin(), variables.end(), y.name) == variables.end()) {
			  //check if y.name is a reserved special var name
			  if(!isValidSpecialVar(y.name)) {
				std::cout << "Error: use of undeclared variable '"<<y.name<<"'\n";
			    return 1;
			  }
			  else {
				//special variable located
			  }
			}
		  }
		}
	  }

	  //done with line checks
	}
	
    //next, check for identifier tokens
  }
  //i believe that is all that needs to happen
  return 0;
}

int consolidate(std::vector<ProtoInstruction>& instructions, const std::vector<Statement>& statements) {
  //for each statement, check if it is valid. If so, create a ProtoInstruction

  for(auto x : statements) {
	ProtoInstruction p;
	//check if x is a valid statement
	//check if first token is valid:
	if(x.size() == 0) {
	  //ignore empty statement
	  continue;
	}
	switch(x[0].type) {
	case Token::Var:
	  if(x.size() != 2) {
		std::cout << "Error: malformed variable declaration\n";
		return 1;
	  }
	  if(x[1].type != Token::Identifier) {
		std::cout << "Error: malformed variable declaration\n";
		return 1;
	  }
	  p.type = Token::Var; //reuse the Token::Type enum to simplify things
	  p.name = x[1].name;
	  //first and second aren't used for variable declarations
	  break;
	case Token::Const:
	  if(x.size() != 3) {
		std::cout << "Error: malformed constant declaration\n";
		return 1;
	  }
	  if(x[1].type != Token::Identifier || x[2].type != Token::String_Constant) {
		std::cout << "Error: malformed constant declaration\n";
		return 1;
	  }
	  p.type = Token::Const;
	  p.name = x[1].name;
	  p.first = x[2].name;
	  break;
	case Token::Load:
	  //more complicated
	  if(x.size() < 4) {
		std::cout << "Error: malformed load statement\n";
	    return 1;
	  }
	  if(x[1].type != Token::Identifier || x[2].type != Token::From) {
		std::cout << "Error: malformed load statement\n";
	    return 1;
	  }
	  //attempt to parse the final thing, which can be a complex operation chain : maybe use an ast?
	  // if operations are used, split into several instructions, operations are implicit and use false variables
	  break;
	case Token::Apply:
	  //more complicated
	  if(x.size() != 4) {
		std::cout << "Error: malformed apply statement\n";
		return 1;
	  }
	  if(x[1].type != Token::Transform || x[2].type != Token::To || x[3].type != Token::Identifier) {
		std::cout << "Error: malformed apply statement\n";
		return 1;
	  }

	  //transform data is stored in x[1].name
	  break;
	case Token::Write:
	  //more complicated
	  break;
	default:
	  std::cout << "Error: statement begins with an invalid token\n";
	  return 1;
	}
  }
  return 0;
}

int organizeTokens(const std::vector<Token>& tlist, std::vector<Statement>& statements) {
  Statement current;
  for(auto x : tlist) {
    switch(x.type) {
	case Token::End_Statement:
	  statements.push_back(current);
	  current.clear();
      break;
	default:
	  current.push_back(x);
      break;
	}
  }
  return 0;
}

int createHeader(std::vector<std::uint8_t>& header, int argmin, int argmax) {
  if(!header.empty()) {
    header.clear();
  }
  if(vector.size() < 8) {
    header.resize(8); //allocate space at the beginning to avoid repeated resizings
  }
  header[0] = 'C';
  header[1] = 'P';
  header[2] = 'T';
  header[3] = 'H';
  header[4] = static_cast<std::uint8_t>(MAJOR_VERSION);
  header[5] = static_cast<std::uint8_t>(MINOR_VERSION);
  header[6] = static_cast<std::uint8_t>(PATCH_NUM);
  header[7] = (argmax << 4) | (argmin);
  return 0;
}

bool isValidSpecialVar(const std::string& name) {
  std::vector<std::string> vars {
	"_", "_o", "_randU", "_randL", "_randE", "_randN", "_randA", "_argc", "_stdin",
	"_stdout", "_null"
  };
  if(name.size() == 0) {
	return false;
  }
  if(name[0] != '_') {
	return false;
  }
  if(std::find(vars.begin(), vars.end(), name) == vars.end()) {
	//check if an argument reference or filename handle
	//for the flow to get here, length has to be more than 1
	if(name[1] == 'f') {
	  //file handle, not implemented yet
		std::cout << "Error: file handles have not yet been implemented\n";
		return false;
	}
	else if(isNum(name.substr(1))) {
	  //argument reference
	  if(std::stoi(name.substr(1)) < 0) {
	  	return false;
	  }
	  else {
		return true;
	  }
	}
  }
  return true;
}
