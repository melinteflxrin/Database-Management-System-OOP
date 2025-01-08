#ifndef VALIDATEDATATYPE_H
#define VALIDATEDATATYPE_H

#include <string>

using namespace std;

class ValidateDataType {
public:
	static bool isValidInt(const string& value);
	static bool isValidFloat(const string& value);
};

#endif 