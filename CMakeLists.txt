cmake_minimum_required(VERSION 3.22)
project(cipherlang)


set(cipherlang_src
	src/token.cpp
	src/transform.cpp
	src/util.cpp
	src/main.cpp
	src/translate.cpp
	src/compile.cpp
)

#find_library(XCIPHER_LIB xcipher)

add_compile_definitions(MAJOR_VERSION=0 MINOR_VERSION=0 PATCH_NUM=0)

add_library(xcipher STATIC IMPORTED)
set_target_properties(xcipher PROPERTIES IMPORTED_LOCATION /usr/lib/libxcipher.a)
#set_property(TARGET foo PROPERTY IMPORTED_LOCATION /usr/lib/libfoo.a)


set (CMAKE_CXX_STANDARD 11)

add_executable(cipherlang ${cipherlang_src})
target_link_libraries(cipherlang xcipher)
install(TARGETS cipherlang DESTINATION bin)
