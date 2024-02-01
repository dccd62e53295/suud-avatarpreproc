cmake_minimum_required(VERSION 3.10)

# Set some basic project attributes
project(uucc
	VERSION 0.1
	DESCRIPTION "A uucc Project")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

find_package(PkgConfig REQUIRED)

# https://stackoverflow.com/questions/45548088
# https://cmake.org/cmake/help/latest/module/FindOpenSSL.html
find_package(OpenSSL REQUIRED)
# https://stackoverflow.com/questions/67812541
pkg_check_modules(JSONCPP jsoncpp)
find_package(PNG 1.6 REQUIRED)

file(GLOB_RECURSE project_src_common
     "src/common/*.c"
     "src/common/*.cpp"
)

file(GLOB_RECURSE project_src_setavatar
     "src/setavatar/*.c"
     "src/setavatar/*.cpp"
)

# Create a simple configuration header
configure_file(config.h.in config.h)

# This project will output an executable file
add_executable(uucc_setavatar 
${project_src_common}
${project_src_setavatar}
)

# https://cmake.org/cmake/help/latest/command/target_compile_definitions.html
target_compile_definitions(uucc_setavatar PUBLIC COMPILEDEF_PATH_USERBRIDGE="../setavatar/userbridge.hpp")

# Include the configuration header in the build
target_include_directories(uucc_setavatar PUBLIC "${PROJECT_BINARY_DIR}")

target_link_libraries(uucc_setavatar
PRIVATE OpenSSL::Crypto
${JSONCPP_LIBRARIES}
PNG::PNG
)
