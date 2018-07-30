#include "Condition.h"

const int OP_EQUAL = 0;
const int OP_NOT_EQUAL = 1;
const int OP_LESS = 2;
const int OP_GREATER = 3;
const int OP_LESS_EQUAL = 4;
const int OP_GREATER_EQUAL = 5;

Condition::Condition(string attr, int op, string val) : attr(attr), op(op), val(val) {
}

Condition::~Condition() {
}

bool Condition::check(int content) {
    stringstream ss;
    int thisvalue;
    ss << this->val;
    ss >> thisvalue;

    switch(op) {
    case OP_EQUAL:
        return content == thisvalue;
        break;
    case OP_GREATER:
        return content > thisvalue;
        break;
    case OP_GREATER_EQUAL:
        return content >= thisvalue;
        break;
    case OP_LESS:
        return content < thisvalue;
        break;
    case OP_LESS_EQUAL:
        return content <= thisvalue;
        break;
    case OP_NOT_EQUAL:
        return content != thisvalue;
        break;
    default:
        return 0;
        break;
    }
}
bool Condition::check(float content) {
    stringstream ss;
    float thisvalue;
    ss << this->val;
    ss >> thisvalue;

    switch(op) {
    case OP_EQUAL:
        return content == thisvalue;
        break;
    case OP_GREATER:
        return content > thisvalue;
        break;
    case OP_GREATER_EQUAL:
        return content >= thisvalue;
        break;
    case OP_LESS:
        return content < thisvalue;
        break;
    case OP_LESS_EQUAL:
        return content <= thisvalue;
        break;
    case OP_NOT_EQUAL:
        return content != thisvalue;
        break;
    default:
        return 0;
        break;
    }
}
bool Condition::check(string content) {
    switch(op) {
    case OP_EQUAL:
        return content == val;
        break;
    case OP_GREATER:
        return content > val;
        break;
    case OP_GREATER_EQUAL:
        return content >= val;
        break;
    case OP_LESS:
        return content < val;
        break;
    case OP_LESS_EQUAL:
        return content <= val;
        break;
    case OP_NOT_EQUAL:
        return content != val;
        break;
    default:
        return 0;
        break;
    }
}
