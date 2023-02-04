#include "interpret.h"

int interpret(std::vector<std::uint8_t>& method, const std::string& text, std::string& out) {
  //extract consts from header
  std::size_t len{method.size()};
  if(len < 16) {
	std::cout << "Error: invalid file\n";
  }
  if(method[0] != 'C' || method[1] != 'P' || method[2] != 'T' || method[3] != 'H' || method[7] != 0x0) {
	std::cout << "Error: invalid magic\n";
  }
  std::size_t const_offset = (method[0xc] << 8) | (method[0xd]);
  std::size_t argmin = method[0x8];
  std::size_t argmax = method[0x9];
  std::size_t const_wid = method[0xa];
  std::size_t num_consts = method[0xb];

  std::deque<std::string> variables(256); // TODO: encode num variables into header
  //I'm using deque rather than vector to prevent excessive reallocation

  //add some kind of version number checking eventually

  std::vector<std::uint8_t> inst(6); //without args, all use less than 6
  std::vector<std::uint8_t>::iterator counter = method.begin()+0x10;
  std::deque<std::string> special_vars(NUM_SPECIAL_VARS);

	//add bounds checking

  while(true) {
	//load instruction
	std::size_t ilen{5};
	switch(*counter) {
	case 0:
	case 2:
	  break;
	case 1:
	  ilen += 1 + 2 * (*(counter+5));
	  break;
	default:
	  std::cout << "Error: unrecognized operation '" << +(*counter) << "'\n";
	  return 1;
	}

	inst.resize(ilen);
	std::copy(counter, counter+ilen, inst.begin());

	//std::cout << "Instruction: ";
	//std::for_each(inst.begin(), inst.end(), [](std::uint8_t t){std::cout << +t<<',';});
	//std::cout << '\n';
	//now, execute the instruction
	std::uint16_t first = (*(counter + 1) << 8) | *(counter + 2);
	std::uint16_t second = (*(counter + 3) << 8) | *(counter + 4);
	std::string tempVar; //used to simplify the move code
	switch(inst[0]) {
	case 0:
	  //load
	  //determine source:
	  if(first < 256) {
		//normal variable
		if(variables.size() > first) {
		  tempVar = variables[first];
		}
		else {
		  std::cout << "Error: out of bounds variable read\n";
		  return 1;
		}
	  }
	  else if(first < 768) {
		//const
		if(first - 256 >= num_consts) {
		  //error
			std::cout << "Error: out of bounds const read\n";
		}
		std::size_t t{ const_offset + method[const_offset + first - 256]};
		//copy from target_offset until a null character into tempVar
		while(method[t] != 0) {
		  tempVar += method[t];
		  t++;
		}

	  }
	  else if(first < 1024) {
		//special variable
		std::cout << "Not implemented yet\n";
	  }
	  else {
		std::cout << "Error: invalid variable token: unallocated location\n";
		return 1;
	  }
	  std::cout << "'" << tempVar << "'\n";
	  

	  if(second < 256) {
		//normal variable
		if(variables.size() > second) {
		  variables[second] = tempVar;
		}
		else {
		  std::cout << "Error: out of bounds variable write\n";
		  return 1;
		}
	  }
	  else if(second < 768) {
		//const
		std::cout << "Error: const writes prohibited\n";
	  }
	  else if(second < 1024) {
		//special variable
		std::cout << "Not implemented yet\n";
	  }
	  else {
		std::cout << "Error: invalid variable token: unallocated location\n";
		return 1;
	  }

	  break;
	case 1:
	  //apply transform
	  break;
	case 2:
	  //operation
	  break;
	default:
	  std::cout << "Error: unrecognized operation\n";
	  break;
	}

	std::advance(counter, ilen);
	if(counter-method.begin() >= const_offset) {
	  std::cout << "Interpretation successful\n";
	  return 0;
	}
  }

  return 0;
}
