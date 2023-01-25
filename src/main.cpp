#include <iostream>
#include <xcipher.hpp>
#include "transform.h"
#include "translate.h"

int main(int argc, char** argv) {
  //first, parse arguments
  //
  std::string infile;
  std::string outfile;
  std::string defineFile;
  std::string methodFile;
  std::string method;
  std::string args;
  bool define{false};
  bool useFile{false};
  bool help{false};
  std::string arg;
  for(int i=1;i<argc;i++) {
    arg = argv[i];
	if(arg.size() == 0)
      continue;
	if(arg[0] == '-') {
      if(arg.size() == 1) {
        std::cout << "Error: invalid argument\n";
        return 1;
	  }
	  switch(arg[1]) {
      case 'a':
		args = arg.substr(2);
		break;
      case 'i':
		infile = arg.substr(2);
		break;
      case 'o':
		outfile = arg.substr(2);
		break;
      case 'd':
		define = true;
		defineFile = arg.substr(2);
	  	break;
      case 'f':
		useFile = true;
		methodFile = arg.substr(2);
	    break;
	  case 'h':
		help = true;
		break;
      default:
        std::cout << "Error: invalid argument\n";
        return 1;

	  }
	}
	else if(method.empty()) {
      method = arg;
	}
	else {
	  std::cout << "BAD\n";
	  return 1;
	}
  }
  if(argc == 1) {
	std::cout << "Error: No method specified\n";
	return 1;
  }
  //determine which mode to enter in, check for invalid arguments
  // find inputs and outputs
  bool stdin{false};
  bool stdout{false};
  if(infile.empty()) {
    // read from stdin
  }
  if(outfile.empty()) {
    // write to stdout
  }
  if(help) {
	//display help message, then quit
	  std::cout << "cipherlang v0, made by xavenna.\nThis is a help menu\n";
	  std::cout << "It isn't very helpful yet\n";
	  return 0;
  }
  if(define) {
	//add specified file to dictionary if it is a valid file

	//tokenize file
	//convert it to a method file
	//place it in dictionary
  }
  else {
	if(useFile)
	  method = methodFile;
	//apply a method to specified input
	transformText(infile, outfile, method, args, useFile);
  }
  return 0;
}
