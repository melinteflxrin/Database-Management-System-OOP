#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "Column.h"
#include "Row.h"
#include "TableNames.h"
#include "Table.h"

using namespace std;

class KeyNode {
public:
	int key;
	KeyNode* next;

	KeyNode(int k) : key(k), next(nullptr) {}
};

class ValueNode {
public:
	std::string value;
	KeyNode* keys;
	ValueNode* next;

	ValueNode(const std::string& val) : value(val), keys(nullptr), next(nullptr) {}
};

class Index {
public:
	Index() : head(nullptr) {}
	//copy constructor
	Index(const Index& other) : head(nullptr) {
		ValueNode* current = other.head;
		while (current != nullptr) {
			ValueNode* newValueNode = new ValueNode(current->value);
			KeyNode* currentKey = current->keys;
			while (currentKey != nullptr) {
				KeyNode* newKeyNode = new KeyNode(currentKey->key);
				newKeyNode->next = newValueNode->keys;
				newValueNode->keys = newKeyNode;
				currentKey = currentKey->next;
			}
			newValueNode->next = head;
			head = newValueNode;
			current = current->next;
		}
	}
	//assignment operator
	Index& operator=(const Index& other) {
		if (this != &other) {
			clear();
			ValueNode* current = other.head;
			while (current != nullptr) {
				ValueNode* newValueNode = new ValueNode(current->value);
				KeyNode* currentKey = current->keys;
				while (currentKey != nullptr) {
					KeyNode* newKeyNode = new KeyNode(currentKey->key);
					newKeyNode->next = newValueNode->keys;
					newValueNode->keys = newKeyNode;
					currentKey = currentKey->next;
				}
				newValueNode->next = head;
				head = newValueNode;
				current = current->next;
			}
		}
		return *this;
	}

	~Index() {
		clear();
	}

	// Method to add a record to the index
	void addRecord(int key, const std::string& value) {
		ValueNode* valueNode = findValueNode(value);
		if (valueNode == nullptr) {
			valueNode = new ValueNode(value);
			valueNode->next = head;
			head = valueNode;
		}

		if (!findKeyNode(key, valueNode->keys)) {
			KeyNode* newKeyNode = new KeyNode(key);
			newKeyNode->next = valueNode->keys;
			valueNode->keys = newKeyNode;
		}
	}

	// Method to remove a record from the index
	void removeRecord(int key, const std::string& value) {
		ValueNode* valueNode = findValueNode(value);
		if (valueNode == nullptr) return;

		KeyNode* current = valueNode->keys;
		KeyNode* prev = nullptr;

		while (current != nullptr && current->key != key) {
			prev = current;
			current = current->next;
		}

		if (current == nullptr) return; // Key not found

		if (prev == nullptr) {
			valueNode->keys = current->next;
		}
		else {
			prev->next = current->next;
		}

		delete current;

		// Remove the ValueNode if it has no keys left
		if (valueNode->keys == nullptr) {
			removeValueNode(valueNode);
		}
	}

	// Method to find keys by value
	void findKeys(const std::string& value) const {
		ValueNode* valueNode = findValueNode(value);
		if (valueNode == nullptr) return;

		KeyNode* current = valueNode->keys;
		while (current != nullptr) {
			std::cout << current->key << " ";
			current = current->next;
		}
		std::cout << std::endl;
	}

	// Method to display the index
	void displayIndex() const {
		ValueNode* currentValue = head;
		while (currentValue != nullptr) {
			std::cout << "Value: " << currentValue->value << " -> Keys: ";
			KeyNode* currentKey = currentValue->keys;
			while (currentKey != nullptr) {
				std::cout << currentKey->key << " ";
				currentKey = currentKey->next;
			}
			std::cout << std::endl;
			currentValue = currentValue->next;
		}
	}

	// Method to get all keys for a value
	int* getKeys(const std::string& value, int& keyCount) const {
		// Find the ValueNode corresponding to the given value
		ValueNode* valueNode = findValueNode(value);
		if (!valueNode) {
			keyCount = 0;
			return nullptr;
		}

		// Count the number of keys associated with the value
		keyCount = 0;
		KeyNode* current = valueNode->keys;
		while (current) {
			keyCount++;
			current = current->next;
		}

		// Allocate memory for the keys array
		int* keys = new int[keyCount];
		int index = 0;

		// Collect the keys into the array
		current = valueNode->keys;
		while (current) {
			keys[index++] = current->key;
			current = current->next;
		}

		return keys;
	}

	// Method to get a value by key
	std::string getValue(int key) const {
		ValueNode* currentValue = head;
		while (currentValue != nullptr) {
			KeyNode* currentKey = currentValue->keys;
			while (currentKey != nullptr) {
				if (currentKey->key == key) {
					return currentValue->value;
				}
				currentKey = currentKey->next;
			}
			currentValue = currentValue->next;
		}
		throw std::out_of_range("Key " + std::to_string(key) + " not found in index");
	}

	//get head
	ValueNode* getHead() const {
		return head;
	}

private:
	ValueNode* head;

	// Helper method to find a ValueNode
	ValueNode* findValueNode(const std::string& value) const {
		ValueNode* current = head;
		while (current != nullptr && current->value != value) {
			current = current->next;
		}
		return current;
	}

	// Helper method to find a KeyNode
	KeyNode* findKeyNode(int key, KeyNode* head) const {
		KeyNode* current = head;
		while (current != nullptr) {
			if (current->key == key) {
				return current;
			}
			current = current->next;
		}
		return nullptr;
	}

	// Helper method to remove a ValueNode
	void removeValueNode(ValueNode* valueNode) {
		if (valueNode == head) {
			head = valueNode->next;
		}
		else {
			ValueNode* prev = head;
			while (prev->next != valueNode) {
				prev = prev->next;
			}
			prev->next = valueNode->next;
		}
		delete valueNode;
	}

	// Clear all nodes in the index
	void clear() {
		while (head != nullptr) {
			ValueNode* nextValue = head->next;
			KeyNode* currentKey = head->keys;
			while (currentKey != nullptr) {
				KeyNode* nextKey = currentKey->next;
				delete currentKey;
				currentKey = nextKey;
			}
			delete head;
			head = nextValue;
		}
	}
};

class IndexManager {
public:
	IndexManager() : head(nullptr) {}
	~IndexManager() {
		while (head != nullptr) {
			IndexNode* temp = head;
			head = head->next;
			delete temp;
		}
	}

	void addIndex(const std::string& indexName, const std::string& tableName, const std::string& columnName, int key, const std::string& value) {
		if (columnName.empty()) {
			std::cerr << "Err: Column name length is 0 for index name in table " << tableName << std::endl;
			return;
		}

		// Find the corresponding index node for the given indexName, tableName, and columnName
		IndexNode* indexNode = findIndexNode(indexName, tableName, columnName);
		if (indexNode == nullptr) {
			// If no index node exists, create a new one
			indexNode = new IndexNode(indexName, tableName, columnName);
			indexNode->next = head;
			head = indexNode;
		}
		// Ensure that the values are written separately for each column index
		indexNode->index.addRecord(key, value); // Add the key-value pair specific to the column
	}

	bool indexExistsByTableName(const std::string& tableName) const {
		return findIndexNodeByTableName(tableName) != nullptr;
	}

	Index* getIndexObjectByTableNameAndColumnName(const std::string& tableName, const std::string& columnName) const {
		IndexNode* indexNode = findIndexNodeByTableNameAndColumnName(tableName, columnName);
		return indexNode ? &indexNode->index : nullptr;
	}
	Index* getIndexObjectByTableName(const std::string& tableName) const {
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->tableName == tableName) {
				return &current->index;
			}
			current = current->next;
		}
		return nullptr; // Return nullptr if no index is found for the given table name
	}
	Index* getIndexObjectByIndexName(const std::string& indexName) const {
		IndexNode* indexNode = findIndexNodeByIndexName(indexName);
		return indexNode ? &indexNode->index : nullptr;
	}

	void removeIndexByTableName(const std::string& tableName) {
		IndexNode* current = head;
		IndexNode* prev = nullptr;

		while (current != nullptr) {
			if (current->tableName == tableName) {
				if (prev == nullptr) {
					head = current->next;
				}
				else {
					prev->next = current->next;
				}
				delete current;
				return;
			}
			prev = current;
			current = current->next;
		}
	}
	void removeIndexByTableNameAndColumnName(const std::string& tableName, const std::string& columnName) {
		IndexNode* current = head;
		IndexNode* prev = nullptr;

		while (current != nullptr) {
			if (current->tableName == tableName && current->columnName == columnName) {
				if (prev == nullptr) {
					head = current->next;
				}
				else {
					prev->next = current->next;
				}
				delete current;
				return;
			}
			prev = current;
			current = current->next;
		}
	}
	void removeIndexByIndexName(const std::string& indexName) {
		IndexNode* current = head;
		IndexNode* prev = nullptr;

		while (current != nullptr) {
			if (current->indexName == indexName) {
				if (prev == nullptr) {
					head = current->next;
				}
				else {
					prev->next = current->next;
				}
				delete current;
				return;
			}
			prev = current;
			current = current->next;
		}
	}

	void displayIndexesFromTable(const std::string& tableName) const {
		IndexNode* current = head;
		bool found = false;
		while (current != nullptr) {
			if (current->tableName == tableName) {
				if (!found) {
					cout << "Indexes from table '" << tableName << "':" << endl;
					found = true;
				}
				cout << " '" << current->indexName << "' on column: '" << current->columnName << "'" << endl;
			}
			current = current->next;
		}
		if (!found) {
			cout << "No indexes found for table '" << tableName << "'." << endl;
		}
	}
	void displayAllIndexes() const {
		IndexNode* current = head;
		if (current == nullptr) {
			cout << "No indexes found in the database." << endl;
			return;
		}

		cout << "List of all indexes:" << endl;
		while (current != nullptr) {
			cout << " '" << current->indexName << "' on column '" << current->columnName << "' from table '" << current->tableName << "'" << endl;
			current = current->next;
		}
	}

	std::string getIndexName(int position) const {
		IndexNode* current = head;
		int count = 0;
		while (current != nullptr) {
			if (count == position) {
				return current->indexName;
			}
			current = current->next;
			count++;
		}
		return ""; // Return an empty string if the position is out of bounds
	}
	std::string getIndexColumnName(const std::string& indexName) const {
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->indexName == indexName) {  // Check for matching indexName
				return current->columnName;  // Return associated column name
			}
			current = current->next;
		}
		return "";  // Return an empty string if no index is found
	}
	std::string getIndexTableName(const std::string& indexName) const {
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->indexName == indexName) {
				return current->tableName;
			}
			current = current->next;
		}
		return ""; // Return an empty string if the index name is not found
	}
	string getIndexNameByTableNameAndColumnName(const string& tableName, const string& columnName) const {
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->tableName == tableName && current->columnName == columnName) {
				return current->indexName;
			}
			current = current->next;
		}
		return ""; // Return an empty string if no index is found
	}

	int getNoIndexes() const {
		int count = 0;
		IndexNode* current = head;
		while (current != nullptr) {
			count++;
			current = current->next;
		}
		return count;
	}
	int getNoIndexesByTableName(const std::string& tableName) const {
		int count = 0;
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->tableName == tableName) {
				count++;
			}
			current = current->next;
		}
		return count;
	}

	int getNoIndexesForTable(const string& tableName) const {
		int count = 0;
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->tableName == tableName) {
				count++;
			}
			current = current->next;
		}
		return count;
	}

