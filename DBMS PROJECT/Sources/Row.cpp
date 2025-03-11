#include "../Headers/Row.h"

using namespace std;

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

	if (!ValidDataType::isValidInt(value)) {
		throw invalid_argument("Provided value is not a valid integer.");
	}

	data[columnIndex] = value;
}
void Row::setFloatData(int columnIndex, const string& value) {
	if (columnIndex < 0 || columnIndex >= noColumns)
		throw out_of_range("Column index out of range.");

	if (!ValidDataType::isValidFloat(value)) {
		throw invalid_argument("Provided value is not a valid float.");
	}

	data[columnIndex] = value;
}

//GETTERS
int Row::getIntData(int columnIndex) const {
	if (columnIndex < 0 || columnIndex >= noColumns)
		throw out_of_range("Column index out of range.");
	if (!ValidDataType::isValidInt(data[columnIndex])) {
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
	if (!ValidDataType::isValidFloat(data[columnIndex])) {
		throw invalid_argument("Data at the specified index is not a valid float.");
	}
	return stof(data[columnIndex]);
}