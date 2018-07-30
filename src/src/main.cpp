#include "attribute.h"
#include "condition.h"
#include "interpreter.h"
#include "API.h"
#include <cstdlib>
#include <time.h>
#include <fstream>


int main(int argc, char* argv[]) {
	API api;
	CatalogManager cm;
	api.cm = &cm;
	RecordManager rm(&api);
	api.rm = &rm;
	IndexManager im(&api);
	api.im = &im;
	Interpreter ip(&api);
	fstream file;


	int state = TRUE_SYNTAX;
	string s;
	clock_t begin, finish;
	cout << "**************************** miniSQL 2017 Summer *****************************" << endl;
	cout << "*                                                                            *" << endl;
	cout << "*         Interpreter & Catalog       :              Han    Xia              *" << endl;
	cout << "*         API         & RecordManager :              LinHao Meng             *" << endl;
	cout << "*         BufferManager               :              Ang    Xie              *" << endl;
	cout << "*         IndexManager                :              Fan    Hu               *" << endl;
	cout << "*                                                                            *" << endl;
	cout << "****************************       Welcome!      *****************************" << endl;
	cout << endl << endl;
	while(true) {
		if(state == FILEREAD) {
			file.open(ip.FileName.c_str(),ios::in);
			if(!file.is_open()) {
				cout << "Commend Error: can not open file :" << ip.FileName << endl;
				state = TRUE_SYNTAX;
				continue;
			}
			while(getline(file, s, ';')) {
				state = ip.mainInterpreter(s);
			}
			file.close();
			finish = clock();
			double t = (double)(finish - begin) / CLOCKS_PER_SEC;
			cout << "Total Time: ";
			cout.width(5);
			cout << t << " Seconds." << endl;
			state = TRUE_SYNTAX;
		}
		else if(state == TRUE_SYNTAX || state == ERROR_SYNTAX) {
			cout << "miniSQL-->>";
			getline(cin, s, ';');
			begin = clock();
			state = ip.mainInterpreter(s);
			finish = clock();
			if(state != ERROR_SYNTAX) {
				double t = (double)(finish - begin) / CLOCKS_PER_SEC;
				cout << "Total Time: ";
				cout.width(5);
				cout << t << " Seconds." << endl;
			}
		}
		else {
			cout << "&& see you &&" << endl;
//			system("pause");
			break;
		}
	}
}
