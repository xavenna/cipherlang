#include "compile.h"
#include <algorithm>

//potential compilation improvements
// - const table organization, combination
// - remove unused instructions

int compileMethod(std::vector<Token>& tlist, std::vector<std::uint8_t>& method) {
  std::clog << "Compiling Method" << std::endl;

  std::vector<Statement> statements;
  std::vector<ProtoInstruction> instructions;
  std::vector<BinaryInstruction> bvec(instructions.size());

  std::vector<std::string> variables;
  std::vector<std::string> constants;
  std::map<std::string, std::string> constvals;

  std::vector<std::uint8_t> text;
  std::vector<std::uint8_t> header;
  std::vector<std::uint8_t> constTable;

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
      std::cerr << "Error: directive is of insufficient size\n";
      return 1;
    }
    if(x.find('=') == std::string::npos) {
      std::cerr << "Error: directive is missing an equal sign\n";
      return 1;
    }
    if(x[0] == '=') { 
      std::cerr << "Error: directive is missing a name\n";
      return 1;
    }
    if(x.back() == '=') {
      std::cerr << "Error: directive is missing a key\n";
      return 1;
    }
      std::string name = x.substr(0,x.find('='));
    std::string value = x.substr(x.find('=')+1);

    if(name == "argmin") {
      //set argmin
      if(argmin != -1) {
        std::cerr << "Error: repeated directive 'argmin'\n";
      return 1;
      }
      if(!isNum(value)) {
      std::cerr << "Error: key for argmin must be an integer\n";
      return 1;
      }
      int s = std::stoi(value);
      if(s < 0) {
      std::cerr << "Error: key for argmin must be positive\n";
      return 1;
      }
      argmin = s;
    }
    else if(name == "argmax") {
      if(argmax != -1) {
        std::cerr << "Error: repeated directive 'argmax'\n";
      return 1;
      }
      if(!isNum(value)) {
      std::cerr << "Error: key for argmax must be an integer\n";
      return 1;
      }
      int s = std::stoi(value);
      if(s < 0) {
      std::cerr << "Error: key for argmax must be positive\n";
      return 1;
      }
      argmax = s;

    }
    else {
      std::cerr << "Error: unrecognized directive '"<<name<<"'\n";
      return 1;
    }
	
  }
  if(argmin < 0 || argmax < 0) {
    std::cerr << "Error: argmin and argmax not specified\n";
	return 1;
  }


  //parse into expressions
  //make sure each statement is well formed
  if(organizeTokens(tlist, statements) != 0) {
    return 1;
  }

  // use each statement from 'statements' to create an instruction

  //ensure each statement is valid in meaning

  //analyze variables and constants
  //search for all variable and const tokens and create an index
  if(locateVars(statements, variables, constants, constvals) != 0) { //error,
    return -1;
  }


  //transform each statement into one (or more) proto-instructions  
  if(consolidate(instructions, statements) != 0) {
    return -1;
  }

  //replace variables and constants with an appropriate reference
  // also, relocate any strings (like args) to const table
  if(resolveReferences(instructions, variables, constants, constvals) != 0) {
    return -1;
  }

  // check for redundant proto-instructions


  //all identifiers will be replaced with a type and number, referring to their position
  //in the appropriate table

  //convert the proto-instructions into binary instructions
  if(binarify(instructions, bvec) != 0) {
    return -1;
  }
  /*  left here in case I want to do something with consts as an option
  for(auto x : constvals) {
	std::clog << x.first << ":" << x.second << '\n';
  }
  std::clog << "=====================\n";
  for(auto x : constants) {
	std::clog << x << '\n';
  }
  */

  //construct the text section of the binary method
  if(generate_text(text, bvec) != 0) {
    return -1;
  }
  
  //construct the constants table
  if(generate_const(constTable, constvals, constants) != 0) {
    return -1;
  }


  //create header
  if(createHeader(header, argmin, argmax, constvals.size(), text.size(), variables.size()) != 0) {
    //error
    return -1;
  }

  //assemble into a complete method file
  if(assemble(method, header, text, constTable) != 0) {
    return -1;
  }
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
        std::cerr << "Error: malformed variable declaration statement\n";
        return 1;
      }
      if(x[1].type != Token::Identifier) {
        std::cerr << "Error: malformed variable declaration statement\n";
        return 1;
      }
      //x[1].name is the variable's name
      if(x[1].name[0] == '_') {
        std::cerr << "Error: the '_' prefix for variable names is reserved\n";
        return 1;
      }
      if(std::find(variables.begin(), variables.end(), x[1].name) != variables.end()) {
        std::cerr << "Error: repeated variable name\n";
        return 1;
      }
      if(std::find(constants.begin(), constants.end(), x[1].name) != constants.end()) {
        std::cerr << "Error: name has already been used for a constant\n";
        return 1;
      }
      variables.push_back(x[1].name);
    }
    else if(x[0].type == Token::Const) {
      //line is a constant declaration
      if(x.size() != 3) {
        std::cerr << "Error: malformed constant declaration statement\n";
        return 1;
      }
      if(x[1].type != Token::Identifier || x[2].type != Token::String_Constant) {
        std::cerr << "Error: malformed constant declaration statement\n";
        return 1;
      }
      //x[1].name is the constant's name
      //x[2].name is the constant's value
      if(std::find(constants.begin(), constants.end(), x[1].name) != constants.end()) {
        std::cerr << "Error: repeated constant name\n";
        return 1;
      }
      if(std::find(variables.begin(), variables.end(), x[1].name) != variables.end()) {
        std::cerr << "Error: name has already been used for a variable\n";
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
                std::cerr << "Error: identifier '"<<y.name<<"' is not a valid var or const name\n";
                return 1;
              }
              else {
                //special variable located
              }
            }
          }
        }
        else if(y.type == Token::Transform) {
          //add name to the const table
          //
          if(searchMapValue<std::string, std::string>(constvals, y.name)) {
            //a constant with specified name already exists
          }
          else {
            //construct a new constant with specified name
            std::string name = "__cpth_cGenConst`" + std::to_string(constants.size());
            constants.push_back(name);
            constvals.emplace(name, y.name);
          }
        }
        else if(y.type == Token::Operation) {
          //add name to the const table
          if(searchMapValue<std::string, std::string>(constvals, y.name)) {
            //a constant with specified name already exists
          }
          else {
            //construct a new constant with specified name
            std::string name = "__cpth_cGenConst`" + std::to_string(constants.size());
            constants.push_back(name);
            constvals.emplace(name, y.name);
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
    p.type = ProtoInstruction::Nil;
    //check if x is a valid statement
    //check if first token is valid:
    if(x.size() == 0) {
      //ignore empty statement
      continue;
    }
    switch(x[0].type) {
    case Token::Var:
      // declarations aren't part of bytecode
      break;
    case Token::Const:
      // declarations aren't part of bytecode
      break;
    case Token::Load:
      if(x.size() < 4) {
      std::cerr << "Error: malformed load statement: Missing token(s)\n";
        return 1;
      }
      if(x[1].type != Token::Identifier || x[2].type != Token::From || x.size() & 1 == 1) {
      std::cerr << "Error: malformed load statement: Invalid token\n";
        return 1;
      }
      // if operations are used, split into several instructions, operations are implicit and use ephemeral variables
        //construct an additional proto-instructions for each operation

      for(unsigned i=x.size()-1;i>3;i-=2) {
        //instruction: type:oper, source1:x[i-2].name, source2:x[i].name, opname:x[i-1].name
        ProtoInstruction pt;

        if(x[i].type != Token::Identifier || x[i-1].type != Token::Operation || x[i-2].type != Token::Identifier) {
          std::cerr << "Error: malformed load statement: Erroneous operation syntax\n";
          return 1;
        }

        pt.type = ProtoInstruction::Operation;
        pt.source = x[i-2].name;
        pt.second_source = x[i].name;
        pt.value = x[i-1].name;
        //replace operation with a reference to implicit variable
        x[i-2].name = "_o";
        instructions.push_back(pt);
      }
      //now, construct the actual proto-instruction
      p.type = ProtoInstruction::Load;
      p.target = x[1].name;
      p.source = x[3].name;
      break;

    case Token::Apply:
      //more complicated
      if(x.size() != 4) {
        std::cerr << "Error: malformed apply statement: Invalid number of tokens\n";
        return 1;
      }
      if(x[1].type != Token::Transform || x[2].type != Token::To || x[3].type != Token::Identifier) {
        std::cerr << "Error: malformed apply statement: Misplaced token\n";
        return 1;
      }
      p.type = ProtoInstruction::Apply;
      p.value = x[1].name;
      p.args = x[1].args;
      p.target = x[3].name;

      break;
    case Token::Write:
      //more complicated
      if(x.size() < 4 || (x.size() & 1) == 1) {
        std::cerr << "Error: malformed write statement: Wrong number of tokens\n";
        return 1;
      }
      //parse first expr

      // if operations are used, split into several instructions, operations are implicit and use ephemeral variables
        //construct an additional proto-instructions for each operation

      for(unsigned i=x.size()-3;i>1;i-=2) {
        //create things
        //instruction: type:oper, source1:x[i-2].name, source2:x[i].name, opname:x[i-1].name
        ProtoInstruction pt;

        if(x[i].type != Token::Identifier || x[i-1].type != Token::Operation || x[i-2].type != Token::Identifier) {
          std::cerr << "Error: malformed load statement: Erroneous operation syntax\n";
          return 1;
        }

        pt.type = ProtoInstruction::Operation;
        pt.source = x[i-2].name;
        pt.second_source = x[i].name;
        pt.value = x[i-1].name;
        //replace operation with a reference to implicit variable to cascade changes
        x[i-2].name = "_o";
        instructions.push_back(pt);
      }
      //now, construct the actual proto-instruction
      // write and load trigger the same internal mechanism, just have different allowed
      // arguments

      p.type = ProtoInstruction::Load;
      p.target = x.back().name;
      p.source = x[1].name;
      break;
    default:
      std::cerr << "Error: statement begins with an invalid token\n";
      return 1;
    }
    if(p.type != ProtoInstruction::Nil) {
      instructions.push_back(p);
    }
  }
  return 0;
}

