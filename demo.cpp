#include "executor.h"

int main(int argc, char** argv) {

    std::string command = "ls -a";   // Command to run
    command = RunScript(command);
    std::cout << command << std::endl;
    
    return 0;

}