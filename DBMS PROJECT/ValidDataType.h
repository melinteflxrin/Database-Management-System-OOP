#ifndef VALIDDATATYPE_H
#define VALIDDATATYPE_H

#include <string>

using namespace std;

class ValidDataType {
public:
	static bool isValidInt(const string& value);
	static bool isValidFloat(const string& value);
};

#endif 