int binarify(const std::vector<ProtoInstruction>& instructions, std::vector<BinaryInstruction>& bvec) {
  uint16_t numberBase = 0x00;
  uint16_t constBase = 0x80;
  bvec.resize(instructions.size());
  for(unsigned i=0;i<instructions.size();i++) {
    bvec[i].opcode = static_cast<std::uint16_t>(instructions[i].type);
    switch(instructions[i].type) {
    case ProtoInstruction::Load:
      bvec[i].first = std::stoi(instructions[i].source);
      bvec[i].second = std::stoi(instructions[i].target);
      bvec[i].argc = 0;
      bvec[i].third = 0;
      break;
    case ProtoInstruction::Operation:
      bvec[i].first = std::stoi(instructions[i].source);
      bvec[i].second = std::stoi(instructions[i].second_source);
      bvec[i].third = std::stoi(instructions[i].value);
      bvec[i].argc = 0;
      break;
    case ProtoInstruction::Apply:
      bvec[i].first = std::stoi(instructions[i].target);
      bvec[i].second = std::stoi(instructions[i].value);
      bvec[i].third = 0;
      bvec[i].argc = instructions[i].args.size();
      bvec[i].args.reserve(bvec[i].argc);
      //add arguments here eventually
      for(auto x : instructions[i].args) {
        //convert argument to a number, with the correct header bits
        if(!x.empty()) {
          if(isNum(x)) {
            //it's a number
            //
            bvec[i].args.push_back(((numberBase & 0x3f) << 10) | (std::stoi(x) & 0x3ff));
            //std::clog << "Making transform args: '" << x << "'->'" << bvec[i].args.back() << "'\n";
          }
            else {
              std::cerr << "Error: not implemented yet\n";
            return 1;
          }
        }
      }
      break;
    }
  }
  return 0;
}


