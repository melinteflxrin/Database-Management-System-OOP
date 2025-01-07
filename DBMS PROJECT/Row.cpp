#include "Row.h"
#include <iostream>

using namespace std;

//private methods
bool Row::isValidInt(const string& value) const {
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
bool Row::isValidFloat(const string& value) const {
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

//public methods
//DESTRUCTOR
Row::~Row() {
	delete[] data;
}
//DEFAULT CONSTRUCTOR
Row::Row() {
	this->data = nullptr;
	this->noColumns = 0;
}
//CONSTRUCTOR
Row::Row(int noColumns) {
	this->noColumns = noColumns;
	data = new string[noColumns];
}
//COPY CONSTRUCTOR
Row::Row(const Row& original) : noColumns(original.noColumns) {
	data = new string[noColumns];
	for (int i = 0; i < noColumns; i++) {
		data[i] = original.data[i];  // deep copy
	}
}
//ASSIGNMENT OPERATOR
Row& Row::operator=(const Row& original) {
	if (this == &original) return *this; //self assignment check

	delete[] data;

	noColumns = original.noColumns;
	data = new string[noColumns];
	for (int i = 0; i < noColumns; i++) {
		data[i] = original.data[i];
	}
	return *this;
}

//SETTERS
void Row::setStringData(int columnIndex, const string& value) {
	if (columnIndex < 0 || columnIndex >= noColumns)
		throw out_of_range("Column index out of range.");

	data[columnIndex] = value;
}
void Row::setIntData(int columnIndex, const string& value) {
	if (columnIndex < 0 || columnIndex >= noColumns)
		throw out_of_range("Column index out of range.");

	if (!isValidInt(value)) {
		throw invalid_argument("Provided value is not a valid integer.");
	}

	data[columnIndex] = value;
}
void Row::setFloatData(int columnIndex, const string& value) {
	if (columnIndex < 0 || columnIndex >= noColumns)
		throw out_of_range("Column index out of range.");

	if (!isValidFloat(value)) {
		throw invalid_argument("Provided value is not a valid float.");
	}

	data[columnIndex] = value;
}

//GETTERS
int Row::getIntData(int columnIndex) const {
	if (columnIndex < 0 || columnIndex >= noColumns)
		throw out_of_range("Column index out of range.");
	if (!isValidInt(data[columnIndex])) {
		throw invalid_argument("Data at the specified index is not a valid integer.");
	}
	return stoi(data[columnIndex]);
}
string Row::getTextData(int columnIndex) const {
	if (columnIndex < 0 || columnIndex >= noColumns)
		throw out_of_range("Column index out of range.");

	return data[columnIndex];
}
float Row::getFloatData(int columnIndex) const {
	if (columnIndex < 0 || columnIndex >= noColumns)
		throw out_of_range("Column index out of range.");
	if (!isValidFloat(data[columnIndex])) {
		throw invalid_argument("Data at the specified index is not a valid float.");
	}
	return stof(data[columnIndex]);
}