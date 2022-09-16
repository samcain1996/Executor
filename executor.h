#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>

/* Defines that are agnostic to OS */

// Pipe IO
#define READ 0
#define WRITE 1
#define Args void*
#define BUFSIZE 4096

/* Defines that are specific to Unix-like OSes */

#if defined(__APPLE__) || defined(__linux__)
    #include <unistd.h>                           // Used for launching processes
    #include <sys/wait.h>                         
    #define PID pid_t                             // Type to uniquely id processes
    #define ARG_START_IDX 0                       // Index to consider the "first" argument
    #define DEDUCE_TYPE reinterpret_cast<char**>  // Casts Args into string array
    
/* Defines that are specific to Windows */

#elif defined(_WIN32)
    #include <Windows.h>    // Used for launching processes and using pipes
    #define PID void*       // Type to uniquely id processes

    // Default path to powershell which will be used to run system commands
    #define PSPATH "C:\\Windows\\System32\\WindowsPowerShell\\V1.0\\powershell.exe"
    #define CMDPATH "C:\\Windows\\System32\\cmd.exe"
    #define ARG_START_IDX 1  // Index to consider the "first" argument

    // Casts Args to string
    #define DEDUCE_TYPE reinterpret_cast<char*>
    
    #if defined(_MSC_VER)
        #pragma warning(disable: 4996)
    #endif
#endif 



std::string RunScript(const std::string& script);