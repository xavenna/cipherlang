#include "interpret.h"

int interpret(std::vector<std::uint8_t>& method, const std::string& text, std::string& out) {
  //extract consts from header
  std::size_t len{method.size()};
  if(len < 16) {
	std::cerr << "Error: invalid file\n";
	return 1;
  }
  if(method[0] != 'C' || method[1] != 'P' || method[2] != 'T' || method[3] != 'H' || method[7] != 0x0) {
	std::cerr << "Error: invalid magic\n";
	return 1;
  }
  std::size_t const_offset = (method[0xc] << 8) | (method[0xd]);
  std::size_t argmin = method[0x8];
  std::size_t argmax = method[0x9];
  std::size_t const_wid = method[0xa];
  std::size_t num_consts = method[0xb];
  std::size_t num_vars = method[0xe];

  std::deque<std::string> variables(num_vars);
  // TODO: encode num variables into header to prevent excessive var allocation

  //I'm using deque rather than vector to prevent excessive reallocation

  //add some kind of version number checking eventually

  std::vector<std::uint8_t> inst(6); //without args, all use less than 6
  std::vector<std::uint8_t>::iterator counter = method.begin()+0x10;

  std::string last_op;
  std::string last_tr;
	//add bounds checking

  std::size_t in_ptr{0};

  while(true) {
	//load instruction
	std::size_t ilen{5};
	switch(*counter) {
	case 0:
	  break;
	case 2:
	  ilen+=2;
	  break;
	case 1:
	  ilen += 1 + 2 * (*(counter+5));
	  break;
	default:
	  std::cerr << "Error: unrecognized operation '" << +(*counter) << "'\n";
	  return 1;
	}

	inst.resize(ilen);
	std::copy(counter, counter+ilen, inst.begin());

	//now, execute the instruction
	std::uint16_t first = (*(counter + 1) << 8) | *(counter + 2);
	std::uint16_t second = (*(counter + 3) << 8) | *(counter + 4);
	std::string tempVar; //used to simplify the move code
	std::string secondVar; //used for operations
	std::string type; //used for operations and transforms
	std::vector<std::string> args; //for transform
	switch(inst[0]) {
	case 0:
	  //load
	  //determine source:
	  if(first < 0x100) {
		//normal variable
		if(variables.size() > first) {
		  tempVar = variables[first];
		}
		else {
		  std::cerr << "Error: out of bounds variable read\n";
		  return 1;
		}
	  }
	  else if(first < 0x300) {
		//const
		if(first - 0x100 >= num_consts) {
		  //error
			std::cerr << "Error: out of bounds const read\n";
			return 1;
		}
		std::size_t t{getConstOffset(method, const_offset, first - 0x100)};
		//copy from target_offset until a null character into tempVar
		while(method[t] != 0) {
		  tempVar += method[t];
		  t++;
		}

	  }
	  else if(first < 0x400) {
		//special variable
		switch(first - 0x300) {
		case 0:
		  //last tr
		  tempVar = last_tr;
		  break;
		case 1:
		  //last operation
		  tempVar = last_op;
		  break;
		case 8:
		  //stdin
		  //get from stdin
		  if(in_ptr < text.size()) { //avoid invalid reads
		    tempVar = text.substr(in_ptr, text.find('\n', in_ptr)-in_ptr);
		    in_ptr += tempVar.size()+1;
		  }
		  break;
		case 9:
		  //stdout
		  std::cerr << "Error: cannot read from stdout\n";
		  return 1;
		  break;
		case 0xa:
		  //null
		  tempVar.clear();
		  break;
		default:
		  std::cerr << "Not implemented yet\n";
		  break;
		}
	  }
	  else {
		std::cerr << "Error: invalid source: unallocated location\n";
		return 1;
	  }
	  

	  if(second < 0x100) {
		//normal variable
		if(variables.size() > second) {
		  variables[second] = tempVar;
		}
		else {
		  std::cerr << "Error: out of bounds variable write\n";
		  return 1;
		}
	  }
	  else if(second < 0x300) {
		//const
		std::cerr << "Error: const writes prohibited\n";
	  }
	  else if(second < 0x400) {
		//special variable
		switch(second - 0x300) {
		case 9:
		  //stdout
		  out += tempVar + (tempVar.empty() ? '\0' : '\n'); // to avoid excessive newlines
		  break;
		default:
		  std::cerr << "Error: illegal write to special var\n";
		  return 1;
		  break;
		}
		//return 1;
	  }
	  else {
		std::cerr << "Error: invalid destination: unallocated location\n";
		return 1;
	  }

	  break;
	case 1:
	  //apply transform
	  //load tempVar with location of name
	  if(first < 0x100) {
		//variable
		if(variables.size() > first) {
		  tempVar = variables[first];
		}
		else {
		  std::cerr << "Error: out of bounds variable access\n";
		  return 1;
		}
	  }
	  else if(first < 0x300) {
		//const
		std::cerr << "Error: cannot use a const as a substrate for transform\n";
		return 1;
	  }
	  else if(first < 0x400) {
		//special var
	    std::cerr << "Error: cannot use special var as substrate for transform\n";
		return 1;
	  }
	  //now, load transform name into type



	  if(second < 0x100) {
		//variable
		std::cerr << "Error: cannot use variable as transform name\n";
		return 1;
	  }
	  else if(second < 0x300) {
		//const
		if(second - 0x100 >= num_consts) {
		  //error
			std::cerr << "Error: out of bounds const read\n";
			return 1;
		}
		std::size_t t{getConstOffset(method, const_offset, second - 0x100)};
		//copy from target_offset until a null character into tempVar
		while(method[t] != 0) {
		  type += method[t];
		  t++;
		}
	  }
	  else if(second < 0x400) {
		//special var
		std::cerr << "Error: cannot use special var as transform name\n";
		return 1;
	  }
	  //now, figure out the args;
	  {
		unsigned argc = *(counter + 5);
		args.clear();
		args.reserve(argc);
		for(std::size_t i=0;i<argc;i++) {
		  std::uint16_t ref{static_cast<std::uint16_t>((*(counter + 6 + i) << 8) | (*(counter + 7 + i)))};
		  // determine if ref refers to an int or a string ref
		  std::uint16_t type = (ref & 0xfc00) >> 10;
		  std::uint16_t value = ref & 0x3ff;
		  if(type == 0x80) {
			std::cerr << "Error: const arguments not yet supported\n";
			return 1;
		  }
		  else if(type == 0x00) {
			args.push_back(std::to_string(value));
		  }
		  else {
			std::cerr << "Error: invalid argument type\n";
			return 1;
		  }
		}
		if(applyTransform(tempVar, type, args) != 0) {
		  std::cerr << "Error: transform failure\n";
		  return 1;
		}
		last_tr = tempVar;
		//now, write tempVar back to its source
		//for it to get to this point, first has to be less than 0x100

	    if(first < 0x100) {
	      //variable
		  if(variables.size() > first) {
			variables[first] = tempVar;
		  }
		  else {
		    std::cerr << "Error: out of bounds variable access\n";
		    return 1;
	      }
	    }
	  }
	  break;
	case 2:
	  //operation
	  if(first < 0x100) {
		//variable
		tempVar = variables[first];
	  }
	  else if(first < 0x300) {
		//const
		if(first - 0x100 >= num_consts) {
		  //error
			std::cerr << "Error: out of bounds const read\n";
			return 1;
		}
		std::size_t t{getConstOffset(method, const_offset, first - 0x100)};
		//copy from target_offset until a null character into tempVar
		while(method[t] != 0) {
		  tempVar += method[t];
		  t++;
		}
	  }
	  else if(first < 0x400) {
		//special var
		switch(first - 0x300) {
		case 0:
		  //last tr
		  tempVar = last_tr;
		  break;
		case 1:
		  //last operation
		  tempVar = last_op;
		  break;
		case 8:
		  //stdin
		  //get from stdin
		  if(in_ptr < text.size()) { //avoid invalid reads
		    tempVar = text.substr(in_ptr, text.find('\n', in_ptr)-in_ptr);
		    in_ptr += tempVar.size()+1;
		  }
		  break;
		case 9:
		  //stdout
		  std::cerr << "Error: cannot read from stdout\n";
		  return 1;
		  break;
		case 0xa:
		  //null
		  tempVar.clear();
		  break;
		default:
		  std::cerr << "Not implemented yet\n";
		  break;
		}
	  }
	  else {
		std::cerr << "Error: read from unallocated source\n";
		return 1;
	  }

	  if(second < 0x100) {
		//variable
		secondVar = variables[second];
	  }
	  else if(second < 0x300) {
		//const
		if(second - 0x100 >= num_consts) {
		  //error
			std::cerr << "Error: out of bounds const read\n";
			return 1;
		}
		std::size_t t{getConstOffset(method, const_offset, second - 0x100)};
		//copy from target_offset until a null character into secondVar
		while(method[t] != 0) {
		  secondVar += method[t];
		  t++;
		}
	  }
	  else if(second < 0x400) {
		//special var
		switch(second - 0x300) {
		case 0:
		  //last tr
		  secondVar = last_tr;
		  break;
		case 1:
		  //last operation
		  secondVar = last_op;
		  break;
		case 8:
		  //stdin
		  //get from stdin
		  if(in_ptr < text.size()) { //avoid invalid reads
		    secondVar = text.substr(in_ptr, text.find('\n', in_ptr)-in_ptr);
		    in_ptr += secondVar.size()+1;
		  }
		  break;
		case 9:
		  //stdout
		  std::cerr << "Error: cannot read from stdout\n";
		  return 1;
		  break;
		case 0xa:
		  //null
		  secondVar.clear();
		  break;
		default:
		  std::cerr << "Not implemented yet\n";
		  break;
		}
	  }
	  else {
		std::cerr << "Error: read from unallocated source\n";
		return 1;
	  }

	  //now, apply operation on tempVar and secondVar
	  {
	    std::uint16_t third = ((*(counter + 5) << 8) | *(counter + 6));
	    if(third < 0x100) {
		  //operation name is a variable
		  std::cerr << "Error: use of variable as operation name is not currently supported\n";
		  return 1;
	    }
		else if(third < 0x300) {
		  //operation name is in a constant
		  if(third - 0x100 >= num_consts) {
		    //error
			std::cerr << "Error: out of bounds const read\n";
			return 1;
		  }
		  std::size_t t{getConstOffset(method, const_offset, third - 0x100)};
		  //copy from target_offset until a null character into secondVar
		  while(method[t] != 0) {
		    type += method[t];
		    t++;
		  }
		}
		else if(third < 0x400) {
		  //operation name is in a special variable
			std::cerr << "Error: use of special variable as operation name is not currenty supported\n";
			return 1;
		}
		if(applyOperation(last_op, tempVar, secondVar, type) != 0) {
		  std::cerr << "Operation Error\n";
		  return 1;
		}
	  }
	  break;
	default:
	  std::cerr << "Error: unrecognized instruction\n";
	  break;
	}
	
	std::advance(counter, ilen);
	if(counter-method.begin() >= const_offset) {
	  std::cerr << "Interpretation successful\n";
	  return 0;
	}
  }

  return 0;
}

