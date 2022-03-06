#include "executor.h"

using namespace std;

/**
 * @brief Counts the number of words in a string
 * 
 * @param words     string to count words of
 * @return int      0 if words is empty, else number of words in string
 */
size_t countWords(const char* words) {
    size_t wordCount = 0;
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
 * @brief               Convert char* array to a char* with a 
 *                      seperator between array elements
 * 
 * @param argc          Number of element in array
 * @param args          Array to convert
 * @param seperator     Character separating array elements
 * @return char*        Character string to flattened array
 */
char* flatten(size_t argc, char** args, char seperator = ' ') {

    // Get length of all words with separator between them
    size_t argSize = 0;
    for (int i = 0; i < argc; i++) {
        argSize += strlen(args[i]) + 1;
    }

    // Convert char** to char*
    char* smashed = new char[argSize];
    size_t pos = 0;
    for (int i = 0; i < argc; i++) {
        // Copy word in word char* array to current position in char*
        memcpy(&smashed[pos], args[i], strlen(args[i]));
        pos += strlen(args[i]);

        // Free memory storing word
        delete[] args[i];

        // Concat a separator between words
        memcpy(&smashed[pos], &seperator, sizeof(char));
        pos++;
    }
    
    // Set null character
    smashed[pos-1] = static_cast<char>(NULL);

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

    // Allow pipe to inhereit 
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
bool ProcessArgs(const string& src, char*& prcname, Args& args) {
    // Nothing to split
    if (src.empty()) { return false; }

    size_t numOfArgs = countWords(src.c_str()) + 1;   // Number of words in str
    size_t startIdx = ARG_START_IDX;
    args = new char*[numOfArgs];

    // Loop through each word in str
    for (size_t argIdx = startIdx, curIdx = 0; 
          argIdx < numOfArgs && curIdx < src.size(); argIdx++) {

        // Copy argument to char*
        char* arg = new char[strlen(src.substr(curIdx, src.substr(curIdx).find(" ")).c_str()) + 1];
        strcpy(arg, src.substr(curIdx, src.substr(curIdx).find(" ")).c_str());

        // Store arg in array of args
        reinterpret_cast<char**>(args)[argIdx] = arg;

        // Move starting index
        curIdx += src.substr(curIdx).find(" ") + 1;
    }

    reinterpret_cast<char**>(args)[numOfArgs] = NULL;   // Last argument is NULL in Unix-like systems
    prcname = reinterpret_cast<char**>(args)[0];                    // Extract process name

    #if defined(_WIN32)

    reinterpret_cast<char**>(args)[0] = new char[] { PSPATH };
    args = flatten(numOfArgs, reinterpret_cast<char**>(args));

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
bool LaunchProcess(const char* prcname, Args& args, PID pipe[2]) {

    #if defined(__APPLE__) || defined(__linux__)

    pid_t pid = fork();  // Create child process

    // Child process executes command
    if (pid == 0) {
        close(pipe[READ]);
        dup2(pipe[WRITE], WRITE);

        execvp(prcname, DEDUCE_TYPE(args));
    }
    // Parent process waits for child to finish
    else {
        close(pipe[WRITE]);
        dup2(pipe[READ], READ);

        // Wait for command to finish
        int status = 0;
        waitpid(pid, &status, 0);

        close(pipe[READ]);  // Allows EOF to be set
    }

    #elif defined(_WIN32)  

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));     
    ZeroMemory(&si.cb, sizeof(si));
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdError = pipe[WRITE];
    si.hStdOutput = pipe[WRITE];

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcessA(
         PSPATH,
         DEDUCE_TYPE(args),
         NULL,
         NULL,
         TRUE,
         0,
         NULL,
         NULL,
         &si,
         &pi
     )) { cerr << "Error creating process!"; return false; }

     CloseHandle(pipe[WRITE]);  // Parent process doesn't need to write

     // Wait for process to finish
     WaitForSingleObject(pi.hProcess, INFINITE);
     CloseHandle(pi.hProcess);
     CloseHandle(pi.hThread);

    #endif
    
    return true;
}

/**
 * @brief               Reads results from child process through pipe
 * 
 * @param readEndPoint  HANDLE to pipe in Windows
 * @return string       Contents of pipe
 */
string RetrieveResults(PID pipeRead) {
    string result;

    #if defined(_WIN32)

    // Variables for reading from pipe
    DWORD dwRead, dwAvail, dwLeft;
    char buffer[4096];
    BOOL bSuccess;


    // Extract data from pipe
    // ASSUMPTION: CHILD WILL NEVER HANG
    do {
        // Check if pipe has content to read
        bSuccess = PeekNamedPipe(pipeRead, buffer, 4095, &dwRead, &dwAvail, &dwLeft);
        if (!bSuccess || dwRead <= 0) { break; }

        // Read content, append to result string
        bSuccess = ReadFile(pipeRead, buffer, 4095, &dwRead, NULL);
        buffer[dwRead] = NULL;

        result.append(buffer);

    } while (bSuccess && dwRead > 0);

    CloseHandle(pipeRead);

    #elif defined(__APPLE__) || defined(__linux__)

    // Read each line from cin
    string line;
    while(getline(cin, line)) {
        result += line + "\n";
    }
    if (!result.empty()) {result.erase(result.end() - 1);}  // Erase trailing '\n'

    #endif

    return result;
}

/**
 * @brief           Deletes arguments
 * 
 * @param args      Arguments to delete
 * @param count     Number of args to delete if stored as a char**
 */
void DeleteArgs(Args& args, size_t count = 0) {
    #if defined(__APPLE__) || defined(_linux__)

    for (int i = 0; i < count; i++) {
        delete[] (DEDUCE_TYPE(args))[i];
    }
    delete[] DEDUCE_TYPE(args);

    #elif defined(_WIN32)

    delete[] DEDUCE_TYPE(args);

    #endif
}

string RunScript(const string& script) {

    string results;

    // Get process name and arguments
    char* prcname = nullptr;
    void* args = nullptr;
    if (!ProcessArgs(script, prcname, args)) { return results; }

    // Create pipe
    PID pipefd[2];
    if (!createPipe(pipefd)) { return results; };

    // Launch process
    if (!LaunchProcess(prcname, args, pipefd)) { return results; }

    // Get results
    results = RetrieveResults(pipefd[READ]);

    // Free memory
    DeleteArgs(args, countWords(script.c_str()));

    return results;
}

int main(/*int argc, char** argv*/) {

    string command = "dir\n";
    command = RunScript(command);
    cout << command << endl;
    return 0;

}