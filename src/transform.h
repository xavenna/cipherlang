#ifndef TRANSFORM_H
#define TRANSFORM_H
#include <string>
#include <map>
#include <xcipher.hpp>
#include "util.h"

int transform_upper(std::string& substrate);
int transform_lower(std::string& substrate);
int transform_trim_special(std::string& substrate);
int transform_trim_numeric(std::string& substrate);
int transform_trim_whitespace(std::string& substrate);
int transform_trim_alpha(std::string& substrate);
int transform_prune(std::string& substrate);
int transform_prune_numeric(std::string& substrate);

int caesarian_shift(std::string& substrate, const std::string& arg);

#endif
