#include <iostream>
#include <string>
#include <type_traits>
#define READ 0
#define WRITE 1
#define Args void*
#define DEDUCE_TYPE reinterpret_cast<char**>

#if defined(__APPLE__) || defined(__linux__)
    #include <unistd.h>
    #define PID pid_t
    #define ARG_START_IDX 0
#elif defined(_WIN32)
    #include <Windows.h>
    #define PID void*
    #define PSPATH "C:\\Windows\\System32\\WindowsPowerShell\\V1.0\\powershell.exe"
    #define ARG_START_IDX 1
    #if defined(_MSC_VER)
        #pragma warning(disable: 4996)
        #undef DEDUCE_TYPE 
        #define DEDUCE_TYPE reinterpret_cast<char*>
    #endif
#endif  
