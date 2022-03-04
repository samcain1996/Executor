#include <iostream>
#include <string>
#include <bit>
#include <type_traits>
#define READ 0
#define WRITE 1
#define ERROR 2
#define Args void*
#define reinterpret reinterpret_cast

#if defined(__APPLE__) || defined(__linux__)
    #include <unistd.h>
    #define PID pid_t
#elif defined(_WIN32)
    #include <Windows.h>
    #define PID void*
    #if defined(_MSC_VER)
        #define reinterpret bit_cast
    #endif
#endif  
