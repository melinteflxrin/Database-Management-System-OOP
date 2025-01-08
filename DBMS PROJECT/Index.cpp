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
	this->indexValues = nullptr;
	this->columnNames = nullptr;
	this->tableNames = nullptr;
	this->noIndexes = 0;
}
//ASSIGNMENT OPERATOR
Index& Index::operator=(const Index& original) {
	if (this == &original) return *this; // self assignment check

	delete[] indexNames;
	delete[] indexValues;
	delete[] columnNames;
	delete[] tableNames;

	noIndexes = original.noIndexes;

	indexNames = new string[noIndexes];
	for (int i = 0; i < noIndexes; i++) {
		indexNames[i] = original.indexNames[i];
	}

	indexValues = new int[noIndexes];
	for (int i = 0; i < noIndexes; i++) {
		indexValues[i] = original.indexValues[i];
	}

	columnNames = new string[noIndexes];
	for (int i = 0; i < noIndexes; i++) {
		columnNames[i] = original.columnNames[i];
	}

	tableNames = new string[noIndexes];
	for (int i = 0; i < noIndexes; i++) {
		tableNames[i] = original.tableNames[i];
	}

	return *this;
}
//COPY CONSTRUCTOR
Index::Index(const Index& original) : noIndexes(original.noIndexes) {
	indexNames = new string[noIndexes];
	for (int i = 0; i < noIndexes; i++) {
		indexNames[i] = original.indexNames[i];
	}

	indexValues = new int[noIndexes];
	for (int i = 0; i < noIndexes; i++) {
		indexValues[i] = original.indexValues[i];
	}

	columnNames = new string[noIndexes];
	for (int i = 0; i < noIndexes; i++) {
		columnNames[i] = original.columnNames[i];
	}

	tableNames = new string[noIndexes];
	for (int i = 0; i < noIndexes; i++) {
		tableNames[i] = original.tableNames[i];
	}
}
//GETTERS
const int Index::getNoIndexes() const {
	return noIndexes;
}
const string& Index::getIndexName(const int index) const {
	return indexNames[index];
}
int Index::getIndexValue(const string& columnName, const string& tableName) const {
	for (int i = 0; i < noIndexes; i++) {
		if (columnNames[i] == columnName && tableNames[i] == tableName) {
			return indexValues[i];
		}
	}
	return -1;
}
string Index::getIndexColumnName(const string& indexName) const {
	for (int i = 0; i < noIndexes; i++) {
		if (indexNames[i] == indexName) {
			return columnNames[i];
		}
	}
	return "";
}
string Index::getIndexTableName(const string& indexName) const {
	for (int i = 0; i < noIndexes; i++) {
		if (indexNames[i] == indexName) {
			return tableNames[i];
		}
	}
	return "";
}

void Index::addIndex(const string& indexName, int indexValue, const string& columnName, const string& tableName) {
	string* newIndexNames = new string[noIndexes + 1];
	int* newIndexValues = new int[noIndexes + 1];
	string* newColumnNames = new string[noIndexes + 1];
	string* newTableNames = new string[noIndexes + 1];

	if (indexNames != nullptr) {
		for (int i = 0; i < noIndexes; i++) {
			newIndexNames[i] = indexNames[i];
		}
	}
	newIndexNames[noIndexes] = indexName;
	delete[] indexNames;
	indexNames = newIndexNames;

	if (indexValues != nullptr) {
		for (int i = 0; i < noIndexes; i++) {
			newIndexValues[i] = indexValues[i];
		}
	}
	newIndexValues[noIndexes] = indexValue;
	delete[] indexValues;
	indexValues = newIndexValues;

	if (columnNames != nullptr) {
		for (int i = 0; i < noIndexes; i++) {
			newColumnNames[i] = columnNames[i];
		}
	}
	newColumnNames[noIndexes] = columnName;
	delete[] columnNames;
	columnNames = newColumnNames;

	if (tableNames != nullptr) {
		for (int i = 0; i < noIndexes; i++) {
			newTableNames[i] = tableNames[i];
		}
	}
	newTableNames[noIndexes] = tableName;
	delete[] tableNames;
	tableNames = newTableNames;

	noIndexes++;
}
void Index::removeIndex(const string& columnName, const string& tableName) {
	int index = -1;
	for (int i = 0; i < noIndexes; i++) {
		if (columnNames[i] == columnName && tableNames[i] == tableName) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		cout << endl << "Index not found.";
		return;
	}

	string* newIndexNames = new string[noIndexes - 1];
	int* newIndexValues = new int[noIndexes - 1];
	string* newColumnNames = new string[noIndexes - 1];
	string* newTableNames = new string[noIndexes - 1];

	int tempIndex = 0;
	for (int i = 0; i < noIndexes; i++) {
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

	noIndexes--;
}
void Index::removeIndexByIndexName(const string& indexName) {
	int index = -1;
	for (int i = 0; i < noIndexes; i++) {
		if (indexNames[i] == indexName) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		cout << endl << "Index not found.";
		return;
	}

	string* newIndexNames = new string[noIndexes - 1];
	int* newIndexValues = new int[noIndexes - 1];
	string* newColumnNames = new string[noIndexes - 1];
	string* newTableNames = new string[noIndexes - 1];

	int tempIndex = 0;
	for (int i = 0; i < noIndexes; i++) {
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

	noIndexes--;
}
void Index::removeIndexByTableName(const string& tableName) {
	int index = -1;
	for (int i = 0; i < noIndexes; i++) {
		if (tableNames[i] == tableName) {
			index = i;
			break;
		}
	}

	if (index == -1) {
		cout << endl << "Index not found.";
		return;
	}

	string* newIndexNames = new string[noIndexes - 1];
	int* newIndexValues = new int[noIndexes - 1];
	string* newColumnNames = new string[noIndexes - 1];
	string* newTableNames = new string[noIndexes - 1];

	int tempIndex = 0;
	for (int i = 0; i < noIndexes; i++) {
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

	noIndexes--;
}

bool Index::indexExistsByIndexName(const string& indexName) const {  //for create index
	for (int i = 0; i < noIndexes; i++) {
		if (indexNames[i] == indexName) {
			return true;
		}
	}
	return false;
}
bool Index::indexExists(const string& columnName, const string& tableName) const {  //for select where & delete where
	for (int i = 0; i < noIndexes; i++) {
		if (columnNames[i] == columnName && tableNames[i] == tableName) {
			return true;
		}
	}
	return false;
}
bool Index::indexExistsByTableName(const string& tableName) const {
	//for drop table
	for (int i = 0; i < noIndexes; i++) {
		if (tableNames[i] == tableName) {
			return true;
		}
	}
	return false;
}

void Index::showIndexesByTableName(const string& tableName) const {
	bool found = false;
	for (int i = 0; i < noIndexes; i++) {
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
	if (noIndexes == 0) {
		cout << endl << "No indexes found.";
		return;
	}
	for (int i = 0; i < noIndexes; i++) {
		cout << endl << i + 1 << ". '" << indexNames[i] << "' on column: " << columnNames[i] << " in table: " << tableNames[i];
	}
}