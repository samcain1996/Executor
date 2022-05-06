# Executor

Cross-platform code that allows for an OS command to be run and its results returned within a C++ program

## Set up

Compile executor.h and executor.cpp, no special flags should be needed.

GCC Example:
```
g++ executor.cpp
```

## Example

```
std::string results = RunCommand("ls -a");
std::cout << results;
```
On macOS the following code gives:
```
.
..
.git
.gitignore
.vscode
LICENSE
README.md
example.cpp
executor
executor.cpp
executor.dSYM
executor.h
executor.h.gch
```
