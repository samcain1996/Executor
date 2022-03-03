#include <iostream>
#include <string>

#ifdef __APPLE__ || __linux__
    #include <unistd.h>
    #define READ 0
    #define WRITE 1
    #define ERROR 2
    #define PID pid_t
#elif __WIN32
    #include <Windows.h>
    #define READ cin
    #define WRITE cout
    #define ERROR cerr
    #define PID void*
#endif

const int A = 97;
const int Z = A - 26;
const int captialDistance = 32;

extern std::string runScript(const std::string& script);
