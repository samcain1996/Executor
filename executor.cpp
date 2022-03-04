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

char* flatten(int argc, char** args) {
    int argSize = 0;
    for (int i = 0; i < argc; i++) {
        argSize += strlen(args[i]) + 1;
    }

    char* smashed = new char[argSize];
    int pos = 0;
    for (int i = 0; i < argc; i++) {
        memcpy(&smashed[pos], args[i], strlen(args[i]));
        pos += strlen(args[i]);
        memcpy(&smashed[pos], " ", 1);
        pos++;
    }
    
    smashed[pos-1] = '\0';

    return smashed;
}

/**
 * @brief Create pipe from specified endpoints
 * 
 * @param endPoints     Read and write ends to pipe
 * @return              True on successful pipe creation,
 *                      otherwise false.
 */
bool createPipe(PID endPoints[2]) {

    #if defined(_WIN32)

    SECURITY_ATTRIBUTES secAttr;
    secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    secAttr.bInheritHandle = TRUE;
    secAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&endPoints[READ], &endPoints[WRITE], &secAttr, 0)) { 
        return false; 
    }

    #elif defined(__APPLE__) || defined(__linux__)
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
bool processArgs(const string& src, char*& prcname, Args& args) {
    // Nothing to split
    if (src.empty()) { return false; }

    int numOfArgs = countWords(src.c_str());   // Number of words in str

    #if defined(__APPLE__) || defined(__linux__)
        numOfArgs++;
    #endif

    args = new char*[numOfArgs];

    // Loop through each word in str
    for (size_t argIdx = 0, curIdx = 0; 
          argIdx < numOfArgs && curIdx < src.size(); argIdx++) {

        // Local copy of word
        char* arg = new char[strlen(src.substr(curIdx, src.substr(curIdx).find(" ")).c_str()) + 1];
        memcpy(arg, src.substr(curIdx, src.substr(curIdx).find(" ")).c_str(), strlen(src.substr(curIdx, src.substr(curIdx).find(" ")).c_str()));
        arg[strlen(arg)] = NULL;

        // Copy word to word array
        reinterpret<char**>(args)[argIdx] = new char[strlen(arg) + 1];
        strcpy(reinterpret<char**>(args)[argIdx], arg);

        // Move starting index
        curIdx += src.substr(curIdx).find(" ") + 1;
    }

    #if defined(__APPLE__) || defined(__linux__)

    prcname = reinterpret<char**>(args)[0];
    reinterpret<char**>(args)[numOfArgs - 1] = NULL;

    #elif defined(_WIN32)

    prcname = const_cast<char*>("C:\\Windows\\System32\\WindowsPowerShell\\V1.0\\powershell.exe");
    args = flatten(numOfArgs, reinterpret<char**>args);
    
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
bool LaunchProcess(const char* prcname, Args args, PID pipe[2]) {

    #if defined(__APPLE__) || defined(__linux__)
    pid_t pid = fork();  // Create child process

    // Child process executes command
    if (pid == 0) {
        close(pipe[READ]);
        dup2(pipe[WRITE], WRITE);
        execvp(prcname, reinterpret<char**>(args));
    }
    // Parent process waits for child to finish
    else {
        close(pipe[WRITE]);
        dup2(pipe[READ], READ);

        // Wait for command to finish
        int exitcode = 0;
        waitpid(pid, &exitcode, 0);

        close(pipe[READ]);  // Allows EOF to be set
    }

    #elif defined(_WIN32)  

     STARTUPINFOA si;
     ZeroMemory(&si, sizeof(si));     // Init memory
     ZeroMemory(&si.cb, sizeof(si));
     si.dwFlags |= STARTF_USESTDHANDLES;
     si.hStdInput = pipe[READ];
     si.hStdError = pipe[WRITE];
     si.hStdOutput = pipe[WRITE];

     PROCESS_INFORMATION pi;
     ZeroMemory(&pi, sizeof(pi));

     if (!CreateProcessA(
         prcname,
         (char*)args,
         NULL,
         NULL,
         TRUE,
         NORMAL_PRIORITY_CLASS,
         NULL,
         NULL,
         &si,
         &pi
     )) { cerr << "Error creating process!"; return false; }


    #endif

    return true;
}

string retrieveResults(PID readEndPoint) {
    string result;

    #if defined(_WIN32)
    DWORD dwRead, dwAvail, dwLeft;
    char buffer[4096];
    BOOL bSuccess;

    // ASSUMPTION: CHILD WILL NEVER HANG
    do {
        bSuccess = PeekNamedPipe(readEndPoint, buffer, 4095, &dwRead, &dwAvail, &dwLeft);
        if (!bSuccess || dwRead <= 0) { break; }
        ReadFile(readEndPoint, buffer, 4095, &dwRead, NULL);
        buffer[dwRead] = '\0';

        result.append(buffer);

    } while (bSuccess && dwRead > 0)
    #elif defined(__APPLE__) || defined(__linux__)

    // Read each line from cin
    string line;
    while(getline(cin, line)) {
        result += line + "\n";
    }

    #endif

    return result;
}

string runScript(const string& script) {

    // Get process name and arguments
    char* prcname = nullptr;
    void* args = nullptr;
    if (!processArgs(script, prcname, args)) { return ""; }

    // Create pipe
    PID pipefd[2];
    if (!createPipe(pipefd)) { return ""; };

    // Launch process
    if (!LaunchProcess(prcname, args, pipefd)) { return ""; }

    // Get results
    return retrieveResults(pipefd[READ]);
}

int main(int argc, char** argv) {

    string command = "ls -a";
    command = runScript(command);
    cout << command << endl;
    return 0;
}