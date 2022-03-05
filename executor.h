#include <iostream>
#include <string>
#include <type_traits>
#define READ 0
#define WRITE 1
#define ERROR 2
#define Args void*
#define DEDUCE_TYPE reinterpret_cast<char**>

#if defined(__APPLE__) || defined(__linux__)
    #include <unistd.h>
    #define PID pid_t
#elif defined(_WIN32)
    #include <Windows.h>
    #define PID void*
    #if defined(_MSC_VER)
        #pragma warning(disable: 4996)
        #define DEDUCE_TYPE reinterpret_cast<char*>
    #endif
#endif  
