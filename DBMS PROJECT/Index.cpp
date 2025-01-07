#include "Index.h"
#include <iostream>

using namespace std;

//public methods
//DESTRUCTOR
Index::~Index() {
	delete[] indexNames;
	delete[] indexValues;
	delete[] columnNames;
	delete[] tableNames;
}
//DEFAULT CONSTRUCTOR
Index::Index() {
	this->indexNames = nullptr;
	this->noIndexeNames = 0;
	this->indexValues = nullptr;
	this->noIndexValues = 0;
	this->columnNames = nullptr;
	this->noColumnNames = 0;
	this->tableNames = nullptr;
	this->noTableNames = 0;
}
//ASSIGNMENT OPERATOR
Index& Index::operator=(const Index& original) {
	if (this == &original) return *this; // self assignment check

	delete[] indexNames;
	delete[] indexValues;
	delete[] columnNames;
	delete[] tableNames;

	noIndexeNames = original.noIndexeNames;
	indexNames = new string[noIndexeNames];
	for (int i = 0; i < noIndexeNames; i++) {
		indexNames[i] = original.indexNames[i];
	}

	noIndexValues = original.noIndexValues;
	indexValues = new int[noIndexValues];
	for (int i = 0; i < noIndexValues; i++) {
		indexValues[i] = original.indexValues[i];
	}

	noColumnNames = original.noColumnNames;
	columnNames = new string[noColumnNames];
	for (int i = 0; i < noColumnNames; i++) {
		columnNames[i] = original.columnNames[i];
	}

	noTableNames = original.noTableNames;
	tableNames = new string[noTableNames];
	for (int i = 0; i < noTableNames; i++) {
		tableNames[i] = original.tableNames[i];
	}

	return *this;
}
//COPY CONSTRUCTOR
Index::Index(const Index& original) : noIndexeNames(original.noIndexeNames), noIndexValues(original.noIndexValues), noColumnNames(original.noColumnNames), noTableNames(original.noTableNames) {
	indexNames = new string[noIndexeNames];
	for (int i = 0; i < noIndexeNames; i++) {
		indexNames[i] = original.indexNames[i];
	}
}

void Index::addIndex(const string& indexName, int indexValue, const string& columnName, const string& tableName) {
	string* newIndexNames = new string[noIndexeNames + 1];
	int* newIndexValues = new int[noIndexValues + 1];
	string* newColumnNames = new string[noColumnNames + 1];
	string* newTableNames = new string[noTableNames + 1];

	if (indexNames != nullptr) {
		for (int i = 0; i < noIndexeNames; i++) {
			newIndexNames[i] = indexNames[i];
		}
	}
	newIndexNames[noIndexeNames] = indexName;
	delete[] indexNames;
	indexNames = newIndexNames;
	noIndexeNames++;

	if (indexValues != nullptr) {
		for (int i = 0; i < noIndexValues; i++) {
			newIndexValues[i] = indexValues[i];
		}
	}
	newIndexValues[noIndexValues] = indexValue;
	delete[] indexValues;
	indexValues = newIndexValues;
	noIndexValues++;

	if (columnNames != nullptr) {
		for (int i = 0; i < noColumnNames; i++) {
			newColumnNames[i] = columnNames[i];
		}
	}
	newColumnNames[noColumnNames] = columnName;
	delete[] columnNames;
	columnNames = newColumnNames;
	noColumnNames++;

	if (tableNames != nullptr) {
		for (int i = 0; i < noTableNames; i++) {
			newTableNames[i] = tableNames[i];
		}
	}
	newTableNames[noTableNames] = tableName;
	delete[] tableNames;
	tableNames = newTableNames;
	noTableNames++;
}
void Index::removeIndex(const string& columnName, const string& tableName) {
	int index = -1;
	for (int i = 0; i < noColumnNames; i++) {
		if (columnNames[i] == columnName && tableNames[i] == tableName) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		cout << endl << "Index not found.";
		return;
	}

	string* newIndexNames = new string[noIndexeNames - 1];
	int* newIndexValues = new int[noIndexValues - 1];
	string* newColumnNames = new string[noColumnNames - 1];
	string* newTableNames = new string[noTableNames - 1];

	int tempIndex = 0;
	for (int i = 0; i < noIndexeNames; i++) {
		if (i != index) {
			newIndexNames[tempIndex] = indexNames[i];
			newIndexValues[tempIndex] = indexValues[i];
			newColumnNames[tempIndex] = columnNames[i];
			newTableNames[tempIndex] = tableNames[i];
			tempIndex++;
		}
	}

	delete[] indexNames;
	delete[] indexValues;
	delete[] columnNames;
	delete[] tableNames;

	indexNames = newIndexNames;
	indexValues = newIndexValues;
	columnNames = newColumnNames;
	tableNames = newTableNames;

	noIndexeNames--;
	noIndexValues--;
	noColumnNames--;
	noTableNames--;
}
void Index::removeIndexByIndexName(const string& indexName) {
	int index = -1;
	for (int i = 0; i < noIndexeNames; i++) {
		if (indexNames[i] == indexName) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		cout << endl << "Index not found.";
		return;
	}

	string* newIndexNames = new string[noIndexeNames - 1];
	int* newIndexValues = new int[noIndexValues - 1];
	string* newColumnNames = new string[noColumnNames - 1];
	string* newTableNames = new string[noTableNames - 1];

	int tempIndex = 0;
	for (int i = 0; i < noIndexeNames; i++) {
		if (i != index) {
			newIndexNames[tempIndex] = indexNames[i];
			newIndexValues[tempIndex] = indexValues[i];
			newColumnNames[tempIndex] = columnNames[i];
			newTableNames[tempIndex] = tableNames[i];
			tempIndex++;
		}
	}

	delete[] indexNames;
	delete[] indexValues;
	delete[] columnNames;
	delete[] tableNames;

	indexNames = newIndexNames;
	indexValues = newIndexValues;
	columnNames = newColumnNames;
	tableNames = newTableNames;

	noIndexeNames--;
	noIndexValues--;
	noColumnNames--;
	noTableNames--;
}
void Index::removeIndexByTableName(const string& tableName) {
	int index = -1;
	for (int i = 0; i < noTableNames; i++) {
		if (tableNames[i] == tableName) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		cout << endl << "Index not found.";
		return;
	}

	string* newIndexNames = new string[noIndexeNames - 1];
	int* newIndexValues = new int[noIndexValues - 1];
	string* newColumnNames = new string[noColumnNames - 1];
	string* newTableNames = new string[noTableNames - 1];

	int tempIndex = 0;
	for (int i = 0; i < noIndexeNames; i++) {
		if (i != index) {
			newIndexNames[tempIndex] = indexNames[i];
			newIndexValues[tempIndex] = indexValues[i];
			newColumnNames[tempIndex] = columnNames[i];
			newTableNames[tempIndex] = tableNames[i];
			tempIndex++;
		}
	}

	delete[] indexNames;
	delete[] indexValues;
	delete[] columnNames;
	delete[] tableNames;

	indexNames = newIndexNames;
	indexValues = newIndexValues;
	columnNames = newColumnNames;
	tableNames = newTableNames;

	noIndexeNames--;
	noIndexValues--;
	noColumnNames--;
	noTableNames--;
}

