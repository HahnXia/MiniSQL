#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <string>
#include <vector>
#include "api.h"
#include "condition.h"
#include "attribute.h"

using namespace std;

#ifndef INTERPRETER_DEF
#define INTERPRETER_DEF
#define FILEREAD 2
#define QUIT -1
#define ERROR_SYNTAX 0
#define TRUE_SYNTAX 1

#endif

class Interpreter {
public:

    Interpreter(API * api) : api(api) {}
    ~Interpreter() {}
    API * api;
    string FileName;
    int mainInterpreter(string s);
private:
    string getNextString(string s, int *pos);
    bool isWord(string s);

};

#endif
