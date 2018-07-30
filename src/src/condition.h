#ifndef MiniSQL_Condition_H
#define MiniSQL_Condition_H

#include <string>
#include <sstream>

using namespace std;

extern const int OP_EQUAL;
extern const int OP_NOT_EQUAL;
extern const int OP_LESS;
extern const int OP_GREATER;
extern const int OP_LESS_EQUAL;
extern const int OP_GREATER_EQUAL;


class Condition {
public:
    Condition(string attr, int op, string val);
    ~Condition();
    bool check(int content);
    bool check(float content);
    bool check(string content);
    string getAttr() { return attr; }
    int getOP() { return op; }
    string getval() { return val; }
private:
    string attr, val;
    int op;
};
#endif // !MiniSQL_Condition_H
