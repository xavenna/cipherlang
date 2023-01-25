#include "transform.h"

//basic utilities

int transform_upper(std::string& substrate) {
	//turns all lowercase letters to uppercase letters
	for(auto& x : substrate) {
		if(x > 96 && x < 123)
			x-=32;
	}
	return 0;
}

int transform_lower(std::string& substrate) {
	for(auto& x : substrate) {
		if(x > 65 && x < 91)
			x+=32;
	}
	return 1;
}

int transform_trim_special(std::string& substrate) {
	return 1;
}

int transform_trim_numeric(std::string& substrate) {
	for(auto x : substrate) {
		if(x >= '0' && x <= '9') {
			substrate.erase(x);
			x--;
		}
	}
	return 1;
}

int transform_trim_whitespace(std::string& substrate) {
	return 1;
}

int transform_trim_alpha(std::string& substrate) {
	return 1;
}

int transform_prune(std::string& substrate) {
	return 1;
}

int transform_prune_numeric(std::string& substrate) {
	for(auto x : substrate) {
		if(!(x >= '0' && x <= '9')) {
			substrate.erase(x);
			x--;
		}
	}
	return 0;
}
//cipher transforms

int caesarian_shift(std::string& substrate, const std::string& arg) {
	//attempt to parse arg as an integer
	if(!isNum(arg)) {
		return -1;
	}
	int distance = std::stoi(arg);
	substrate = cs_encode(substrate, distance);
	return 0;
}