std::vector<std::uint8_t> BinaryInstruction::binary() {
  std::vector<std::uint8_t> t(5 + (opcode == 1) + args.size()*2 + 2*(opcode == 2));
  t[0] = opcode;
  t[1] = (first & 0xff00) >> 8;
  t[2] = first & 0xff;
  t[3] = (second & 0xff00) >> 8;
  t[4] = second & 0xff;
  if(opcode == 1) {
    t[5] = argc;
    for(unsigned i=0;i<args.size();i++) {
      t[2*i+6] = (args[i] & 0xff00) >> 8;
      t[2*i+7] = args[i] & 0xff;
    }
  }
  else if(opcode == 2) {
    t[5] = (third & 0xff00) >> 8;
    t[6] = third & 0xff;
  }

  return t;
}

int resolveReferences(std::vector<ProtoInstruction>& instructions, std::vector<std::string>& variables, std::vector<std::string>& constants, std::map<std::string, std::string>& constvals) {
  // if transforms are allowed to have bare string constant arguments eventually, this
  // function will need to search for them and add them to the const table
  // (or maybe this could happen earlier in the compilation process)

  

  // search for identifiers
  for(auto& x : instructions) {
    bool located{false};
    switch(x.type) {
    case ProtoInstruction::Load:
      //source, target have identifiers
      if(!updateReference(variables, x.source, VAR_OFFSET)) {
      //identifiers used as both var and const are filtered out by earlier stages
        if(!updateReference(constants, x.source, CONST_OFFSET)) {
          if(getSpecialVarNum(x.source) != -1) {
            x.source = std::to_string(SPECIAL_VAR_OFFSET + getSpecialVarNum(x.source));
          }
        }
      }

      if(!updateReference(variables, x.target, VAR_OFFSET)) {
      //identifiers used as both var and const are filtered out by earlier stages
        //it is illegal
        if(!updateReference(constants, x.target, CONST_OFFSET)) {
          if(getSpecialVarNum(x.target) != -1) {
            x.target = std::to_string(SPECIAL_VAR_OFFSET + getSpecialVarNum(x.target));
          }
        }
        else {
          std::clog << "Error: illegal write to constant\n";
          return 1;
        }
      }
      break;
    case ProtoInstruction::Apply:
      //target has identifier
      if(!updateReference(variables, x.target, VAR_OFFSET)) {
      //identifiers used as both var and const are filtered out by earlier stages
        if(!updateReference(constants, x.target, CONST_OFFSET)) {
          if(getSpecialVarNum(x.target) != -1) {
            x.target = std::to_string(SPECIAL_VAR_OFFSET + getSpecialVarNum(x.target));
          }
        }
      }
      // value should be in const table, find it
      for(auto y : constvals) {
        if(y.second == x.value) {
          for(unsigned i=0;i<constants.size();i++) {
            if(constants[i] == y.first) {
              x.value = std::to_string(CONST_OFFSET + i);
            }
          }
        }
      }
      break;
    case ProtoInstruction::Operation:
      //source and second_source have identifiers
      if(!updateReference(variables, x.source, VAR_OFFSET)) {
      //identifiers used as both var and const are filtered out by earlier stages
        if(!updateReference(constants, x.source, CONST_OFFSET)) {
          if(getSpecialVarNum(x.source) != -1) {
            x.source = std::to_string(SPECIAL_VAR_OFFSET + getSpecialVarNum(x.source));
          }
        }
      }
      if(!updateReference(variables, x.second_source, VAR_OFFSET)) {
      //identifiers used as both var and const are filtered out by earlier stages
        if(!updateReference(constants, x.second_source, CONST_OFFSET)) {
          if(getSpecialVarNum(x.second_source) != -1) {
            x.second_source = std::to_string(SPECIAL_VAR_OFFSET + getSpecialVarNum(x.second_source));
          }
        }
      }
      // value should be in const table, find it
      for(auto y : constvals) {
        if(y.second == x.value) {
          for(unsigned i=0;i<constants.size();i++) {
            if(constants[i] == y.first) {
              x.value = std::to_string(CONST_OFFSET + i);
            }
          }
        }
      }
      break;
    case ProtoInstruction::Nil:
      //invalid instruction, maybe error?
      std::cerr << "Error: invalid instruction\n";
      return 1;
      break;
    }
  }

  return 0;
}