int applyOperation(std::string& result, const std::string& tempVar, const std::string& secondVar, const std::string& type) {
  if(type == "cat") {
	result = tempVar + secondVar;
  }
  else {
	std::cerr << "Error: invalid operation '" << type << "'\n";
	return 1;
  }
  return 0;
}

int applyTransform(std::string& input, const std::string& type, const std::vector<std::string>& args) {
  //std::cerr << "calling '" << type << "'\n";
  //if type is a built-in transform, execute it
  if(type == "upper") {  // no args
	if(transform_upper(input) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "lower") {  // no args
	if(transform_lower(input) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "trim_special") {  // no args
	if(transform_trim_special(input) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "trim_numeric") {  // no args
	if(transform_trim_numeric(input) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "trim_whitespace") {  // no args
	if(transform_trim_whitespace(input) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "trim_alpha") {  // no args
	if(transform_trim_alpha(input) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "prune") {  // no args
	if(transform_prune(input) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "prune_numeric") { // no args
	if(transform_prune_numeric(input) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "shift") {  // int
	if(args.size() < 1) {
	  return 1;
	}
	std::int16_t arg = std::stoi(args[0]);
	if(caesarian_shift(input, arg) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "rc") { // int
	if(args.size() < 1) {
	  return 1;
	}
	std::int16_t arg = std::stoi(args[0]);
	if(rail_cipher(input, arg) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else if(type == "rc_dec") { // int
	if(args.size() < 1) {
	  return 1;
	}
	std::int16_t arg = std::stoi(args[0]);
	if(inverse_rail_cipher(input, arg) != 0) {
	  std::cerr << "Error: failed transform\n";
	  return 1;
	}
  }
  else {
	// check if method file with matching name exists in current dir
	// check if method file with matching name exists in global script dir
    std::cerr << "External transforms haven't been implemented yet\n";
  }
  return 0;
}
std::uint16_t getConstOffset(const std::vector<std::uint8_t>& method, std::uint16_t off, std::uint16_t num) {
  return ( off + ( (method[off + (CONST_HEADER_WIDTH*num)] << 8) | 
  (method[off + (CONST_HEADER_WIDTH*num) + 1]) ));
}