bool Index::indexExistsByIndexName(const string& indexName) const {  //for create index
	for (int i = 0; i < noIndexeNames; i++) {
		if (indexNames[i] == indexName) {
			return true;
		}
	}
	return false;
}
bool Index::indexExists(const string& columnName, const string& tableName) const {  //for select where & delete where
	for (int i = 0; i < noColumnNames; i++) {
		if (columnNames[i] == columnName && tableNames[i] == tableName) {
			return true;
		}
	}
	return false;
}
bool Index::indexExistsByTableName(const string& tableName) const {
	//for drop table
	for (int i = 0; i < noTableNames; i++) {
		if (tableNames[i] == tableName) {
			return true;
		}
	}
	return false;
}

int Index::getIndexValue(const string& columnName, const string& tableName) const {
	for (int i = 0; i < noColumnNames; i++) {
		if (columnNames[i] == columnName && tableNames[i] == tableName) {
			return indexValues[i];
		}
	}
	return -1;
}

string Index::getIndexColumnName(const string& indexName) const {
	for (int i = 0; i < noIndexeNames; i++) {
		if (indexNames[i] == indexName) {
			return columnNames[i];
		}
	}
	return "";
}
string Index::getIndexTableName(const string& indexName) const {
	for (int i = 0; i < noIndexeNames; i++) {
		if (indexNames[i] == indexName) {
			return tableNames[i];
		}
	}
	return "";
}

void Index::showIndexesByTableName(const string& tableName) const {
	bool found = false;
	for (int i = 0; i < noTableNames; i++) {
		if (tableNames[i] == tableName) {
			cout << endl << i + 1 << ". '" << indexNames[i] << "' on column: " << columnNames[i];
			found = true;
		}
	}
	if (!found) {
		cout << endl << "No indexes found for table '" << tableName << "'.";
	}
}
void Index::showAllIndexes() const {
	if (noIndexeNames == 0) {
		cout << endl << "No indexes found.";
		return;
	}
	for (int i = 0; i < noIndexeNames; i++) {
		cout << endl << i + 1 << ". '" << indexNames[i] << "' on column: " << columnNames[i] << " in table: " << tableNames[i];
	}
}