private:
	class IndexNode {
	public:
		std::string indexName;
		std::string tableName;
		std::string columnName;
		Index index;
		IndexNode* next;

		IndexNode(const std::string& indexName, const std::string& tableName, const std::string& columnName)
			: indexName(indexName), tableName(tableName), columnName(columnName), next(nullptr) {}
	};

	IndexNode* head;

	IndexNode* findIndexNode(const std::string& indexName, const std::string& tableName, const std::string& columnName) const {
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->indexName == indexName && current->tableName == tableName && current->columnName == columnName) {
				return current;
			}
			current = current->next;
		}
		return nullptr;
	}

	IndexNode* findIndexNodeByTableName(const std::string& tableName) const {
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->tableName == tableName) {
				return current;
			}
			current = current->next;
		}
		return nullptr;
	}

	IndexNode* findIndexNodeByTableNameAndColumnName(const std::string& tableName, const std::string& columnName) const {
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->tableName == tableName && current->columnName == columnName) {
				return current;
			}
			current = current->next;
		}
		return nullptr;
	}

	IndexNode* findIndexNodeByIndexName(const std::string& indexName) const {
		IndexNode* current = head;
		while (current != nullptr) {
			if (current->indexName == indexName) {
				return current;
			}
			current = current->next;
		}
		return nullptr;
	}
};

class Database {          //many tabless
private:
	Table** database = nullptr;
	int noTables = 0;

	TableNames* tableNames = nullptr;
	IndexManager* indexes = nullptr;

	static int selectCount;

