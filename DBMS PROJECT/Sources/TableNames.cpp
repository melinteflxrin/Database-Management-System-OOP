#include "../Headers/TableNames.h"

using namespace std;

//private methods
string* names = nullptr;
int noNames = 0;

//public methods
//DESTRUCTOR
TableNames::~TableNames() {
	delete[] names;
}
//DEFAULT CONSTRUCTOR
TableNames::TableNames() {
	this->names = nullptr;
	this->noNames = 0;
	this->names = new string[noNames];
}
//ASSIGNMENT OPERATOR
TableNames& TableNames::operator=(const TableNames& original) {
	if (this == &original) return *this; // self assignment check

	delete[] names;

	noNames = original.noNames;
	names = new string[noNames];
	for (int i = 0; i < noNames; i++) {
		names[i] = original.names[i];
	}
	return *this;
}
//COPY CONSTRUCTOR (for somewhere in the assignment operator for table)
TableNames::TableNames(const TableNames& original) : noNames(original.noNames) {
	names = new string[noNames];
	for (int i = 0; i < noNames; i++) {
		names[i] = original.names[i];
	}
}

void TableNames::addName(const string& name) {
	string* newNames = new string[this->noNames + 1];

	if (names != nullptr) {
		for (int i = 0; i < this->noNames; i++) {
			newNames[i] = this->names[i];
		}
	}
	newNames[this->noNames] = name;
	delete[] names;
	names = newNames;
	noNames++;
}
void TableNames::removeName(const string& name) {
	if (!nameExists(name) || names == nullptr) {
		return;
	}

	string* newNames = new string[noNames - 1];
	int index = 0;

	for (int i = 0; i < noNames; i++) {
		if (names[i] != name) {
			newNames[index++] = names[i];
		}
	}

	delete[] names;
	names = (noNames - 1 > 0) ? newNames : nullptr;  //case where array becomes empty
	noNames--;
}

bool TableNames::nameExists(const string& name) const {
	for (int i = 0; i < noNames; i++) {
		if (names[i] == name) {
			return true;
		}
	}
	return false;
}
int TableNames::getSize() const {
	return noNames;
}