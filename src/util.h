#ifndef UTIL_H
#define UTIL_H
#include <ctime>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>


//! determines whether n is contained in key
bool ifMatch(char n, std::string key);
//! returns true if supplied string is a number
bool isNum(const std::string& st);
//! returns true if specified string is a boolean ("0" or "1")
bool isBool(const std::string& st);
bool getEntireFile(std::ifstream& in, std::string& out);
bool getBinFile(std::ifstream& in, std::vector<std::uint8_t> out);
size_t binFileSize(std::ifstream& in);
//! breaks n into a vector, starting a new element wherever a component of key is found
void parse(const std::string& n, std::vector<std::string>& th);
void genericParse(const std::string& n, std::vector<std::string>& th, const std::string key);
#endif
