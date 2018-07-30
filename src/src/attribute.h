#ifndef MiniSQL_Attribute_h
#define MiniSQL_Attribute_h

#include <string>
#include <iostream>
using namespace std;

#ifndef Attribute_TYPE
#define Attribute_TYPE
#define TYPE_FLOAT -1	//the type of the attribute,-1 represents float
#define TYPE_INT 0		//0 represents int
// other positive integer represents char and the value is the number of char
#endif

class Attribute {
public:
    string name;
    int type;

    bool ifUnique;
    string index;	//default value is "", representing no index

	Attribute():index("") {}
    Attribute(string n, int t, bool i):name(n), type(t), ifUnique(i), index("") {}
    ~Attribute() { }
    string GetIndex() { return index; }

    void Print() {
        cout << "name: " << name << endl;
        cout << "type: " << type << endl;
        cout << "ifUnique: " << ifUnique << endl;
        cout << "index: " << index << endl;
    }
};

#endif