	//remove these and use the ValidDataType class
	bool isValidInt(const string& value) {
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
	bool isValidFloat(const string& value) {
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
public:
	//DEFAULT CONSTRUCTOR
	Database() {
		this->database = nullptr;
		this->noTables = 0;
		this->tableNames = nullptr;
		this->tableNames = new TableNames();
		this->indexes = nullptr;
		this->indexes = new IndexManager();
	}
	//DESTRUCTOR
	~Database() {
		for (int i = 0; i < noTables; i++) {
			delete database[i];
		}
		delete[] database;
		delete tableNames;  //i need to delete because i used 'new' in the constructor
		delete indexes;
	}
	//GETTERS
	int getTableIndex(const string& name) const {
		for (int i = 0; i < noTables; i++) {
			if (database[i]->getName() == name) {
				return i;
			}
		}
		return -1;
	}
	//--------------------------------------------------
	void addTableToDatabase(const Table& table) {
		Table** newDatabase = new Table * [noTables + 1];

		for (int i = 0; i < noTables; i++) {
			newDatabase[i] = new Table(*database[i]);     //i am copying pointers from database to newDatabase, not the actual Table objects
		}
		newDatabase[noTables] = new Table(table); // add new table at the end

		for (int i = 0; i < noTables; i++) {
			delete database[i];  //delete the table objects
		}
		delete[] database;
		database = newDatabase;
		noTables++;

		tableNames->addName(table.getName());
	}
	bool tableExists(const string& name) const {
		if (tableNames->nameExists(name)) {
			return true;
		}
		return false;
	}
	void removeTable(int index) {
		if (index < 0 || index >= noTables) {
			cout << endl << "Error: Invalid table index.";
			return;
		}

		Table** newDatabase = new Table * [noTables - 1];
		int tempIndex = 0;

		for (int i = 0; i < noTables; i++) {
			if (i != index) {
				newDatabase[tempIndex++] = database[i];
			}
			else {
				delete database[i];
			}
		}

		delete[] database;
		database = newDatabase;
		noTables--;

		tableNames->removeName(database[index]->getName());
	}
	//--------------------------------------------------
	void createTable(const string& name, Column* columns, int noColumns) {
		//check
		if (tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " already exists.";
			return;
		}

		//create
		Table* newTable = new Table(name, columns, noColumns);

		//add
		addTableToDatabase(*newTable);

		delete newTable;

		cout << endl << "Table " << "'" << name << "'" << " created successfully.";
	}
	void describeTable(const string& name) const {
		if (!tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		int index = getTableIndex(name);
		database[index]->describeTable();
	}
	void dropTable(const string& name, const string& tablesConfigAddress) {
		if (!tableExists(name)) {
			cout << "Error: Table '" << name << "' does not exist." << endl;
			return;
		}

		int indexToRemove = getTableIndex(name);
		if (indexToRemove == -1) return;

		// Check for indexes and remove them
		while (indexes->indexExistsByTableName(name)) {
			indexes->removeIndexByTableName(name);
		}

		// Delete the table
		delete database[indexToRemove];

		// Shift remaining table pointers to the left
		for (int i = indexToRemove; i < noTables - 1; i++) {
			database[i] = database[i + 1];
		}

		noTables--;
		database[noTables] = nullptr; // Set last element to nullptr to avoid dangling pointer

		// Also remove the table name from tableNames
		tableNames->removeName(name);

		// Remove the table file
		string filename = tablesConfigAddress + name + ".bin";
		if (filesystem::exists(filename)) {
			filesystem::remove(filename);
		}

		cout << "Table '" << name << "' dropped successfully." << endl;
	}
	void insertIntoTable(const string& name, const string* values, int noValues) {
		if (!tableExists(name)) {
			cout << "Error: Table '" << name << "' does not exist." << endl;
			return;
		}

		int tableIndex = getTableIndex(name);

		// Get the table at our index
		Table* table = database[tableIndex];

		if (noValues != table->getNoColumns()) {
			cout << "Error: Number of values does not match the number of columns in the table." << endl;
			return;
		}

		// Add the row
		table->addRow(values);
		int newRowId = table->getNoRows() - 1; // Get the ID of the newly added row

		// Update indexes
		for (int i = 0; i < noValues; i++) {
			const string& columnName = table->getColumn(i).getName();
			Index* index = indexes->getIndexObjectByTableNameAndColumnName(name, columnName);
			if (index != nullptr) {
				ColumnType columnType = table->getColumn(i).getType();
				try {
					switch (columnType) {
					case INT:
						index->addRecord(newRowId, to_string(stoi(values[i])));
						break;
					case TEXT:
						index->addRecord(newRowId, values[i]);
						break;
					case FLOAT:
						index->addRecord(newRowId, to_string(stof(values[i])));
						break;
					default:
						cout << "Error: Unsupported column type for indexing." << endl;
						return;
					}
				}
				catch (const invalid_argument& e) {
					cout << "Error: Invalid value for column '" << columnName << "': " << values[i] << endl;
					return;
				}
				catch (const out_of_range& e) {
					cout << "Error: Value out of range for column '" << columnName << "': " << values[i] << endl;
					return;
				}
			}
		}
	}
	void selectALL(const string& name, const string& selectCommandsAddress) const {
		if (!tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		int index = getTableIndex(name);

		//find a unique file name
		string fileName;
		do {
			selectCount++;
			fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
		} while (filesystem::exists(fileName));

		//redirect cout to a file
		ofstream outFile(fileName);
		streambuf* coutBuf = cout.rdbuf(); //save old buffer
		cout.rdbuf(outFile.rdbuf()); //redirect cout to file

		//display table
		database[index]->displayTable();

		//restore cout to its original state
		cout.rdbuf(coutBuf);

		//also display on the screen
		database[index]->displayTable();
	}
	//HELPER FUNCTIONS FOR SELECT COMMANDS
	bool rowMatchesCondition(const Row& row, const Column& conditionColumn, int conditionColumnIndex, const std::string& value) {
		// Assuming the Row class has a method to get the value of a column by index
		std::string rowValue = row.getTextData(conditionColumnIndex);

		// Compare the row value with the given value
		return rowValue == value;
	}
	void printRow(const Row& row, const Table* table, const int* columnIndexes, int noColumns, const int* maxWidth) {
		for (int i = 0; i < noColumns; ++i) {
			int columnIndex = columnIndexes[i];
			std::string value = row.getTextData(columnIndex); // Assuming Row has a getValue method
			std::cout << std::left << std::setw(maxWidth[i]) << value << " | ";
		}
		std::cout << std::endl;
	}
	int* calculateMaxWidths(const Table* table, const std::string* columnNames, int noColumns, const int* columnIndexes) {
		int* maxWidth = new int[noColumns];

		// Initialize maxWidth with the length of the column names
		for (int i = 0; i < noColumns; ++i) {
			maxWidth[i] = columnNames[i].length();
		}

		// Iterate through each row in the table
		for (int i = 0; i < table->getNoRows(); ++i) {
			const Row& row = table->getRow(i);

			// Update maxWidth based on the length of the data in each column
			for (int j = 0; j < noColumns; ++j) {
				int columnIndex = columnIndexes[j];
				std::string value = row.getTextData(columnIndex); // Assuming Row has a getValue method
				if (value.length() > maxWidth[j]) {
					maxWidth[j] = value.length();
				}
			}
		}

		return maxWidth;
	}
	bool validateColumn(const Table* table, const std::string& columnName, int& columnIndex) {
		for (int i = 0; i < table->getNoColumns(); ++i) {
			if (table->getColumn(i).getName() == columnName) {
				columnIndex = i;
				return true;
			}
		}
		std::cout << "Error: Column '" << columnName << "' does not exist in table '" << table->getName() << "'." << std::endl;
		return false;
	}
	bool validateColumns(const Table* table, const std::string* columnNames, int noColumns, int* columnIndexes) {
		for (int i = 0; i < noColumns; ++i) {
			if (!validateColumn(table, columnNames[i], columnIndexes[i])) {
				std::cout << "Error: Column '" << columnNames[i] << "' does not exist in table '" << table->getName() << "'." << std::endl;
				return false;
			}
		}
		return true;
	}
	void printHeader(const std::string* columnNames, const int* maxWidth, int noColumns) {
		for (int i = 0; i < noColumns; ++i) {
			std::cout << std::left << std::setw(maxWidth[i]) << columnNames[i] << " | ";
		}
		std::cout << std::endl;
	}
	void printSeparator(const int* maxWidth, int noColumns) {
		for (int i = 0; i < noColumns; ++i) {
			std::cout << std::string(maxWidth[i], '-') << "-+-";
		}
		std::cout << std::endl;
	}
	void selectWHERE(const string& tableName, const string* columnNames, int noColumns, const string& conditionColumn, const string& value, const string& selectCommandsAddress) {
		selectCount++;

		// Validate table existence
		if (!tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		int tableIndex = getTableIndex(tableName);
		if (tableIndex == -1) {
			cout << "Error: Table '" << tableName << "' not found." << endl;
			return;
		}

		Table* table = database[tableIndex];
		int conditionColumnIndex = table->getColumnIndex(conditionColumn);
		if (conditionColumnIndex == -1) {
			cout << "Error: Column '" << conditionColumn << "' does not exist in table: " << tableName << endl;
			return;
		}

		// Check if there is an index on the condition column
		Index* index = indexes->getIndexObjectByTableNameAndColumnName(tableName, conditionColumn);
		int* rows = nullptr;
		int rowCount = 0;

		if (index != nullptr) {
			// Use the index to get the rows
			cout << "Using index" << endl;
			rows = index->getKeys(value, rowCount);
		}
		else {
			// No index, scan the table
			cout << "Scanning table" << endl;
			rowCount = table->getNoRows();
			rows = new int[rowCount];
			int rowIndex = 0;
			for (int i = 0; i < rowCount; i++) {
				if (table->getRow(i).getTextData(conditionColumnIndex) == value) {
					rows[rowIndex++] = i;
				}
			}
			rowCount = rowIndex;
		}

		// Validate column existence and initialize indexes
		int* columnIndexes = new int[noColumns];
		if (!validateColumns(table, columnNames, noColumns, columnIndexes)) {
			delete[] columnIndexes;
			delete[] rows;
			return;
		}

		// Calculate column widths
		int* maxWidth = calculateMaxWidths(table, columnNames, noColumns, columnIndexes);

		// Create the file path
		string fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
		ofstream outFile(fileName);
		if (!outFile.is_open()) {
			cout << "Error: Could not open file: " << fileName << endl;
			delete[] columnIndexes;
			delete[] maxWidth;
			delete[] rows;
			return;
		}

		// Redirect cout to a file
		streambuf* coutBuf = cout.rdbuf();
		cout.rdbuf(outFile.rdbuf());

		// Print headers and rows to file
		printSeparator(maxWidth, noColumns);
		printHeader(columnNames, maxWidth, noColumns);
		printSeparator(maxWidth, noColumns);

		// Fetch rows based on condition, using index if available
		bool found = false;
		for (int i = 0; i < rowCount; i++) {
			const Row& row = table->getRow(rows[i]);
			printRow(row, table, columnIndexes, noColumns, maxWidth);
			found = true;
		}

		if (!found) {
			cout << "No rows found with " << conditionColumn << " = " << value << endl;
		}

		printSeparator(maxWidth, noColumns);

		// Restore cout
		cout.rdbuf(coutBuf);

		// Print headers and rows to screen
		printSeparator(maxWidth, noColumns);
		printHeader(columnNames, maxWidth, noColumns);
		printSeparator(maxWidth, noColumns);

		found = false;
		for (int i = 0; i < rowCount; i++) {
			const Row& row = table->getRow(rows[i]);
			printRow(row, table, columnIndexes, noColumns, maxWidth);
			found = true;
		}

		if (!found) {
			cout << "No rows found with " << conditionColumn << " = " << value << endl;
		}

		printSeparator(maxWidth, noColumns);

		// Cleanup
		delete[] columnIndexes;
		delete[] maxWidth;
		delete[] rows;
	}
	void selectColumns(const string& tableName, const string* columnNames, int noColumns, const string& selectCommandsAddress) {
		selectCount++;

		// Validate table existence
		if (!tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		// Validate column existence and initialize indexes
		int* columnIndexes = new int[noColumns];
		if (!validateColumns(table, columnNames, noColumns, columnIndexes)) {
			delete[] columnIndexes;
			return;
		}

		// Calculate column widths
		int* maxWidth = calculateMaxWidths(table, columnNames, noColumns, columnIndexes);

		// Create the file path
		string fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
		ofstream outFile(fileName);
		if (!outFile.is_open()) {
			cout << "Error: Could not open file: " << fileName << endl;
			delete[] columnIndexes;
			delete[] maxWidth;
			return;
		}

		// Redirect cout to a file
		streambuf* coutBuf = cout.rdbuf();
		cout.rdbuf(outFile.rdbuf());

		// Print headers and rows to file
		printSeparator(maxWidth, noColumns);
		printHeader(columnNames, maxWidth, noColumns);
		printSeparator(maxWidth, noColumns);

		// Print rows
		for (int i = 0; i < table->getNoRows(); i++) {
			const Row& row = table->getRow(i);
			printRow(row, table, columnIndexes, noColumns, maxWidth);
		}

		printSeparator(maxWidth, noColumns);

		// Restore cout
		cout.rdbuf(coutBuf);

		// Print headers and rows to screen
		printSeparator(maxWidth, noColumns);
		printHeader(columnNames, maxWidth, noColumns);
		printSeparator(maxWidth, noColumns);

		for (int i = 0; i < table->getNoRows(); i++) {
			const Row& row = table->getRow(i);
			printRow(row, table, columnIndexes, noColumns, maxWidth);
		}

		printSeparator(maxWidth, noColumns);

		// Cleanup
		delete[] columnIndexes;
		delete[] maxWidth;
	}
	void createIndex(const string& indexName, const string& columnName, const string& tableName) {
		if (!tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		if (indexes->getIndexObjectByIndexName(indexName) != nullptr) {
			cout << "Error: Index with name '" << indexName << "' already exists." << endl;
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		int columnIndex = table->getColumnIndex(columnName);
		if (columnIndex == -1) {
			cout << "Error: Column '" << columnName << "' does not exist in table '" << tableName << "'." << endl;
			return;
		}

		// Create the index by iterating over all rows in the table
		for (int i = 0; i < table->getNoRows(); i++) {
			Row row = table->getRow(i);
			string value = row.getTextData(columnIndex);
			indexes->addIndex(indexName, tableName, columnName, i, value); // Use row index as the key
		}

		cout << "Index '" << indexName << "' created successfully on column '" << columnName << "' in table '" << tableName << "'." << endl;
	}
	void deleteRowFromTable(const string& name, const string& columnName, const string& value) {
		if (!tableExists(name)) {
			cout << "Error: Table '" << name << "' does not exist." << endl;
			return;
		}

		int tableIndex = getTableIndex(name);
		Table* table = database[tableIndex];

		int columnIndex = table->getColumnIndex(columnName);
		if (columnIndex == -1) {
			cout << "Error: Column '" << columnName << "' does not exist in table '" << name << "'." << endl;
			return;
		}

		ColumnType columnType = table->getColumn(columnName).getType();

		// Check if there is an index on the column
		Index* index = indexes->getIndexObjectByTableNameAndColumnName(name, columnName);
		if (index != nullptr) {
			// Use the index to find the rows
			cout << "Using index" << endl;
			int rowCount = 0;
			int* rows = index->getKeys(value, rowCount);
			if (rowCount > 0) {
				for (int i = 0; i < rowCount; i++) {
					table->deleteRow(rows[i]);
				}
				cout << "Rows deleted successfully." << endl;
				delete[] rows;
				return;
			}
			delete[] rows;
		}
		else {
			// No index, scan the table
			bool rowDeleted = false;
			for (int i = 0; i < table->getNoRows(); i++) {
				Row& row = table->getRow(i);
				switch (columnType) {
				case INT:
					if (row.getIntData(columnIndex) == stoi(value)) {
						table->deleteRow(i);
						rowDeleted = true;
						i--; // Adjust index after deletion
					}
					break;
				case TEXT:
					if (row.getTextData(columnIndex) == value) {
						table->deleteRow(i);
						rowDeleted = true;
						i--; // Adjust index after deletion
					}
					break;
				case FLOAT:
					if (row.getFloatData(columnIndex) == stof(value)) {
						table->deleteRow(i);
						rowDeleted = true;
						i--; // Adjust index after deletion
					}
					break;
				default:
					cout << "Error: Unsupported column type." << endl;
					return;
				}
			}
			if (rowDeleted) {
				cout << "Rows deleted successfully." << endl;
				return;
			}
		}

		cout << "Error: Row with value '" << value << "' not found in column '" << columnName << "'." << endl;
	}
	void updateTable(const string& tableName, const string& setColumnName, const string& setValue, const string& whereColumnName, const string& whereValue) {
		if (!tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		int setColumnIndex;
		int whereColumnIndex;

		bool setColumnHasIndex = indexes->indexExistsByTableName(tableName) && indexes->getIndexColumnName(tableName) == setColumnName;
		bool whereColumnHasIndex = indexes->indexExistsByTableName(tableName) && indexes->getIndexColumnName(tableName) == whereColumnName;

		if (setColumnHasIndex) {
			setColumnIndex = table->getColumnIndex(setColumnName);
			if (!table->columnExistsByIndex(setColumnIndex)) {
				cout << "Error: Column with index '" << setColumnIndex << "' does not exist in table '" << tableName << "'." << endl;
				return;
			}
		}
		else {
			if (!table->columnExists(setColumnName)) {
				cout << "Error: Column '" << setColumnName << "' does not exist in table '" << tableName << "'." << endl;
				return;
			}
			setColumnIndex = table->getColumnIndex(setColumnName);
		}

		if (whereColumnHasIndex) {
			whereColumnIndex = table->getColumnIndex(whereColumnName);
			if (!table->columnExistsByIndex(whereColumnIndex)) {
				cout << "Error: Column with index '" << whereColumnIndex << "' does not exist in table '" << tableName << "'." << endl;
				return;
			}
		}
		else {
			if (!table->columnExists(whereColumnName)) {
				cout << "Error: Column '" << whereColumnName << "' does not exist in table '" << tableName << "'." << endl;
				return;
			}
			whereColumnIndex = table->getColumnIndex(whereColumnName);
		}

		ColumnType setColumnType = table->getColumnType(setColumnIndex);
		int setColumnSize = table->getColumnSize(setColumnIndex);

		try {
			switch (setColumnType) {
			case INT:
				if (!isValidInt(setValue)) {
					throw invalid_argument("Set value must be a valid integer.");
				}
				if (setValue.size() > setColumnSize) {
					throw invalid_argument("Set value exceeds the maximum size for the integer column.");
				}
				break;
			case FLOAT:
				if (!isValidFloat(setValue)) {
					throw invalid_argument("Set value must be a valid float.");
				}
				if (setValue.size() > setColumnSize) {
					throw invalid_argument("Set value exceeds the maximum size for the float column.");
				}
				break;
			case TEXT:
				if (setValue.size() > setColumnSize) {
					throw invalid_argument("Set value exceeds the maximum size for the text column.");
				}
				break;
			default:
				throw invalid_argument("Unsupported column type.");
			}
		}
		catch (const invalid_argument& e) {
			cout << "Error: " << e.what() << endl;
			return;
		}

		int updatedRows = 0;
		if (whereColumnHasIndex) {
			Index* index = indexes->getIndexObjectByTableNameAndColumnName(tableName, whereColumnName);
			int keyCount;
			int* rowIds = index->getKeys(whereValue, keyCount);
			for (int i = 0; i < keyCount; i++) {
				Row& row = table->getRow(rowIds[i]);
				row.setStringData(setColumnIndex, setValue);
				updatedRows++;
			}
			delete[] rowIds;
		}
		else {
			for (int i = 0; i < table->getNoRows(); i++) {
				Row& row = table->getRow(i);
				if (row.getTextData(whereColumnIndex) == whereValue) {
					row.setStringData(setColumnIndex, setValue);
					updatedRows++;
				}
			}
		}

		// Update the index if the set column has an index
		if (setColumnHasIndex) {
			Index* setIndex = indexes->getIndexObjectByTableNameAndColumnName(tableName, setColumnName);
			for (int i = 0; i < table->getNoRows(); i++) {
				Row& row = table->getRow(i);
				if (row.getTextData(whereColumnIndex) == whereValue) {
					setIndex->removeRecord(i, row.getTextData(setColumnIndex));
					setIndex->addRecord(i, setValue);
				}
			}
		}

		cout << "Updated " << updatedRows << " rows in table '" << tableName << "' by setting " << setColumnName << " to '" << setValue << "' where " << whereColumnName << " is '" << whereValue << "'." << endl;
	}
	void alterTableAddColumn(const string& tableName, const Column& newColumn) {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		table->addColumn(newColumn);

		cout << endl << "Column '" << newColumn.getName() << "' added to table '" << tableName << "' successfully.";
	}
	void alterTableDeleteColumn(const string& tableName, const string& columnName) {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		if (!table->columnExists(columnName)) {
			cout << endl << "Error: Column '" << columnName << "' does not exist in table '" << tableName << "'.";
			return;
		}

		int columnIndex = table->getColumnIndex(columnName);
		bool hasIndex = indexes->indexExistsByTableName(tableName) && indexes->getIndexColumnName(tableName) == columnName;

		if (hasIndex) {
			indexes->removeIndexByTableNameAndColumnName(tableName, columnName);
			cout << endl << "Index on column '" << columnName << "' in table '" << tableName << "' removed successfully.";
		}

		table->deleteColumnByIndex(columnIndex);
		cout << endl << "Column '" << columnName << "' deleted from table '" << tableName << "' successfully.";
	}
	void dropIndex(const string& indexName) {
		if (indexes->getIndexObjectByIndexName(indexName) == nullptr) {
			cout << endl << "Error: Index with name '" << indexName << "' does not exist.";
			return;
		}

		indexes->removeIndexByIndexName(indexName);
		cout << endl << "Index with name '" << indexName << "' removed successfully.";
	}
	void showTables() const {
		if (noTables == 0) {
			cout << endl << "No tables found.";
			return;
		}

		cout << endl << "My Tables:";
		for (int i = 0; i < noTables; i++) {
			cout << endl << i + 1 << ". " << database[i]->getName();
		}
	}
	void showIndexFromTable(const string& tableName) const {
		if (!tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		indexes->displayIndexesFromTable(tableName);
	}
	void showIndexFromAll() const {
		indexes->displayAllIndexes();
	}
	//ALTA CLASA PT ASTEA 2
	void printHelpMenu() {
		string commands[] = {
			"CREATE TABLE",
			"DESCRIBE TABLE",
			"DROP TABLE",
			"SELECT ALL",
			"INSERT INTO VALUES",
			"DELETE FROM WHERE",
			"SELECT WHERE",
			"SELECT",
			"UPDATE TABLE",
			"ALTER TABLE ADD COLUMN",
			"ALTER TABLE DROP COLUMN",
			"CREATE INDEX",
			"DROP INDEX",
			"SHOW TABLES",
			"SHOW INDEX FROM",
			"SHOW INDEX FROM ALL",
			"clear",
			"exit"
		};

		string descriptions[] = {
			" Create a new table with specified columns and constraints.",
			" Show the structure of an existing table.",
			" Delete an existing table from the database.",
			" Select all rows from a table.",
			" Insert a new row into a table.",
			" Delete rows from a table based on a condition.",
			" Select specific columns from a table based on a condition.",
			" Select specific columns from a table without conditions.",
			" Update existing rows in a table based on a condition.",
			"Add a new column to an existing table.",
			"Drop an existing column from a table.",
			"Create an index on a specified column of a table.",
			"Drop an existing index from a table.",
			"Show all the tables in the database.",
			"Show indexes associated with a specific table.",
			"Show all indexes in the database.",
			"Clear the console screen.",
			"Exit the program."
		};

		cout << "Available Commands:\n";
		cout << "----------------------------------\n";

		for (int i = 0; i < 18; ++i) {
			cout << i + 1 << ". " << commands[i];
			int spaces = 30 - commands[i].length();
			for (int j = 0; j < spaces; ++j) {
				cout << " "; // fill the space to align descriptions
			}
			cout << descriptions[i] << "\n\n";
		}

		cout << "Type 'help 2' to see each command's syntax.\n";
	}
	void printSyntaxMenu() {
		std::cout << "========================== HELP MENU ==========================\n";
		std::cout << "1. CREATE TABLE table_name (column_name, column_type, size, default_value)\n";
		std::cout << "   - Creates a new table with the specified columns.\n";
		std::cout << "\n";
		std::cout << "2. DESCRIBE table_name\n";
		std::cout << "   - Displays the structure of the specified table.\n";
		std::cout << "\n";
		std::cout << "3. DROP TABLE table_name\n";
		std::cout << "   - Deletes the specified table.\n";
		std::cout << "\n";
		std::cout << "4. SELECT ALL FROM table_name\n";
		std::cout << "   - Retrieves all rows from the specified table.\n";
		std::cout << "\n";
		std::cout << "5. INSERT INTO table_name VALUES (value1, value2, ...)\n";
		std::cout << "   - Inserts new values into the specified table.\n";
		std::cout << "\n";
		std::cout << "6. DELETE FROM table_name WHERE condition\n";
		std::cout << "   - Deletes rows from the table matching the condition.\n";
		std::cout << "\n";
		std::cout << "7. SELECT column1, column2, ... FROM table_name WHERE condition\n";
		std::cout << "   - Retrieves specific columns from the table based on the condition.\n";
		std::cout << "\n";
		std::cout << "8. SELECT column1, column2, ... FROM table_name\n";
		std::cout << "   - Retrieves specific columns from the table.\n";
		std::cout << "\n";
		std::cout << "9. UPDATE table_name SET column1 = value1, column2 = value2, ... WHERE condition\n";
		std::cout << "   - Updates rows in the table that match the condition.\n";
		std::cout << "\n";
		std::cout << "10. ALTER TABLE table_name ADD (column_name, column_type, size, default_value)\n";
		std::cout << "    - Adds a new column to the specified table.\n";
		std::cout << "\n";
		std::cout << "11. ALTER TABLE table_name DROP COLUMN column_name\n";
		std::cout << "    - Removes a column from the specified table.\n";
		std::cout << "\n";
		std::cout << "12. CREATE INDEX index_name ON table_name (column_name)\n";
		std::cout << "    - Creates an index on the specified column in the table.\n";
		std::cout << "\n";
		std::cout << "13. DROP INDEX index_name\n";
		std::cout << "    - Deletes the specified index.\n";
		std::cout << "\n";
		std::cout << "14. SHOW TABLES\n";
		std::cout << "    - Lists all the tables in the database.\n";
		std::cout << "\n";
		std::cout << "15. SHOW INDEX FROM table_name\n";
		std::cout << "    - Displays all indices on the specified table.\n";
		std::cout << "\n";
		std::cout << "16. SHOW INDEX FROM ALL\n";
		std::cout << "    - Displays all indices across all tables.\n";
		std::cout << "\n";
		std::cout << "17. clear\n";
		std::cout << "    - Clears the console screen.\n";
		std::cout << "\n";
		std::cout << "18. exit\n";
		std::cout << "    - Exits the program.\n";
		std::cout << "================================================================\n";
	}
	//--------------------------------------------------
	void writeTableStructure(ofstream& outFile, const Table& table) const {
		int nameLength = table.getName().length();
		outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
		outFile.write(table.getName().c_str(), nameLength);

		int noColumns = table.getNoColumns();
		outFile.write(reinterpret_cast<const char*>(&noColumns), sizeof(noColumns));
		for (int j = 0; j < noColumns; ++j) {
			const Column& column = table.getColumn(j);
			const string& columnName = column.getName();
			int columnNameLength = columnName.length();
			outFile.write(reinterpret_cast<const char*>(&columnNameLength), sizeof(columnNameLength));
			outFile.write(columnName.c_str(), columnNameLength);

			ColumnType columnType = column.getType();
			outFile.write(reinterpret_cast<const char*>(&columnType), sizeof(columnType));

			int columnSize = column.getSize();
			outFile.write(reinterpret_cast<const char*>(&columnSize), sizeof(columnSize));

			const string& defaultValue = column.getDefaultValue();
			int defaultValueLength = defaultValue.length();
			outFile.write(reinterpret_cast<const char*>(&defaultValueLength), sizeof(defaultValueLength));
			outFile.write(defaultValue.c_str(), defaultValueLength);

			bool unique = column.isUnique();
			outFile.write(reinterpret_cast<const char*>(&unique), sizeof(unique));
		}
	}
	void writeTableData(ofstream& outFile, const Table& table) const {
		int noRows = table.getNoRows();
		outFile.write(reinterpret_cast<const char*>(&noRows), sizeof(noRows));
		for (int j = 0; j < noRows; ++j) {
			const Row& row = table.getRow(j);
			for (int k = 0; k < table.getNoColumns(); ++k) {
				const string& value = row.getTextData(k);
				int valueLength = value.length();
				outFile.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
				outFile.write(value.c_str(), valueLength);
			}
		}
	}
	void writeTableIndexes(ofstream& outFile, const Table& table) const {
		int noIndexes = indexes->getNoIndexesForTable(table.getName());
		cout << "Saving " << noIndexes << " indexes for table: " << table.getName() << endl;
		outFile.write(reinterpret_cast<const char*>(&noIndexes), sizeof(noIndexes));

		for (int j = 0; j < noIndexes; ++j) {
			cout << "Processing index " << j + 1 << " for table: " << table.getName() << endl;

			Index* index = indexes->getIndexObjectByTableName(table.getName());
			if (index != nullptr) {
				writeIndexMetadata(outFile, j);
				writeIndexValues(outFile, index);
			}
			else {
				cout << "No index object found for table: " << table.getName() << endl;
			}
		}
	}
	void writeIndexMetadata(ofstream& outFile, int indexPosition) const {
		const string& indexName = indexes->getIndexName(indexPosition);
		cout << "Index name: " << indexName << endl;
		int indexNameLength = indexName.length();
		outFile.write(reinterpret_cast<const char*>(&indexNameLength), sizeof(indexNameLength));
		outFile.write(indexName.c_str(), indexNameLength);

		const string& columnName = indexes->getIndexColumnName(indexName);
		cout << "Associated column name: " << columnName << endl;
		int columnNameLength = columnName.length();
		outFile.write(reinterpret_cast<const char*>(&columnNameLength), sizeof(columnNameLength));
		outFile.write(columnName.c_str(), columnNameLength);

		const string& tableName = indexes->getIndexTableName(indexName);
		cout << "Associated table name: " << tableName << endl;
		int tableNameLength = tableName.length();
		outFile.write(reinterpret_cast<const char*>(&tableNameLength), sizeof(tableNameLength));
		outFile.write(tableName.c_str(), tableNameLength);
	}
	void writeIndexValues(ofstream& outFile, Index* index) const {
		ValueNode* currentValue = index->getHead();
		int valueCount = 0;
		while (currentValue != nullptr) {
			cout << "Writing value: " << currentValue->value << endl;
			int valueLength = currentValue->value.length();
			outFile.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
			outFile.write(currentValue->value.c_str(), valueLength);

			KeyNode* currentKey = currentValue->keys;
			int keyCount = 0;
			while (currentKey != nullptr) {
				cout << "Writing key: " << currentKey->key << " for value: " << currentValue->value << endl;
				outFile.write(reinterpret_cast<const char*>(&currentKey->key), sizeof(currentKey->key));
				currentKey = currentKey->next;
				++keyCount;
			}
			cout << "Total keys written for value \"" << currentValue->value << "\": " << keyCount << endl;

			currentValue = currentValue->next;
			++valueCount;
		}
		cout << "Total values written for index: " << valueCount << endl;
	}

	void saveDatabase(const string& tablesConfigAddress) const {
		for (int i = 0; i < noTables; i++) {
			const Table& table = *database[i]; string filename = tablesConfigAddress + table.getName() + ".bin"; ofstream outFile(filename, ios::binary); if (!outFile) { cout << endl << "Error: Could not create file " << filename; continue; }
			// Write the table structure to the file
			int nameLength = table.getName().length();
			outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
			outFile.write(table.getName().c_str(), nameLength);

			int noColumns = table.getNoColumns();
			outFile.write(reinterpret_cast<const char*>(&noColumns), sizeof(noColumns));
			for (int j = 0; j < noColumns; ++j) {
				const Column& column = table.getColumn(j);
				const string& columnName = column.getName();
				int columnNameLength = columnName.length();
				outFile.write(reinterpret_cast<const char*>(&columnNameLength), sizeof(columnNameLength));
				outFile.write(columnName.c_str(), columnNameLength);

				ColumnType columnType = column.getType();
				outFile.write(reinterpret_cast<const char*>(&columnType), sizeof(columnType));

				int columnSize = column.getSize();
				outFile.write(reinterpret_cast<const char*>(&columnSize), sizeof(columnSize));

				const string& defaultValue = column.getDefaultValue();
				int defaultValueLength = defaultValue.length();
				outFile.write(reinterpret_cast<const char*>(&defaultValueLength), sizeof(defaultValueLength));
				outFile.write(defaultValue.c_str(), defaultValueLength);

				bool unique = column.isUnique();
				outFile.write(reinterpret_cast<const char*>(&unique), sizeof(unique));
			}

			// Write the table data to the file
			int noRows = table.getNoRows();
			outFile.write(reinterpret_cast<const char*>(&noRows), sizeof(noRows));
			for (int j = 0; j < noRows; ++j) {
				const Row& row = table.getRow(j);
				for (int k = 0; k < noColumns; ++k) {
					const string& value = row.getTextData(k);
					int valueLength = value.length();
					outFile.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
					outFile.write(value.c_str(), valueLength);
				}
			}

			// Write the index metadata for the table
			int noIndexes = indexes->getNoIndexesForTable(table.getName());
			cout << "Saving " << noIndexes << " indexes for table: " << table.getName() << endl;
			outFile.write(reinterpret_cast<const char*>(&noIndexes), sizeof(noIndexes));

			for (int j = 0; j < noIndexes; ++j) {
				cout << "Processing index " << j + 1 << " for table: " << table.getName() << endl;

				// Retrieve the index name from the IndexManager for the specific table and column
				const string& indexName = indexes->getIndexNameByTableNameAndColumnName(table.getName(), table.getColumn(j).getName());
				Index* index = indexes->getIndexObjectByTableNameAndColumnName(table.getName(), table.getColumn(j).getName());

				if (index != nullptr) {
					// Write index metadata
					cout << "Index name: " << indexName << endl;
					int indexNameLength = indexName.length();
					outFile.write(reinterpret_cast<const char*>(&indexNameLength), sizeof(indexNameLength));
					outFile.write(indexName.c_str(), indexNameLength);

					const string& columnName = table.getColumn(j).getName(); // Using the column from the table
					cout << "Associated column name: " << columnName << endl;
					int columnNameLength = columnName.length();
					outFile.write(reinterpret_cast<const char*>(&columnNameLength), sizeof(columnNameLength));
					outFile.write(columnName.c_str(), columnNameLength);

					const string& tableName = table.getName(); // Using the table name directly
					cout << "Associated table name: " << tableName << endl;
					int tableNameLength = tableName.length();
					outFile.write(reinterpret_cast<const char*>(&tableNameLength), sizeof(tableNameLength));
					outFile.write(tableName.c_str(), tableNameLength);

					// Write index values and associated keys
					ValueNode* currentValue = index->getHead();
					int valueCount = 0;
					while (currentValue != nullptr) {
						cout << "Writing value: " << currentValue->value << endl;
						int valueLength = currentValue->value.length();
						outFile.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
						outFile.write(currentValue->value.c_str(), valueLength);

						KeyNode* currentKey = currentValue->keys;
						int keyCount = 0;
						while (currentKey != nullptr) {
							cout << "Writing key: " << currentKey->key << " for value: " << currentValue->value << endl;
							outFile.write(reinterpret_cast<const char*>(&currentKey->key), sizeof(currentKey->key));
							currentKey = currentKey->next;
							++keyCount;
						}
						cout << "Total keys written for value \"" << currentValue->value << "\": " << keyCount << endl;

						currentValue = currentValue->next;
						++valueCount;
					}
					cout << "Total values written for index \"" << indexName << "\": " << valueCount << endl;
				}
				else {
					cout << "No index object found for table: " << table.getName() << endl;
				}
			}

			outFile.close();
		}

		cout << endl << "Database saved successfully." << endl;
	}

	void loadDatabase(const string& tablesConfigAddress, const string& selectCommandsAddress) {
		// Clear the current database
		cout << "Clearing current database..." << endl;
		for (int i = 0; i < noTables; ++i) {
			cout << "Removing table " << i << endl;
			removeTable(0);
		}

		// Clear the contents of the select_commands folder
		cout << "Clearing select_commands folder..." << endl;
		for (const auto& entry : filesystem::directory_iterator(selectCommandsAddress)) {
			cout << "Removing: " << entry.path() << endl;
			filesystem::remove_all(entry.path());
		}

		// Iterate over the files in the directory where the tables are saved
		cout << "Loading tables from: " << tablesConfigAddress << endl;
		for (const auto& entry : filesystem::directory_iterator(tablesConfigAddress)) {
			if (entry.path().extension() == ".bin") {
				cout << "Opening file: " << entry.path() << endl;
				ifstream inFile(entry.path(), ios::binary);
				if (!inFile) {
					cout << "Error: Could not open file " << entry.path() << endl;
					continue;
				}

				// Read the table structure from the file
				int nameLength;
				inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
				string tableName(nameLength, ' ');
				inFile.read(&tableName[0], nameLength);

				int noColumns;
				inFile.read(reinterpret_cast<char*>(&noColumns), sizeof(noColumns));
				Column* columns = new Column[noColumns];
				cout << "Table name: " << tableName << ", Number of columns: " << noColumns << endl;
				for (int j = 0; j < noColumns; ++j) {
					int columnNameLength;
					inFile.read(reinterpret_cast<char*>(&columnNameLength), sizeof(columnNameLength));
					string columnName(columnNameLength, ' ');
					inFile.read(&columnName[0], columnNameLength);

					ColumnType columnType;
					inFile.read(reinterpret_cast<char*>(&columnType), sizeof(columnType));

					int columnSize;
					inFile.read(reinterpret_cast<char*>(&columnSize), sizeof(columnSize));

					int defaultValueLength;
					inFile.read(reinterpret_cast<char*>(&defaultValueLength), sizeof(defaultValueLength));
					string defaultValue(defaultValueLength, ' ');
					inFile.read(&defaultValue[0], defaultValueLength);

					bool unique;
					inFile.read(reinterpret_cast<char*>(&unique), sizeof(unique));

					try {
						columns[j] = Column(columnName, columnType, columnSize, defaultValue, unique);
						cout << "Loaded column: " << columnName << " of type " << columnType << endl;
					}
					catch (const invalid_argument& e) {
						cout << endl << e.what();
					}
				}

				Table table(tableName, columns, noColumns);
				delete[] columns;

				// Read the table data from the file
				int noRows;
				inFile.read(reinterpret_cast<char*>(&noRows), sizeof(noRows));
				cout << "Number of rows: " << noRows << endl;
				for (int j = 0; j < noRows; ++j) {
					string* values = new string[noColumns];
					for (int k = 0; k < noColumns; ++k) {
						int valueLength;
						inFile.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
						string value(valueLength, ' ');
						inFile.read(&value[0], valueLength);
						values[k] = value;
						cout << "Loaded value: " << value << " for column " << k << endl;
					}
					table.addRowWithoutPrintMessage(values);
					delete[] values;
				}

				// Read the index metadata for the table
				int noIndexes;
				inFile.read(reinterpret_cast<char*>(&noIndexes), sizeof(noIndexes));
				cout << "Loaded " << noIndexes << " indexes for table: " << tableName << endl;

				for (int j = 0; j < noIndexes; ++j) {
					int indexNameLength;
					inFile.read(reinterpret_cast<char*>(&indexNameLength), sizeof(indexNameLength));

					if (indexNameLength > 0) {
						string indexName(indexNameLength, ' ');
						inFile.read(&indexName[0], indexNameLength);
						cout << "Loaded index name: " << indexName << endl;

						int columnNameLength;
						inFile.read(reinterpret_cast<char*>(&columnNameLength), sizeof(columnNameLength));
						string columnName(columnNameLength, ' ');
						inFile.read(&columnName[0], columnNameLength);
						cout << "Associated column name: " << columnName << endl;

						int tableNameLength;
						inFile.read(reinterpret_cast<char*>(&tableNameLength), sizeof(tableNameLength));
						string indexTableName(tableNameLength, ' ');
						inFile.read(&indexTableName[0], tableNameLength);
						cout << "Associated table name: " << indexTableName << endl;

						// Add index records
						int valueCount;
						inFile.read(reinterpret_cast<char*>(&valueCount), sizeof(valueCount));
						cout << "Index " << indexName << " has " << valueCount << " values." << endl;

						for (int v = 0; v < valueCount; ++v) {
							int valueLength;
							inFile.read(reinterpret_cast<char*>(&valueLength), sizeof(valueLength));
							string value(valueLength, ' ');
							inFile.read(&value[0], valueLength);
							cout << "Loaded index value: " << value << endl;

							int keyCount;
							inFile.read(reinterpret_cast<char*>(&keyCount), sizeof(keyCount));
							cout << "Index value " << value << " has " << keyCount << " associated keys." << endl;

							for (int k = 0; k < keyCount; ++k) {
								int key;
								inFile.read(reinterpret_cast<char*>(&key), sizeof(key));
								cout << "Loaded key: " << key << " for value: " << value << endl;

								// Add the key-value pair to the index
								//addIndex(indexName, indexTableName, columnName, key, value);
							}
						}
					}
					else {
						cout << "Error: Invalid index name length (" << indexNameLength << "). Skipping index." << endl;
					}
				}

				inFile.close();
			}
		}

		cout << endl << "Database loaded successfully." << endl;
	}
};

int Database::selectCount = 0;

class Commands { //doar daca e comanda scrisa cum trebuie
	//VERIFIC DACA ARE FORMATU BUN LA COMANDA SI GENEREAZA UN OBIECT SPECIFIC COMENZII INTRODUSE
	//NIMIC LA MEMBRI
private:
	Database* db = nullptr;
	//HELPER FUNCTIONS----------------------------------
	ColumnType parseColumnType(const string& type) {
		//convert string to ColumnType for the column constructor
		if (type == "INT") return INT;
		if (type == "TEXT") return TEXT;
		if (type == "FLOAT") return FLOAT;
		if (type == "BOOLEAN") return BOOLEAN;
		if (type == "DATE") return DATE;
		throw invalid_argument("Invalid column type: " + type);
	}
	bool parseUnique(const string& unique) {
		//convert string to bool for the column constructor
		if (unique == "UNIQUE") return true;
		return false;
	}
	bool containsWord(const string& command, const string& word) {
		return command.find(word) != string::npos; //if not equak to "not found" then return true
	}
	void trim(string& str) {
		//find the first non-space character
		size_t start = str.find_first_not_of(" ");
		//find the last non-space character
		size_t end = str.find_last_not_of(" ");

		//if the string is only spaces set it to an empty string
		if (start == string::npos) {
			str = "";
		}
		else {
			//make the trimmed string
			str = str.substr(start, end - start + 1);
		}
	}
	void splitCommand(const string& command, const string& delimiter, string*& tokens, int& tokenCount) {
		string commandCopy = command;
		trim(commandCopy);
		tokenCount = 0;
		size_t pos = 0;
		string token;
		while ((pos = commandCopy.find(delimiter)) != string::npos) {
			token = commandCopy.substr(0, pos);
			if (!token.empty()) {
				tokenCount++;
			}
			commandCopy.erase(0, pos + delimiter.length());
		}
		if (!commandCopy.empty()) {
			tokenCount++;
		}

		tokens = new string[tokenCount];
		commandCopy = command;
		pos = 0;
		int i = 0;
		while ((pos = commandCopy.find(delimiter)) != string::npos) {
			token = commandCopy.substr(0, pos);
			if (!token.empty()) {
				tokens[i++] = token;
			}
			commandCopy.erase(0, pos + delimiter.length());
		}
		if (!commandCopy.empty()) {
			tokens[i] = commandCopy;
		}
	}
private:
	//every command is in a try block to avoid crashing the program and printing error messages instead
	void stringCommandCreateTable(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if command starts with "CREATE TABLE "
			if (commandCopy.find("CREATE TABLE ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of the first '('
			size_t pos = commandCopy.find("(");
			if (pos == string::npos) {
				cout << endl << "Invalid command format. Missing '('.";
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(13, pos - 13);  // 13 is the length of "CREATE TABLE "
			trim(tableName);

			//check if the table name is empty
			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//check if the table name contains spaces
			if (tableName.find(' ') != string::npos) {
				cout << endl << "Invalid command format. Table name cannot contain spaces.";
				return;
			}

			//find the position of the last ')'
			size_t endPos = commandCopy.find_last_of(")");
			if (endPos == string::npos) {
				cout << endl << "Invalid command format. Missing ')'.";
				return;
			}

			//get the columns part
			string columnsPart = commandCopy.substr(pos + 1, endPos - pos - 1);  // from after '(' to before ')'
			trim(columnsPart);

			if (columnsPart.empty()) {
				cout << endl << "Invalid command format. Columns cannot be empty.";
				return;
			}

			//count the number of columns
			int noColumns = 0;
			size_t start = 0;
			size_t end = 0;
			while ((end = columnsPart.find("),", start)) != string::npos) {
				noColumns++;
				start = end + 2;
				while (start < columnsPart.length() && isspace(columnsPart[start])) {
					start++;
				}
				if (start < columnsPart.length() && columnsPart[start] == '(') {
					start++;
				}
			}
			noColumns++; //increment for the last column

			if (noColumns == 0) {
				cout << endl << "Invalid command format. No columns specified.";
				return;
			}

			//split the columns part into individual columns
			string* columns = new string[noColumns];
			start = 0;
			int colIndex = 0;
			while ((end = columnsPart.find("),", start)) != string::npos) {
				string column = columnsPart.substr(start, end - start + 1);
				trim(column);
				columns[colIndex++] = column;
				start = end + 2;
				while (start < columnsPart.length() && isspace(columnsPart[start])) {
					start++;
				}
				if (start < columnsPart.length() && columnsPart[start] == '(') {
					start++;
				}
			}
			string lastColumn = columnsPart.substr(start);
			trim(lastColumn);
			columns[colIndex] = lastColumn;

			Column* tableColumns = new Column[noColumns];
			for (int i = 0; i < noColumns; i++) {
				string column = columns[i];
				trim(column);

				//remove leading and trailing parentheses
				if (!column.empty() && column.front() == '(') column.erase(0, 1);
				if (!column.empty() && column.back() == ')') column.pop_back();

				//split the column into individual parts
				string* columnParts = nullptr;
				int noParts = 0;
				splitCommand(column, ",", columnParts, noParts);

				if (noParts < 4 || noParts > 5) {
					cout << endl << "Invalid command format. Invalid column format.";
					delete[] columnParts;
					delete[] columns;
					delete[] tableColumns;
					return;
				}

				//get the column name
				string columnName = columnParts[0];
				trim(columnName);

				if (columnName.empty()) {
					cout << endl << "Invalid command format. Column name cannot be empty.";
					delete[] columnParts;
					delete[] columns;
					delete[] tableColumns;
					return;
				}

				//get the column type
				string columnType = columnParts[1];
				trim(columnType);

				ColumnType type;
				try {
					type = parseColumnType(columnType);
				}
				catch (const invalid_argument& e) {
					cout << endl << e.what();
					delete[] columnParts;
					delete[] columns;
					delete[] tableColumns;
					return;
				}

				//size
				string columnSize = columnParts[2];
				trim(columnSize);

				if (columnSize.empty()) {
					cout << endl << "Invalid command format. Column size cannot be empty.";
					delete[] columnParts;
					delete[] columns;
					delete[] tableColumns;
					return;
				}

				//default value
				string columnDefaultValue = columnParts[3];
				trim(columnDefaultValue);

				//check if column is unique
				bool unique = false;
				if (noParts == 5) {
					string uniquePart = columnParts[4];
					trim(uniquePart);
					if (uniquePart == "UNIQUE") {
						unique = true;
					}
					else {
						cout << endl << "Invalid command format. Invalid column format.";
						delete[] columnParts;
						delete[] columns;
						delete[] tableColumns;
						return;
					}
				}

				//create the column
				tableColumns[i] = Column(columnName, type, stoi(columnSize), columnDefaultValue, unique);
			}

			db->createTable(tableName, tableColumns, noColumns);

			delete[] columns;
			delete[] tableColumns;
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandSelectAll(const string& command, const string& selectCommandsAddress) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "SELECT ALL FROM "
			if (commandCopy.find("SELECT ALL FROM ") != 0) {    //if it does not start with "SELECT ALL FROM " with space
				//or without a space after "FROM"
				if (commandCopy.find("SELECT ALL FROM") == 0) {  //if it starts with "SELECT ALL FROM" without a space after
					cout << endl << "Invalid command format.";
				}
				else {
					cout << endl << "Invalid command format.";
				}
				return;
			}

			//find the position of "FROM " and make sure there is a space after it
			size_t pos = commandCopy.find("FROM ") + 5;  // 5 is the length of "FROM " with the space
			if (pos >= commandCopy.length()) {
				cout << endl << "Invalid command format. Too few arguments.";
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(pos);
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Too few arguments.";
				return;
			}

			//check for extra arguments
			size_t extraArgsPos = tableName.find(' ');
			if (extraArgsPos != string::npos) {
				cout << endl << "Invalid command format. Too many arguments.";
				return;
			}

			db->selectALL(tableName, selectCommandsAddress);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandDropTable(const string& command, const string& tablesConfigAddress) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "DROP TABLE "
			if (commandCopy.find("DROP TABLE ") != 0) {
				if (commandCopy.find("DROP TABLE") == 0) {
					cout << endl << "Invalid command format.";
				}
				else {
					cout << endl << "Invalid command format.";
				}
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(11);  // 11 is the length of "DROP TABLE " with a space after
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Too few arguments.";
				return;
			}

			//check for extra arguments
			size_t extraArgsPos = tableName.find(' ');
			if (extraArgsPos != string::npos) {
				cout << endl << "Invalid command format. Too many arguments.";
				return;
			}

			db->dropTable(tableName, tablesConfigAddress);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandDescribe(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "DESCRIBE "
			if (commandCopy.find("DESCRIBE ") != 0) {
				if (commandCopy.find("DESCRIBE") == 0) {
					cout << endl << "Invalid command format.";
				}
				else {
					cout << endl << "Invalid command format.";
				}
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(9);  // 9 is the length of "DESCRIBE " with a space after
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Too few arguments.";
				return;
			}

			//check for extra arguments
			size_t extraArgsPos = tableName.find(' ');
			if (extraArgsPos != string::npos) {
				cout << endl << "Invalid command format. Too many arguments.";
				return;
			}

			db->describeTable(tableName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandinsertIntoValues(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "INSERT INTO "
			if (commandCopy.find("INSERT INTO ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of "VALUES "
			size_t pos = commandCopy.find("VALUES ");
			if (pos == string::npos) {
				cout << endl << "Invalid command format. Missing 'VALUES'.";
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(12, pos - 12);  // 12 is the length of "INSERT INTO " with a space
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//find the position of the first '('
			size_t startPos = commandCopy.find("(");
			if (startPos == string::npos) {
				cout << endl << "Invalid command format. Missing '('.";
				return;
			}

			//find the position of the last ')'
			size_t endPos = commandCopy.find_last_of(")");
			if (endPos == string::npos) {
				cout << endl << "Invalid command format. Missing ')'.";
				return;
			}

			//get the values part
			string valuesPart = commandCopy.substr(startPos + 1, endPos - startPos - 1);  // from after '(' to before ')'
			trim(valuesPart);

			if (valuesPart.empty()) {
				cout << endl << "Invalid command format. Values cannot be empty.";
				return;
			}

			//split the values part into individual values
			string* values = nullptr;
			int noValues = 0;
			splitCommand(valuesPart, ",", values, noValues);

			if (noValues == 0) {
				cout << endl << "Invalid command format. No values specified.";
				return;
			}

			//trim each value individually
			for (int i = 0; i < noValues; i++) {
				trim(values[i]);
			}

			db->insertIntoTable(tableName, values, noValues);

			delete[] values;
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandDeleteFromWhere(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "DELETE FROM "
			if (commandCopy.find("DELETE FROM ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of "WHERE "
			size_t pos = commandCopy.find("WHERE ");
			if (pos == string::npos) {
				cout << endl << "Invalid command format. Missing 'WHERE'.";
				return;
			}

			//make sure there is a space before "WHERE "
			if (commandCopy[pos - 1] != ' ') {
				cout << endl << "Invalid command format. Missing space before 'WHERE'.";
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(12, pos - 13);  // 12 is the length of "DELETE FROM " with a space
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//get the where part
			string wherePart = commandCopy.substr(pos + 6);  // 6 is the length of "WHERE " with a space
			trim(wherePart);

			if (wherePart.empty()) {
				cout << endl << "Invalid command format. Where part cannot be empty.";
				return;
			}

			//split the where part into individual parts
			string* whereParts = nullptr;
			int noParts = 0;
			splitCommand(wherePart, "=", whereParts, noParts);

			if (noParts != 2) {
				cout << endl << "Invalid command format. Invalid where part.";
				delete[] whereParts;
				return;
			}

			//get the column name
			string columnName = whereParts[0];
			trim(columnName);

			if (columnName.empty()) {
				cout << endl << "Invalid command format. Column name cannot be empty.";
				delete[] whereParts;
				return;
			}

			//get the value
			string value = whereParts[1];
			trim(value);

			if (value.empty()) {
				cout << endl << "Invalid command format. Value cannot be empty.";
				delete[] whereParts;
				return;
			}

			db->deleteRowFromTable(tableName, columnName, value);

			delete[] whereParts;
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandSelectColumns(const string& command, const string& selectCommandsAddress) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "SELECT "
			if (commandCopy.find("SELECT ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of "FROM "
			size_t pos = commandCopy.find("FROM ");
			if (pos == string::npos) {
				cout << endl << "Invalid command format. Missing 'FROM'.";
				return;
			}

			//make sure there is a space before "FROM "
			if (commandCopy[pos - 1] != ' ') {
				cout << endl << "Invalid command format. Missing space before 'FROM'.";
				return;
			}

			//get the columns part
			string columnsPart = commandCopy.substr(7, pos - 8);  // 7 is the length of "SELECT " with a space
			trim(columnsPart);

			if (columnsPart.empty()) {
				cout << endl << "Invalid command format. Columns cannot be empty.";
				return;
			}

			//count the number of columns
			int noColumns = 1;
			for (char c : columnsPart) {
				if (c == ',') {
					noColumns++;
				}
			}

			//split the columns part into individual columns
			string* columns = new string[noColumns];
			size_t start = 0;
			size_t end = columnsPart.find(',');
			int index = 0;

			while (end != string::npos) {
				columns[index] = columnsPart.substr(start, end - start);
				trim(columns[index]);
				start = end + 1;
				end = columnsPart.find(',', start);
				index++;
			}

			//add the last column
			columns[index] = columnsPart.substr(start);
			trim(columns[index]);

			if (noColumns == 0) {
				cout << endl << "Invalid command format. No columns specified.";
				delete[] columns;
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(pos + 5);  // 5 is the length of "FROM " with a space
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				delete[] columns;
				return;
			}

			//check for extra arguments
			size_t extraArgsPos = tableName.find(' ');
			if (extraArgsPos != string::npos) {
				cout << endl << "Invalid command format. Too many arguments.";
				delete[] columns;
				return;
			}

			db->selectColumns(tableName, columns, noColumns, selectCommandsAddress);

			delete[] columns;
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandSelectWhere(const string& command, const string& selectCommandsAddress) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "SELECT "
			if (commandCopy.find("SELECT ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of "FROM "
			size_t fromPos = commandCopy.find("FROM ");
			if (fromPos == string::npos) {
				cout << endl << "Invalid command format. Missing 'FROM'.";
				return;
			}

			//make sure there is a space before "FROM "
			if (commandCopy[fromPos - 1] != ' ') {
				cout << endl << "Invalid command format. Missing space before 'FROM'.";
				return;
			}

			//get the columns part
			string columnsPart = commandCopy.substr(7, fromPos - 8);  // 7 is the length of "SELECT " with a space
			trim(columnsPart);

			if (columnsPart.empty()) {
				cout << endl << "Invalid command format. Columns cannot be empty.";
				return;
			}

			//count the number of columns
			int noColumns = 1;
			for (char c : columnsPart) {
				if (c == ',') {
					noColumns++;
				}
			}

			//split the columns part into individual columns
			string* columns = new string[noColumns];
			size_t start = 0;
			size_t end = columnsPart.find(',');
			int index = 0;

			while (end != string::npos) {
				columns[index] = columnsPart.substr(start, end - start);
				trim(columns[index]);
				start = end + 1;
				end = columnsPart.find(',', start);
				index++;
			}

			//add the last column
			columns[index] = columnsPart.substr(start);
			trim(columns[index]);

			if (noColumns == 0) {
				cout << endl << "Invalid command format. No columns specified.";
				delete[] columns;
				return;
			}

			//find the position of "WHERE "
			size_t wherePos = commandCopy.find(" WHERE ");
			if (wherePos == string::npos) {
				cout << endl << "Invalid command format. Missing 'WHERE' or missing space before 'WHERE'.";
				delete[] columns;
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(fromPos + 5, wherePos - (fromPos + 5));  // 5 is the length of "FROM " with a space
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				delete[] columns;
				return;
			}

			//get the condition part
			string conditionPart = commandCopy.substr(wherePos + 7);  // 7 is the length of " WHERE " with spaces
			trim(conditionPart);

			if (conditionPart.empty()) {
				cout << endl << "Invalid command format. Condition cannot be empty.";
				delete[] columns;
				return;
			}

			//split the condition part into column and value
			size_t equalPos = conditionPart.find('=');
			if (equalPos == string::npos) {
				cout << endl << "Invalid command format. Missing '=' in condition.";
				delete[] columns;
				return;
			}

			string conditionColumn = conditionPart.substr(0, equalPos);
			trim(conditionColumn);
			string conditionValue = conditionPart.substr(equalPos + 1);
			trim(conditionValue);

			if (conditionColumn.empty() || conditionValue.empty()) {
				cout << endl << "Invalid command format. Condition column or value cannot be empty.";
				delete[] columns;
				return;
			}

			db->selectWHERE(tableName, columns, noColumns, conditionColumn, conditionValue, selectCommandsAddress);

			delete[] columns;
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandUpdateTable(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "UPDATE "
			if (commandCopy.find("UPDATE ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of " SET "
			size_t setPos = commandCopy.find(" SET ");
			if (setPos == string::npos) {
				cout << endl << "Invalid command format. Missing 'SET'.";
				return;
			}

			//find the position of " WHERE "
			size_t wherePos = commandCopy.find(" WHERE ");
			if (wherePos == string::npos) {
				cout << endl << "Invalid command format. Missing 'WHERE'.";
				return;
			}

			//extract the table name
			string tableName = commandCopy.substr(7, setPos - 7);  // 7 is the length of "UPDATE "
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//extract the set part
			string setPart = commandCopy.substr(setPos + 5, wherePos - (setPos + 5));  // 5 is the length of " SET "
			trim(setPart);

			if (setPart.empty()) {
				cout << endl << "Invalid command format. Set part cannot be empty.";
				return;
			}

			//split the set part into column and value
			size_t equalPos = setPart.find('=');
			if (equalPos == string::npos) {
				cout << endl << "Invalid command format. Missing '=' in set part.";
				return;
			}

			string setColumnName = setPart.substr(0, equalPos);
			trim(setColumnName);
			string setValue = setPart.substr(equalPos + 1);
			trim(setValue);

			if (setColumnName.empty() || setValue.empty()) {
				cout << endl << "Invalid command format. Set column or value cannot be empty.";
				return;
			}

			//extract the condition part
			string conditionPart = commandCopy.substr(wherePos + 7);  // 7 is the length of " WHERE "
			trim(conditionPart);

			if (conditionPart.empty()) {
				cout << endl << "Invalid command format. Condition cannot be empty.";
				return;
			}

			//split the condition part into column and value
			size_t conditionEqualPos = conditionPart.find('=');
			if (conditionEqualPos == string::npos) {
				cout << endl << "Invalid command format. Missing '=' in condition part.";
				return;
			}

			string whereColumnName = conditionPart.substr(0, conditionEqualPos);
			trim(whereColumnName);
			string whereValue = conditionPart.substr(conditionEqualPos + 1);
			trim(whereValue);

			if (whereColumnName.empty() || whereValue.empty()) {
				cout << endl << "Invalid command format. Condition column or value cannot be empty.";
				return;
			}

			db->updateTable(tableName, setColumnName, setValue, whereColumnName, whereValue);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandAlterTableAddColumn(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "ALTER TABLE "
			if (commandCopy.find("ALTER TABLE ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of " ADD "
			size_t addPos = commandCopy.find(" ADD ");
			if (addPos == string::npos) {
				cout << endl << "Invalid command format.";
				return;
			}

			//check for '(' after " ADD "
			size_t openParenPos = commandCopy.find("(", addPos);
			if (openParenPos == string::npos) {
				cout << endl << "Invalid command format. Missing '('.";
				return;
			}

			//extract the table name
			string tableName = commandCopy.substr(12, addPos - 12);  // 12 is the length of "ALTER TABLE "
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//extract the column definition part
			string columnDef = commandCopy.substr(openParenPos + 1);  // 1 is the length of "("
			if (columnDef.back() != ')') {
				cout << endl << "Invalid command format. Missing ')'.";
				return;
			}
			columnDef.pop_back();  //remove the closing parenthesis
			trim(columnDef);

			//split the column definition into parts
			string columnParts[4];
			int partIndex = 0;
			size_t start = 0;
			size_t end = columnDef.find(',');

			while (end != string::npos && partIndex < 4) {
				columnParts[partIndex++] = columnDef.substr(start, end - start);
				start = end + 1;
				end = columnDef.find(',', start);
			}
			if (partIndex < 4) {
				columnParts[partIndex++] = columnDef.substr(start);
			}

			if (partIndex != 4) {
				cout << endl << "Invalid command format. Column definition must have 4 parameters.";
				return;
			}

			//trim each part
			for (int i = 0; i < 4; i++) {
				trim(columnParts[i]);
			}

			//extract column details
			string columnName = columnParts[0];
			string columnTypeStr = columnParts[1];
			int columnSize = stoi(columnParts[2]);
			string defaultValue = columnParts[3];

			ColumnType columnType;
			if (columnTypeStr == "INT") {
				columnType = INT;
			}
			else if (columnTypeStr == "TEXT") {
				columnType = TEXT;
			}
			else if (columnTypeStr == "FLOAT") {
				columnType = FLOAT;
			}
			else if (columnTypeStr == "BOOLEAN") {
				columnType = BOOLEAN;
			}
			else if (columnTypeStr == "DATE") {
				columnType = DATE;
			}
			else {
				cout << endl << "Invalid column type.";
				return;
			}

			Column newColumn(columnName, columnType, columnSize, defaultValue);

			db->alterTableAddColumn(tableName, newColumn);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandAlterTableDropColumn(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "ALTER TABLE "
			if (commandCopy.find("ALTER TABLE ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of " DROP COLUMN "
			size_t dropPos = commandCopy.find(" DROP COLUMN ");
			if (dropPos == string::npos) {
				cout << endl << "Invalid command format.";
				return;
			}

			//extract the table name
			string tableName = commandCopy.substr(12, dropPos - 12);  // 12 is the length of "ALTER TABLE "
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//extract the column name
			string columnName = commandCopy.substr(dropPos + 13);  // 13 is the length of " DROP COLUMN "
			trim(columnName);

			if (columnName.empty()) {
				cout << endl << "Invalid command format. Column name cannot be empty.";
				return;
			}

			db->alterTableDeleteColumn(tableName, columnName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandCreateIndex(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "CREATE INDEX "
			if (commandCopy.find("CREATE INDEX ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//find the position of " ON "
			size_t onPos = commandCopy.find(" ON ");
			if (onPos == string::npos) {
				cout << endl << "Invalid command format. Missing 'ON'.";
				return;
			}

			//ensure there is an index name between "CREATE INDEX " and " ON "
			if (onPos <= 13) {  // 13 is the length of "CREATE INDEX "
				cout << endl << "Invalid command format. Missing index name.";
				return;
			}

			//extract the index name
			string indexName = commandCopy.substr(13, onPos - 13);
			trim(indexName);

			if (indexName.empty()) {
				cout << endl << "Invalid command format. Index name cannot be empty.";
				return;
			}

			//extract the table name and column part
			string tableAndColumnPart = commandCopy.substr(onPos + 4);  // 4 is the length of " ON "
			trim(tableAndColumnPart);

			//find the position of the opening parenthesis '('
			size_t openParenPos = tableAndColumnPart.find('(');
			if (openParenPos == string::npos) {
				cout << endl << "Invalid command format. Missing '('.";
				return;
			}

			//table name
			string tableName = tableAndColumnPart.substr(0, openParenPos);
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//find the position of the closing parenthesis ')'
			size_t closeParenPos = tableAndColumnPart.find(')', openParenPos);
			if (closeParenPos == string::npos) {
				cout << endl << "Invalid command format. Missing closing parenthesis.";
				return;
			}

			//column name
			string columnName = tableAndColumnPart.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
			trim(columnName);

			if (columnName.empty()) {
				cout << endl << "Invalid command format. Column name cannot be empty.";
				return;
			}

			// check for extra arguments after the closing parenthesis
			if (closeParenPos + 1 < tableAndColumnPart.length()) {
				string extraArgs = tableAndColumnPart.substr(closeParenPos + 1);
				trim(extraArgs);
				if (!extraArgs.empty()) {
					cout << endl << "Invalid command format. Too many arguments.";
					return;
				}
			}

			db->createIndex(indexName, columnName, tableName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandDropIndex(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "DROP INDEX "
			if (commandCopy.find("DROP INDEX ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//extract the index name
			string indexName = commandCopy.substr(11);  // 11 is the length of "DROP INDEX "
			trim(indexName);

			if (indexName.empty()) {
				cout << endl << "Invalid command format. Index name cannot be empty.";
				return;
			}

			//check for extra arguments
			size_t extraArgsPos = indexName.find(' ');
			if (extraArgsPos != string::npos) {
				cout << endl << "Invalid command format. Too many arguments.";
				return;
			}

			db->dropIndex(indexName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandShowTables(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "SHOW TABLES"
			if (commandCopy != "SHOW TABLES") {
				cout << endl << "Invalid command format.";
				return;
			}

			db->showTables();
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandShowIndexFromTable(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command starts with "SHOW INDEX FROM "
			if (commandCopy.find("SHOW INDEX FROM ") != 0) {
				cout << endl << "Invalid command format.";
				return;
			}

			//extract the table name
			string tableName = commandCopy.substr(16);  // 16 is the length of "SHOW INDEX FROM "
			trim(tableName);

			if (tableName.empty()) {
				cout << endl << "Invalid command format. Table name cannot be empty.";
				return;
			}

			//check for extra arguments
			size_t extraArgsPos = tableName.find(' ');
			if (extraArgsPos != string::npos) {
				cout << endl << "Invalid command format. Too many arguments.";
				return;
			}

			db->showIndexFromTable(tableName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandShowIndexFromAll(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "SHOW INDEX FROM ALL"
			if (commandCopy != "SHOW INDEX FROM ALL") {
				cout << endl << "Invalid command format.";
				return;
			}

			db->showIndexFromAll();
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandClearConsole(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "clear"
			if (commandCopy != "clear") {
				cout << endl << "Invalid command format.";
				return;
			}

			system("cls");
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandHelpMenu(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "help"
			if (commandCopy != "help") {
				cout << endl << "Invalid command format.";
				return;
			}

			db->printHelpMenu();
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandSyntaxMenu(const string& command) {
		try {
			system("cls");
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "help 2"
			if (commandCopy != "help 2") {
				cout << endl << "Invalid command format.";
				return;
			}

			db->printSyntaxMenu();
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
public:
	//DEFAULT CONSTRUCTOR
	Commands() {
		this->db = nullptr;
	}
	//CONSTRUCTOR
	Commands(Database* database) : db(database) {}
	//--------------------------------------------------
	void handleCommand(const string& command, const string& tablesConfigAddress, const string& selectCommandsAddress) {
		//detect and handle the command accordingly
		if (containsWord(command, "CREATE") && containsWord(command, "TABLE")) {
			stringCommandCreateTable(command);//verific daca are formatu bun si creez un obiect tip CREATE TABLE de ex (return an object)
			//if syntaxa e buna
			//creez obiect si returnez
			//functie validare sintaxsa in fiecare clasa STATICA
		}
		else if (containsWord(command, "DESCRIBE")) {
			stringCommandDescribe(command);
		}
		else if (containsWord(command, "DROP") && containsWord(command, "TABLE") && !containsWord(command, "ALTER")) {
			stringCommandDropTable(command, tablesConfigAddress);
		}
		else if (containsWord(command, "SELECT") && containsWord(command, "ALL")) {
			stringCommandSelectAll(command, selectCommandsAddress);
		}
		else if (containsWord(command, "INSERT") && containsWord(command, "INTO") && containsWord(command, "VALUES")) {
			stringCommandinsertIntoValues(command);
		}
		else if (containsWord(command, "DELETE") && containsWord(command, "FROM") && containsWord(command, "WHERE")) {
			stringCommandDeleteFromWhere(command);
		}
		else if (containsWord(command, "SELECT") && containsWord(command, "WHERE")) {
			stringCommandSelectWhere(command, selectCommandsAddress);
		}
		else if (containsWord(command, "SELECT") && !containsWord(command, "ALL") && !containsWord(command, "WHERE")) {
			stringCommandSelectColumns(command, selectCommandsAddress);
		}
		else if (containsWord(command, "UPDATE") && containsWord(command, "SET") && containsWord(command, "WHERE")) {
			stringCommandUpdateTable(command);
		}
		else if (containsWord(command, "ALTER") && containsWord(command, "TABLE") && containsWord(command, "ADD")) {
			stringCommandAlterTableAddColumn(command);
		}
		else if (containsWord(command, "ALTER") && containsWord(command, "TABLE") && containsWord(command, "DROP") && containsWord(command, "COLUMN")) {
			stringCommandAlterTableDropColumn(command);
		}
		else if (containsWord(command, "CREATE") && containsWord(command, "INDEX")) {
			stringCommandCreateIndex(command);
		}
		else if (containsWord(command, "DROP") && containsWord(command, "INDEX")) {
			stringCommandDropIndex(command);
		}
		else if (containsWord(command, "SHOW") && containsWord(command, "TABLES")) {
			stringCommandShowTables(command);
		}
		else if (containsWord(command, "SHOW") && containsWord(command, "INDEX") && containsWord(command, "FROM") && !containsWord(command, "ALL")) {
			stringCommandShowIndexFromTable(command);
		}
		else if (containsWord(command, "SHOW") && containsWord(command, "INDEX") && containsWord(command, "FROM") && containsWord(command, "ALL")) {
			stringCommandShowIndexFromAll(command);
		}
		else if (containsWord(command, "clear")) {
			stringCommandClearConsole(command);
		}
		else if (containsWord(command, "help") && !containsWord(command, "2")) {
			stringCommandHelpMenu(command);
		}
		else if (containsWord(command, "help") && containsWord(command, "2")) {
			stringCommandSyntaxMenu(command);
		}
		else {
			cout << "Unknown command." << endl;
		}
	}
};

class FileManager {
public:
	const static string TABLES_CONFIG_ADDRESS;
	const static string SELECT_COMMANDS_ADDRESS;
	const static int MAX_COMMANDS_FILES;
	const static string START_COMMANDS_ADDRESSES[];
public:
	void readStartCommandsFromFiles(const string* filenames, int count, Commands& commands) {
		for (int i = 0; i < count; i++) {
			if (filenames[i].empty()) {
				continue; //skip empty addresses
			}
			ifstream file(filenames[i]);
			if (!file.is_open()) {
				cout << endl << "Error: Could not open file " << filenames[i] << endl;
				continue; //skip to the next file if one can't be opened
			}
			string command;
			while (getline(file, command)) {
				if (!command.empty()) {
					commands.handleCommand(command, FileManager::TABLES_CONFIG_ADDRESS, FileManager::SELECT_COMMANDS_ADDRESS);
				}
			}
			cout << endl;
			file.close();
		}
	}
};

const string FileManager::TABLES_CONFIG_ADDRESS = "D:\\VS PROJECTS\\!DBMS PROJECT\\DBMS PROJECT\\tables_config\\";
const string FileManager::SELECT_COMMANDS_ADDRESS = "D:\\VS PROJECTS\\!DBMS PROJECT\\DBMS PROJECT\\select_commands\\";
const int FileManager::MAX_COMMANDS_FILES = 5;
const string FileManager::START_COMMANDS_ADDRESSES[FileManager::MAX_COMMANDS_FILES] = {
	"D:\\VS PROJECTS\\!DBMS PROJECT\\DBMS PROJECT\\start_commands\\commands1.txt",
	"D:\\VS PROJECTS\\!DBMS PROJECT\\DBMS PROJECT\\start_commands\\commands2.txt"
};

int main() {
	Database db;
	Commands commands(&db);
	FileManager fm;
	string userCommand;

	cout << "Use the 'help' command to view available commands and their syntax." << endl;

	db.loadDatabase(FileManager::TABLES_CONFIG_ADDRESS, FileManager::SELECT_COMMANDS_ADDRESS);

	//read commands from multiple files at the start
	fm.readStartCommandsFromFiles(FileManager::START_COMMANDS_ADDRESSES, FileManager::MAX_COMMANDS_FILES, commands);

	//continue with console input
	while (true) {
		cout << endl << ">> ";
		getline(cin, userCommand);

		if (userCommand == "exit") {
			break;
		}
		commands.handleCommand(userCommand, FileManager::TABLES_CONFIG_ADDRESS, FileManager::SELECT_COMMANDS_ADDRESS);
	}

	db.saveDatabase(FileManager::TABLES_CONFIG_ADDRESS);

	return 0;
}