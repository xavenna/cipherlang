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
	return 0;
}

int transform_trim_special(std::string& substrate) {
	for(auto x : substrate) {
		if((x >= '!' && x <= '/') || (x >= ':' && x <= '@') || (x >= '[' && x <= '`') || (x >= '{' && x <= '~')) {
			substrate.erase(x);
			x--;
		}
	}
	return 0;
}

int transform_trim_numeric(std::string& substrate) {
	for(auto x : substrate) {
		if(x >= '0' && x <= '9') {
			substrate.erase(x);
			x--;
		}
	}
	return 0;
}

int transform_trim_whitespace(std::string& substrate) {
	for(auto x : substrate) {
		if(x == ' ' || x == '\t') { // I know there are more whitespace chars...
			substrate.erase(x);
			x--;
		}
	}
	return 0;
}

int transform_trim_alpha(std::string& substrate) {
	for(auto x : substrate) {
		if((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z')) {
			substrate.erase(x);
			x--;
		}
	}
	return 0;
}

int transform_prune(std::string& substrate) {
	//remove num, special, whitespace
	transform_trim_numeric(substrate);
	transform_trim_whitespace(substrate);
	transform_trim_special(substrate);
	return 0;
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
//cipher transforms (using xcipher)

int caesarian_shift(std::string& substrate, std::int16_t arg) {
	//attempt to parse arg as an integer
	substrate = xc::cs_encode(substrate, arg);
	return 0;
}

int rail_cipher(std::string& substrate, std::int16_t arg) {
	substrate = xc::rc_encode(substrate, arg);
	return 0;
}

int inverse_rail_cipher(std::string& substrate, std::int16_t arg) {
	substrate = xc::rc_decode(substrate, arg);
	return 0;
}

