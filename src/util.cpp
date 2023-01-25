#include "util.h"
#include <limits>

// util: various utility functions used by other parts of the code. None of these are stellar-enigma specific


bool isNum(const std::string& st) {
  for(unsigned i=0;i<st.size();i++) {
    if(!isdigit(st[i]) && (st[i] != '-' || i != 0))
      return false;
  }
  return true;
}
bool isBool(const std::string& st) {
  return (st == "0" || st == "1");
}

/*
//returns whether n is found in key
bool ifMatch(char n, std::string key) {
	return (key.find(n) != std::string::npos);
}
*/

bool getEntireFile(std::ifstream& in, std::string& out) {
	if(!in.is_open())
		return false;
    std::ostringstream sstr;
    sstr << in.rdbuf();
	out = sstr.str();
	return true;
}

bool getBinFile(std::ifstream& in, std::vector<std::uint8_t> out) {
	if(!in.is_open())
		return false;
	auto size = binFileSize(in);
	out.resize(size);
	in.read(reinterpret_cast<char*>(out.data()), size);
	return true;
}

size_t binFileSize(std::ifstream& in) {
	if(!in.is_open())
		return false;
	in.ignore( std::numeric_limits<std::streamsize>::max() );
	std::streamsize length = in.gcount();
	in.clear();   //  Since ignore will have set eof.
	in.seekg( 0, std::ios_base::beg );
	return length;
}


//splits a string into a vector of strings
void parse(const std::string& n, std::vector<std::string>& th) {

  bool inString = false;
  std::string cw;
  for(auto x : n) {
    switch(x) {
    case ' ':
      if(inString) {
      	cw += x;
	  } else {
		if(!cw.empty()) {
          th.push_back(cw);
		  cw.clear();
		}
	  }
      break;
    case '"':
	  if(inString) {
		inString = false;
	  	cw += '"';
        th.push_back(cw);
		cw.clear();
	  } else {
	    if(cw.empty()) {
      	  inString = true;
	  	cw += '"';
		}
	  }
      break;
    default:
      cw += x;
    }
  }
  th.push_back(cw);
}

void genericParse(const std::string& n, std::vector<std::string>& th, const std::string key) {
  std::string cw;
  for(auto x : n) {
	if(ifMatch(x, key)) {
	  if(!cw.empty()) {
        th.push_back(cw);
	    cw.clear();
	  }
	} else {
      cw += x;
    }
  }
  th.push_back(cw);
}
