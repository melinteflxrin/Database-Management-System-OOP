#include "Column.h"
#include <iostream>

using namespace std;

//private methods
bool Column::isValidInt(const string& value) {
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
bool Column::isValidFloat(const string& value) {
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
//SETTERS
void Column::setName(const string& name) {
	if (name.empty() || name.size() < 2) {
		throw invalid_argument("Name cannot be empty or less than two characters.");
	}
	this->name = name;
}
void Column::setType(ColumnType type) {
	this->type = type;
}
void Column::setSize(int size) {
	if (size <= 0) {
		throw invalid_argument("Size must be larger than zero.");
	}
	this->size = size;
}
void Column::setDefaultValue(const string& defaultValue) {
	if (defaultValue.size() > this->size) {
		throw invalid_argument("Default value cannot be larger than the column size.");
	}
	switch (this->type) {
	case TEXT:
		this->defaultValue = defaultValue;
		break;
	case INT:
		if (isValidInt(defaultValue)) {
			this->defaultValue = defaultValue;
		}
		else {
			throw invalid_argument("Default value must be a valid integer.");
		}
		break;
	case FLOAT:
		if (isValidFloat(defaultValue)) {
			this->defaultValue = defaultValue;
		}
		else {
			throw invalid_argument("Default value must be a valid float.");
		}
		break;
	default:
		throw invalid_argument("Invalid column type.");
	}
}

//GETTERS
const string& Column::getName() const {
	return this->name;
}
ColumnType Column::getType() const {
	return this->type;
}
int Column::getSize() const {
	return this->size;
}
const string& Column::getDefaultValue() const {
	return this->defaultValue;
}
const bool Column::isUnique() const {
	return this->unique;
}

//DEFAULT CONSTRUCTOR
Column::Column() {
	this->name = "";
	this->type = TEXT;
	this->size = 0;
	this->defaultValue = "";
	this->unique = false;
}
//CONSTRUCTOR
Column::Column(const string& name, ColumnType type, int size, const string& defaultValue) {
	this->setName(name);
	this->setType(type);
	this->setSize(size);
	this->setDefaultValue(defaultValue);
	this->unique = false;
}
//CONSTRUCTOR FOR UNIQUE
Column::Column(const string& name, ColumnType type, int size, const string& defaultValue, bool unique) {
	this->setName(name);
	this->setType(type);
	this->setSize(size);
	this->setDefaultValue(defaultValue);
	this->unique = unique;
}