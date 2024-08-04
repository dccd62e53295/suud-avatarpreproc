
cmake_minimum_required(VERSION 3.10)

project(suud
	VERSION 0.1
	DESCRIPTION "A suud Project")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

find_package(PkgConfig REQUIRED)
# https://stackoverflow.com/questions/45548088
# https://cmake.org/cmake/help/latest/module/FindOpenSSL.html
find_package(OpenSSL REQUIRED)
# https://stackoverflow.com/questions/67812541
pkg_check_modules(JSONCPP jsoncpp REQUIRED)
find_package(PNG 1.6 REQUIRED)
# https://www.boost.org/doc/libs/1_74_0/libs/gil/doc/html/io.html#supported-platforms
find_package(TIFF REQUIRED)
find_package(JPEG REQUIRED)
pkg_check_modules(RAW libraw REQUIRED)
find_package(ZLIB REQUIRED)
pkg_check_modules(WEBP libwebp REQUIRED)

# shared source
file(GLOB_RECURSE project_src_common
     "src/common/*.c"
     "src/common/*.cpp"
)

file(GLOB_RECURSE project_src_setavatar
     "src/setavatar/*.c"
     "src/setavatar/*.cpp"
)

file(GLOB_RECURSE project_src_imgci
     "src/imgci/*.c"
     "src/imgci/*.cpp"
)

# Create a simple configuration header
configure_file(config.h.in config.h)

# This project will output an executable file
add_executable(setavatar 
${project_src_common}
${project_src_setavatar}
)

add_executable(imgci 
${project_src_common}
${project_src_imgci}
)

# https://cmake.org/cmake/help/latest/command/target_compile_definitions.html
target_compile_definitions(setavatar PUBLIC COMPILEDEF_PATH_USERBRIDGE="../setavatar/userbridge.hpp")
target_compile_definitions(imgci PUBLIC COMPILEDEF_PATH_USERBRIDGE="../imgci/userbridge.hpp")

# Include the configuration header in the build
target_include_directories(setavatar PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(imgci PUBLIC "${PROJECT_BINARY_DIR}")

target_link_libraries(setavatar
PRIVATE OpenSSL::Crypto
${JSONCPP_LIBRARIES}
PNG::PNG
)

target_link_libraries(imgci
PRIVATE OpenSSL::Crypto
${JSONCPP_LIBRARIES}
PNG::PNG
TIFF::TIFF
JPEG::JPEG
${RAW_LIBRARIES}
ZLIB::ZLIB
${WEBP_LIBRARIES}
)
