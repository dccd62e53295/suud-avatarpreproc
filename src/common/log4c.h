#pragma once

# ifdef __cplusplus
extern "C" {
# endif

#define log4c_log(...) log4c_raw(__FILE__, __LINE__, __VA_ARGS__)

void log4c_raw(const char*,int,const char*,...) __attribute__((format(printf,3,4)));

# ifdef __cplusplus
}
# endif
