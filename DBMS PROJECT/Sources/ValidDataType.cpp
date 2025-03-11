#include "../Headers/ValidDataType.h"

using namespace std;

//public methods
bool ValidDataType::isValidInt(const string& value) {
	if (value.empty()) return false;
	size_t i = 0;

	//handle negative sign
	if (value[0] == '-') {
		if (value.length() == 1) return false; // "-" alone is not a valid integer
		i++;
	}

	//check that all remaining characters are digits
	for (; i < value.length(); i++) {
		if (value[i] < '0' || value[i] > '9') {
			return false; //non-digit character found
		}
	}

	return true;
}
bool ValidDataType::isValidFloat(const string& value) {
	if (value.empty()) return false;
	size_t i = 0;
	bool decimalPointFound = false;

	if (value[0] == '-') {
		if (value.length() == 1) return false;
		i++;
	}

	//iterate through the characters
	for (; i < value.length(); i++) {
		if (value[i] == '.') {
			if (decimalPointFound) return false; //more than one decimal point
			decimalPointFound = true;
		}
		else if (value[i] < '0' || value[i] > '9') {
			return false; //non-digit character found
		}
	}

	return true;
}