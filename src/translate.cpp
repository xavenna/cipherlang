#include <cstring>
#include "translate.h"
#include "token.h"
#include "util.h"
#include "interpret.h"




int transformText(const std::string& infile, const std::string& outfile, const std::string& methodName, const std::string& args, bool localMethod) {
  //load method file

  //execute method file using specified file as substrate
  
  std::ifstream loader;
  std::string substrate;
  std::vector<std::uint8_t> method;
  if(localMethod) {
    if(methodName.empty()) {
      std::cout << "Error: null local filename - invalid call\n";
	  return -1;
	}
    loader.open(methodName);
	if(!loader.is_open()) {
      std::cout << "Error: Local file '" << methodName << "' could not be located\n";
	  return -1;
	}
	std::string rawMethod;
	getEntireFile(loader, rawMethod);
	loader.close();
	//convert the contents of rawMethod to a method file (tokenize, transform)
	convertToMethod(method, rawMethod);
	//for debugging, write to a file
	std::ofstream write("bin.cpth", std::ios::binary);
	write.write(reinterpret_cast<const char*>(method.data()), method.size());
  }
  else {
    //search ~/.ciplang/.methods for requested method
    std::string line;
    loader.open("~/.ciplang/.methods");
	//search for 'method' in loader
	bool found{false};
	while(loader.peek() != EOF) {
      std::getline(loader, line);
      if(line == methodName) {
        //valid method, we can load it in
		found = true;
	  }
	}
	if(!found) {
      std::cout << "Error: requested method '"<<methodName<<"' could not be located.\n"
	            << "Did you forget -f?\n";
	  return -1;
	}
	//load contents of requested file
	loader.close();
	loader.open("~/.ciplang/methods/"+methodName, std::ios::binary);
	getBinFile(loader, method);
	loader.close();
  }
  //now that the method has been determined, get the input text, transform it, and 
  //write it to the appropriate place

  //call the interpreter using procured method

  //determine text

  std::string input;
  std::string output;
  if(infile.empty()) {
	//std::cout << "Error: stdin hasn't been implemented yet\n";
    std::istreambuf_iterator<char> begin(std::cin), end;
    std::string s(begin, end);
	input = s;
  }
  else {
	std::ifstream load(infile);
	if(!load.is_open()) {
	  std::cout << "Error: input file could not be opened\n";
	}
	getEntireFile(load, input);
  }


  interpret(method, input, output);

  if(outfile.empty()) {
	std::cout << output;
  }
  else {
	std::cout << "yim\n";
  }
  return 0;
}



int convertToMethod(std::vector<std::uint8_t>& output, const std::string& input) {
	//first, tokenize the method
	std::vector<Token> tokens;
	std::vector<std::string> lines;
	genericParse(input, lines, "\n");
	tokenizeString(lines, tokens);
	/*
	for(auto x : tokens) {
	  std::cout << tokenInfo(x) << '\n';
	}
	*/

	//now, compile to bytecode
	compileMethod(tokens, output);

	return 0;
}
