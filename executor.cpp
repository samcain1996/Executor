#include "executor.h"

using namespace std;

/**
 * @brief Counts the number of words in a string
 * 
 * @param words     string to count words of
 * @return int      -1 if words is empty, else number of words in string
 */
int countWords(const char* words) {
    int wordCount = -1;
    const char* prevChar = nullptr;

    for (size_t index = 0; index < strlen(words); index++) {
        if (index == 0) { 
            prevChar = &words[0];
            wordCount = 1; 
            continue; 
        }

        if (*prevChar == ' ') { wordCount++; }
        prevChar = &words[index];
    }

    return wordCount;
}

/**
 * @brief Create pipe from specified endpoints
 * 
 * @param endPoints     Read and write ends to pipe
 * @return              True on successful pipe creation,
 *                      otherwise false.
 */
bool createPipe(PID endPoints[2]) {

    #ifdef _WIN32

    SECURITY_ATTRIBUTES secAttr;
    secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    secAttr.bInheritHandle = TRUE;
    secAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&endPoints[READ], &endPoints[WRITE], &secAttr, 0)) { 
        return false; 
    }

    #elif __APPLE__ || __linux__
        if(pipe(endPoints) == -1) { return false; }
    #endif

    return true;
}

/**
 * @brief Converts command and command-line arguments to required format
 * 
 * @param prcName   command name
 * @param args      command-line arguments
 * @return bool     True if src is not empty, otherwise false
 */
bool processArgs(const string& src, char*& prcname, char**& args) {
    // Nothing to split
    if (src.empty()) { return false; }

    int numOfArgs = countWords(src.c_str());   // Number of words in str

    #ifdef __APPLE__ || __linux__
        numOfArgs++;
    #endif


    args = new char*[numOfArgs];

    // Loop through each word in str
    for (size_t argIdx = 0, curIdx = 0; 
          argIdx < numOfArgs && curIdx < src.size(); argIdx++) {
        
        // Local copy of word
        const char* arg = src.substr(curIdx, src.substr(curIdx).find(' ')).c_str();
        
        // Copy word to word array
        args[argIdx] = new char[strlen(arg) + 1];
        strcpy(args[argIdx], arg);

        // Move starting index
        curIdx += src.substr(curIdx).find(' ') + 1;
    }

    #ifdef __APPLE__ || __linux__

    prcname = args[0];
    args[numOfArgs - 1] = NULL;

    #elif __WIN32

    prcname = "C:\\Windows\\System32\\WindowsPowerShell\\V1.0\\powershell.exe";
    
    #endif

    return true;
}

/**
 * @brief Launches process with command-line arguments
 * 
 * @param prcname   Process name
 * @param args      Command-line arguments
 * @param pipe      Anonymous pipe to send data through
 * @return          True if process successfully launched
 */
bool LaunchProcess(const char* prcname, char** args, PID pipe[2]) {

    #ifdef __APPLE__ || __linux__
    pid_t pid = fork();  // Create child process

    // Child process executes command
    if (pid == 0) {
        close(READ);
        dup2(pipe[WRITE], WRITE);
        execvp(prcname, args);
    }
    // Parent process waits for child to finish
    else {
        close(WRITE);
        dup2(pipe[READ], READ);

        // Wait for command to finish
        int exitcode = 0;
        waitpid(pid, &exitcode, 0);
        if (exitcode) { return false; }

        close(READ);  // Allows EOF to be set
    }

    #elif __WIN32  

    // TODO

    #endif

    return true;
}

string retrieveResults(PID readEndPoint) {
    string result;

    #ifdef __WIN32
    DWORD dwRead, dwAvail, dwLeft;
    char buffer[4096];
    BOOL bSuccess;

    // ASSUMPTION: CHILD WILL NEVER HANG
    do {
        bSuccess = PeekNamedPipe(*readEndPoint, buffer, 4095, &dwRead, &dwAvail, &dwLeft);
        if (!bSuccess || dwRead <= 0) { break; }
        ReadFile(*readEndPoint, buffer, 4095, &dwRead, NULL);
        buffer[dwRead] = '\0';

        result.append(buffer);

    } while (bSuccess && dwRead > 0)
    #elif __APPLE__ || __linux__

    // Read each line from cin
    string line;
    while(getline(cin, line)) {
        result += line;
    }

    #endif

    return result;
}

string runScript(const string& script) {

    // Get process name and arguments
    char* prcname = nullptr;
    char** args = nullptr;
    if (!processArgs(script, prcname, args)) { return ""; }

    // Create pipe
    PID pipefd[2];
    if (!createPipe(pipefd)) { return ""; };

    // Launch process
    LaunchProcess(prcname, args, pipefd);

    // Get results
    return retrieveResults(pipefd[READ]);
}

int main(int argc, char** argv) {

    string command = "ls";
    command = runScript(command);
    cout << command << endl;
    return 0;
}