bool updateReference(const std::vector<std::string>& identList, std::string& ident, int offset) {
  for(unsigned i=0;i<identList.size();i++) {
    if(identList[i] == ident) {
      ident = std::to_string(i + offset);
      return true;
    }
  }
  return false;
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

int createHeader(std::vector<std::uint8_t>& header, int argmin, int argmax, int numConsts, unsigned textLen, unsigned varNum) {
  header.resize(16); //allocate space at the beginning to avoid repeated resizings
  header[0x0] = 'C';
  header[0x1] = 'P';
  header[0x2] = 'T';
  header[0x3] = 'H';
  header[0x4] = static_cast<std::uint8_t>(MAJOR_VERSION);
  header[0x5] = static_cast<std::uint8_t>(MINOR_VERSION);
  header[0x6] = static_cast<std::uint8_t>(PATCH_NUM);
  header[0x7] = 0x0;
  header[0x8] = static_cast<std::uint8_t>(argmin);
  header[0x9] = static_cast<std::uint8_t>(argmax);
  header[0xa] = CONST_HEADER_WIDTH;
  header[0xb] = numConsts;
  header[0xc] = ((HEADER_LEN + textLen) & 0xff00) >> 8;
  header[0xd] = (HEADER_LEN + textLen) & 0xff;
  header[0xe] = varNum;
  header[0xf] = 0x0;

  return 0;
}


int generate_text(std::vector<std::uint8_t>& text, const std::vector<BinaryInstruction>& bvec) {
  //figure out how to reserve enough space?
  for(auto x : bvec) {
    auto p = x.binary();
    text.insert(text.end(), std::make_move_iterator(p.begin()), std::make_move_iterator(p.end()));
  }
  return 0;
}

int generate_const(std::vector<std::uint8_t>& table, const std::map<std::string, std::string>& value_hash, std::vector<std::string> constants) {
  // before generating the actual const table, search for possible optimizations:


  //first, generate const table head (contains the offsets for each const)
  int totalCharLen{0};
  for(auto x : value_hash) {
    totalCharLen += x.second.size()+1;
  }

  table.reserve( CONST_HEADER_WIDTH * value_hash.size() + totalCharLen +  value_hash.size());
  std::uint16_t runningPos{0};
  for(auto x : constants) {
    if(value_hash.find(x) == value_hash.end()) {
      std::cerr << "Error: undeclared constant\n";
    }
    auto y = (*value_hash.find(x)).second;
    if(y.size() > MAX_CONST_LEN) {
      std::cerr << "Error: Oversized string constant located\n";
      return 1;
    }
    table.push_back(((runningPos + CONST_HEADER_WIDTH * value_hash.size()) & 0xff00) >> 8);
    table.push_back((runningPos + CONST_HEADER_WIDTH * value_hash.size()) & 0xff);

    runningPos += y.size()+1;
  }

  //now, place the string constants in the table
  for(auto x : constants) {
    auto y = (*value_hash.find(x)).second;
    for(auto z : y) {
      table.push_back(z);
    }
    table.push_back(0x0);
  }
  return 0;
}


int assemble(std::vector<std::uint8_t>& method, const std::vector<std::uint8_t>& head, const std::vector<std::uint8_t>& text, const std::vector<std::uint8_t>& data) {
  //assemble all sections together, make any other necessary adjustments
  unsigned total = head.size() + text.size() + data.size();
  std::clog << std::hex << std::uppercase <<  "header : 0x"<< head.size()
	  << ";  text : 0x"<< text.size() << ";  data : 0x" << data.size() << ";  total : 0x"
	  << total << "\n" << std::dec << std::nouppercase;
  method.reserve(total);
  method.insert(method.end(), std::make_move_iterator(head.begin()), std::make_move_iterator(head.end()));
  method.insert(method.end(), std::make_move_iterator(text.begin()), std::make_move_iterator(text.end()));
  method.insert(method.end(), std::make_move_iterator(data.begin()), std::make_move_iterator(data.end()));
  return 0;
}


int getSpecialVarNum(const std::string& name) {
  std::vector<std::string> vars {
    "_", "_o", "_randU", "_randL", "_randE", "_randN", "_randA", "_argc", "_stdin",
    "_stdout", "_null"
  };
  for(unsigned i=0;i<vars.size();i++) {
    if(vars[i]==name){
      return i;
    }
  }
  return -1;
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
      std::cerr << "Error: file handles have not yet been implemented\n";
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
