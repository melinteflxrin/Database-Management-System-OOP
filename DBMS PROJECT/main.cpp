#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "Column.h"
#include "Row.h"
#include "TableNames.h"
#include "Index.h"

using namespace std;

class Table {
private:
	string name = "";
	Column** columns = nullptr;           // array of pointers to Column objects so i can use their constructors because i kept getting the no default constructor error
	int noColumns = 0;                    //cant use the actual column because i dont have a column constructor inside the table class; thats why Column**
	Row** rows = nullptr;
	int noRows = 0;

public:
	//DEFAULT CONSTRUCTOR
	Table() {
		this->name = "";
		this->columns = nullptr;
		this->noColumns = 0;
		this->rows = nullptr;
		this->noRows = 0;
	}
	//CONSTRUCTOR
	Table(const string& name, const Column* columns, int noColumns) {
		this->setName(name);
		this->setColumns(columns, noColumns);
	}
	//COPY CONSTRUCTOR
	Table(const Table& original) : name(original.name), noColumns(original.noColumns), noRows(original.noRows) {  //I copy FROM original  //i create an object based on another object
		if (original.columns) {   // if copy.columns is not nul (an array of pointers to Column objects)
			this->columns = new Column * [noColumns];
			for (int i = 0; i < this->noColumns; i++) {
				this->columns[i] = new Column(*original.columns[i]); // deep copy  //dereference
			}
		}
		else {
			this->columns = nullptr;  //if copy.columns is null set columns to null aswell
		}

		if (original.rows) {
			this->rows = new Row * [this->noRows];
			for (int i = 0; i < this->noRows; i++) {
				this->rows[i] = new Row(*original.rows[i]); //deep copy
			}
		}
		else {
			this->rows = nullptr;
		}
	}
	//ASSIGNMENT OPERATOR  - FOR WHEN I ASSIGN AN OBJECT TO ANOTHER (=)   // table1 = table2;
	Table& operator=(const Table& copy) {
		if (this == &copy) return *this; // self assignment check

		// clean up previous data
		for (int i = 0; i < this->noColumns; i++) {
			delete this->columns[i];
		}
		delete[] this->columns;

		for (int i = 0; i < this->noRows; i++) {
			delete this->rows[i];
		}
		delete[] this->rows;

		this->name = copy.name;
		this->noColumns = copy.noColumns;
		this->noRows = copy.noRows;

		//same as copy constructor -> assign new data
		if (copy.columns) {
			this->columns = new Column * [this->noColumns];
			for (int i = 0; i < this->noColumns; i++) {
				this->columns[i] = new Column(*copy.columns[i]); //deep copy
			}
		}
		else {
			this->columns = nullptr;
		}
		//
		if (copy.rows) {
			this->rows = new Row * [this->noRows];
			for (int i = 0; i < this->noRows; i++) {
				this->rows[i] = new Row(*copy.rows[i]);
			}
		}
		else {
			this->rows = nullptr;
		}

		return *this;
	}
	//DESTRUCTOR
	~Table() {
		if (columns != nullptr) {
			for (int i = 0; i < noColumns; i++) {
				delete this->columns[i];   // clean up each column contens
			}
			delete[] this->columns; // clean up the column array    i have to delete 2 times basically because i have Column** columns and Row** rows
		}
		if (rows != nullptr) {
			for (int i = 0; i < noRows; i++) {
				delete this->rows[i];
			}
			delete[] this->rows;
		}
	}
	//--------------------------------------------------
	//SETTERS
	void setName(const string& name) {
		if (name.empty() || name.size() < 2) {
			throw invalid_argument("Name cannot be empty or less than two characters.");
		}
		this->name = name;
	}
	void setColumns(const Column* columns, int noColumns) {   //no need for Column** because Im not passing an array of pointers; Im passing a simple array that can be accessed linearly
		if (columns == nullptr || noColumns < 1) {
			throw invalid_argument("Columns cannot be null or empty.");
		}

		if (this->columns != nullptr) {
			for (int i = 0; i < this->noColumns; i++) {
				if (this->columns[i] != nullptr) {
					delete this->columns[i];
				}
			}
			delete[] this->columns;   //erase the pointer to the array
		}

		this->noColumns = noColumns;
		this->columns = new Column * [noColumns];
		for (int i = 0; i < noColumns; i++) {
			this->columns[i] = new Column(columns[i]); // create each column using its constructor
		}
	}
	//GETTERS
	const string& getName() const {
		return this->name;
	}
	int getNoColumns() const {
		return this->noColumns;
	}
	int getNoRows() const {
		return this->noRows;
	}
	const Column& getColumn(int index) const {
		if (index < 0 || index >= this->noColumns) {
			throw out_of_range("Invalid column index.");
		}
		return *(this->columns[index]);
	}
	const Column& getColumn(const string& name) const {
		for (int i = 0; i < noColumns; i++) {
			if (columns[i]->getName() == name) {
				return *columns[i];
			}
		}
		throw invalid_argument("Column not found.");
	}
	const ColumnType getColumnType(int index) const {
		if (index < 0 || index >= this->noColumns) {
			throw out_of_range("Invalid column index.");
		}
		return this->columns[index]->getType();
	}
	const int getColumnSize(int index) const {
		if (index < 0 || index >= this->noColumns) {
			throw out_of_range("Invalid column index.");
		}
		return this->columns[index]->getSize();
	}
	int getColumnIndex(const string& name) const {
		for (int i = 0; i < noColumns; i++) {
			if (columns[i]->getName() == name) {
				return i;
			}
		}
		throw invalid_argument("Column not found.");
	}
	Row& getRow(int index) const {
		if (index < 0 || index >= this->noRows) {
			throw out_of_range("Invalid row index.");
		}
		return *this->rows[index];
	}
	//--------------------------------------------------
	//ROWS
	void deleteRow(int index) {
		if (index < 0 || index >= this->noRows) {
			cout << endl << "Error: Invalid row index.";
			return;
		}

		Row** tempRows = new Row * [this->noRows - 1];
		int tempIndex = 0;

		for (int i = 0; i < this->noRows; i++) {    //copy everything except the row with the index to be deleted
			if (i != index) {
				tempRows[tempIndex++] = this->rows[i];
			}
			else {
				delete this->rows[i];
			}
		}

		delete[] this->rows;
		this->rows = tempRows;
		this->noRows--;
	}

	void addRow(const string* values) {
		if (values == nullptr) {
			cout << endl << "Error: row values cannot be null.";
			return;
		}
		Row* newRow = new Row(this->noColumns);

		for (int i = 0; i < this->noColumns; i++) {
			const Column& column = this->getColumn(i);

			//check if is UNIQUE
			if (column.isUnique()) {
				for (int j = 0; j < this->noRows; j++) {
					if (this->rows[j]->getTextData(i) == values[i]) {
						cout << endl << "Error: Value for column: " << column.getName() << " must be unique.";
						return;
					}
				}
			}

			//check for size
			if (values[i].size() > column.getSize()) {
				cout << endl << "Error: Value for column: " << column.getName() << " exceeds the maximum size of " << column.getSize() << ".";
				return;
			}

			//setting the value
			switch (column.getType()) {
			case INT:
				newRow->setIntData(i, values[i]);
				break;
			case TEXT:
				newRow->setStringData(i, values[i]);
				break;
			case FLOAT:
				newRow->setFloatData(i, values[i]);
				break;
			case BOOLEAN:
				newRow->setStringData(i, values[i] == "TRUE" ? "TRUE" : "FALSE");
				break;
			case DATE:
				newRow->setStringData(i, values[i]); //DATE AS STRING
				break;
			default:
				cout << endl << "Error: Unsupported column type.";
				return;
			}
		}

		Row** tempRows = new Row * [this->noRows + 1];

		for (int i = 0; i < this->noRows; ++i) {
			tempRows[i] = this->rows[i];
		}

		tempRows[this->noRows] = newRow;

		delete[] this->rows;
		this->rows = tempRows;
		this->noRows++;

		cout << endl << "Values inserted into table '" << name << "' successfully.";
	}

	//--------------------------------------------------
	//COLUMNS
	bool columnExists(const string& name) const {
		for (int i = 0; i < noColumns; i++) {
			if (columns[i]->getName() == name) {
				return true;
			}
		}
		return false;
	}

	bool columnExistsByIndex(int index) const {
		if (index < 0 || index >= noColumns) {
			return false;
		}
		return true;
	}

	void deleteColumn(const string& name) {
		int index = -1;
		for (int i = 0; i < noColumns; i++) {
			if (columns[i]->getName() == name) {
				index = i;
				break;
			}
		}

		if (index == -1) {
			cout << endl << "Column: " << "'" << name << "'" << " not found.";
			return;
		}

		Column** tempColumns = new Column * [noColumns - 1];
		int tempIndex = 0;

		for (int i = 0; i < noColumns; i++) {
			if (i != index) {
				tempColumns[tempIndex++] = columns[i];    //copy everything except the column to delete
			}
			else {
				delete columns[i];
			}
		}

		delete[] columns;
		columns = tempColumns;
		noColumns--;

		//update the rows
		for (int i = 0; i < noRows; i++) {
			Row* updatedRow = new Row(noColumns);   //iterate through all rows

			for (int j = 0; j < noColumns; j++) {   //for each column of every row copy the data except the index to be deleted
				if (j < index) {
					updatedRow->setStringData(j, rows[i]->getTextData(j));
				}
				else {
					updatedRow->setStringData(j, rows[i]->getTextData(j + 1));  //after index to delete shift left positions
				}
			}

			delete rows[i];
			rows[i] = updatedRow;
		}
	}

	void deleteColumnByIndex(int index) {
		if (index < 0 || index >= noColumns) {
			cout << endl << "Error: Invalid column index.";
			return;
		}

		Column** tempColumns = new Column * [noColumns - 1];
		int tempIndex = 0;

		for (int i = 0; i < noColumns; i++) {
			if (i != index) {
				tempColumns[tempIndex++] = columns[i];
			}
			else {
				delete columns[i];
			}
		}

		delete[] columns;
		columns = tempColumns;
		noColumns--;

		for (int i = 0; i < noRows; i++) {
			Row* updatedRow = new Row(noColumns);

			for (int j = 0; j < noColumns; j++) {
				if (j < index) {
					updatedRow->setStringData(j, rows[i]->getTextData(j));
				}
				else {
					updatedRow->setStringData(j, rows[i]->getTextData(j + 1));
				}
			}

			delete rows[i];
			rows[i] = updatedRow;
		}
	}

	void addColumn(const Column& newColumn) {
		Column** tempColumns = new Column * [noColumns + 1];
		for (int i = 0; i < noColumns; ++i) {
			tempColumns[i] = columns[i];                  //deep copy
		}
		tempColumns[noColumns] = new Column(newColumn);

		//cleanup old array
		delete[] columns;
		columns = tempColumns;
		noColumns++;

		for (int i = 0; i < noRows; i++) {
			Row* updatedRow = new Row(noColumns);

			for (int j = 0; j < noColumns - 1; ++j) {  //copy existing data to the new row
				updatedRow->setStringData(j, rows[i]->getTextData(j));
			}

			switch (newColumn.getType()) {
			case INT:
				updatedRow->setIntData(noColumns - 1, newColumn.getDefaultValue());
				break;
			case TEXT:
				updatedRow->setStringData(noColumns - 1, newColumn.getDefaultValue());
				break;
			case FLOAT:
				updatedRow->setFloatData(noColumns - 1, newColumn.getDefaultValue());
				break;
			case BOOLEAN:
				updatedRow->setStringData(noColumns - 1, newColumn.getDefaultValue() == "true" ? "true" : "false");
				break;
			case DATE:
				updatedRow->setStringData(noColumns - 1, newColumn.getDefaultValue()); //date as string
				break;
			default:
				throw invalid_argument("Unsupported column type.");
			}

			delete rows[i];
			rows[i] = updatedRow;
		}
	}

	//--------------------------------------------------
	//ONLY PRINTS THE TABLE STRUCTURE WITHOUT THE CONTENTS (only columns)
	void describeTable() const {
		cout << endl << "Table name: " << this->getName() << endl;

		//maximum width for each column
		int maxNameWidth = 6; // "Column" length
		int maxTypeWidth = 4; // "Type" length
		int maxSizeWidth = 4; // "Size" length
		int maxDefaultValueWidth = 13; // "Default value" length
		int maxUniqueWidth = 6; // "Unique" length

		for (int i = 0; i < noColumns; i++) {
			maxNameWidth = max(maxNameWidth, (int)getColumn(i).getName().length());
			maxTypeWidth = max(maxTypeWidth, (int)getColumnTypeName(getColumn(i).getType()).length());
			maxSizeWidth = max(maxSizeWidth, (int)to_string(getColumn(i).getSize()).length());
			maxDefaultValueWidth = max(maxDefaultValueWidth, (int)getColumn(i).getDefaultValue().length());
		}

		//total width for the separator line
		int totalWidth = maxNameWidth + maxTypeWidth + maxSizeWidth + maxDefaultValueWidth + maxUniqueWidth + 8 * 5;

		//print header
		cout << endl
			<< "Column" << string(maxNameWidth - 6 + 8, ' ')
			<< "Type" << string(maxTypeWidth - 4 + 8, ' ')
			<< "Size" << string(maxSizeWidth - 4 + 8, ' ')
			<< "Default value" << string(maxDefaultValueWidth - 13 + 8, ' ')
			<< "Unique";
		cout << endl << string(totalWidth, '-');

		//print details for each column
		for (int i = 0; i < noColumns; i++) {
			cout << endl;

			// NAME
			cout << getColumn(i).getName() << string(maxNameWidth - getColumn(i).getName().length() + 8, ' ');

			// TYPE
			cout << getColumnTypeName(getColumn(i).getType()) << string(maxTypeWidth - getColumnTypeName(getColumn(i).getType()).length() + 8, ' ');

			// SIZE
			cout << getColumn(i).getSize() << string(maxSizeWidth - to_string(getColumn(i).getSize()).length() + 8, ' ');

			// DEFAULT VALUE
			cout << getColumn(i).getDefaultValue() << string(maxDefaultValueWidth - getColumn(i).getDefaultValue().length() + 8, ' ');

			// UNIQUE
			cout << (getColumn(i).isUnique() ? "Yes" : "No");
		}
		cout << endl << string(totalWidth, '-') << endl;
	}
	string getColumnTypeName(ColumnType type) const {
		switch (type) {
		case INT: return "INT";
		case TEXT: return "TEXT";
		case FLOAT: return "FLOAT";
		case BOOLEAN: return "BOOLEAN";
		case DATE: return "DATE";
		default: return "UNKNOWN";
		}
	}
	//DISPLAY CONTENTS AND EVERYTHING (columns and rows)
	void displayTable() const {
		cout << endl << "Table: " << this->getName() << endl;

		int* maxWidth = new int[this->noColumns];
		int totalWidth = 0;

		//maximum width for each column
		for (int i = 0; i < this->noColumns; i++) {
			maxWidth[i] = this->getColumn(i).getName().length();  // get length of each column to determine the max width
		}

		for (int i = 0; i < this->noRows; i++) {
			Row* row = this->rows[i];  // current row
			for (int j = 0; j < this->noColumns; j++) {
				int currentLength = 0;
				const Column& column = this->getColumn(j);

				if (column.getType() == INT) {
					currentLength = to_string(row->getIntData(j)).length();
				}
				else if (column.getType() == TEXT) {
					currentLength = row->getTextData(j).length();
				}
				else if (column.getType() == FLOAT) {
					float value = row->getFloatData(j);
					string floatStr = to_string(value);
					size_t dotPos = floatStr.find('.');
					if (dotPos != string::npos) {
						floatStr = floatStr.substr(0, dotPos + 3);  //keep two decimal places
					}
					currentLength = floatStr.length();
				}

				if (currentLength > maxWidth[j]) {
					maxWidth[j] = currentLength;  // update max width if needed (if data too big)
				}
			}
		}

		//calculate the total width of the table
		for (int i = 0; i < this->noColumns; i++) {
			totalWidth += maxWidth[i] + 8;  // add 8 for padding between columns
		}

		//separator line
		cout << string(totalWidth, '-') << endl;

		//column names
		for (int i = 0; i < this->noColumns; i++) {
			cout << this->getColumn(i).getName();
			for (int j = 0; j < (maxWidth[i] - this->getColumn(i).getName().length()); j++) {
				cout << " ";
			}
			cout << string(8, ' ');  // add 8 spaces between columns
		}
		cout << endl << string(totalWidth, '-') << endl;

		//rows
		for (int i = 0; i < this->noRows; i++) {
			Row* row = this->rows[i];  // current row
			for (int j = 0; j < this->noColumns; j++) {
				const Column& column = this->getColumn(j);  // column for current row

				if (j > 0) {
					cout << string(8, ' ');  // add 8 spaces between columns
				}

				string dataToString;
				if (column.getType() == INT) {
					dataToString = to_string(row->getIntData(j));
				}
				else if (column.getType() == TEXT) {
					dataToString = row->getTextData(j);
				}
				else if (column.getType() == FLOAT) {
					float value = row->getFloatData(j);
					string floatStr = to_string(value);
					size_t dotPos = floatStr.find('.');
					if (dotPos != string::npos) {
						floatStr = floatStr.substr(0, dotPos + 3);  //keep two decimal places
					}
					dataToString = floatStr;
				}
				else {
					dataToString = "N/A";  // for bool and date
				}

				cout << dataToString;

				// spaces to align with the max column width
				for (int k = 0; k < (maxWidth[j] - dataToString.length()); k++) {
					cout << " ";
				}
			}
			cout << endl;
		}

		//print the separator line at the end
		cout << string(totalWidth, '-') << endl;

		delete[] maxWidth;
	}
};

class Database {          //many tables
private:
	Table** database = nullptr;
	int noTables = 0;

	TableNames* tableNames = nullptr;
	Index* indexes = nullptr;

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
		this->indexes = new Index();
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
	void dropTable(const string& name) {
		if (!tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		int indexToRemove = getTableIndex(name);
		if (indexToRemove == -1) return;

		//check for indexes and remove them
		while (indexes->indexExistsByTableName(name)) { //the indexExists loops through tableNames and if it finds duplicates it removes all of them
			indexes->removeIndexByTableName(name);
		}

		delete database[indexToRemove];

		//shift remaining table pointers to the left
		for (int i = indexToRemove; i < noTables - 1; i++) {
			database[i] = database[i + 1];
		}

		noTables--;
		database[noTables] = nullptr; //set last element to nullptr for no dangling pointer

		//also remove the table name from tableNames
		tableNames->removeName(name);

		cout << endl << "Table: " << "'" << name << "'" << " dropped successfully.";
	}
	void insertIntoTable(const string& name, const string* values, int noValues) {
		if (!tableExists(name)) {
			cout << endl << "Error: Table: '" << name << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(name);

		//get the table at our index
		Table* table = database[tableIndex];

		if (noValues != table->getNoColumns()) {
			cout << endl << "Error: Number of values does not match the number of columns in the table.";
			return;
		}

		//add the row
		table->addRow(values);
	}
	void deleteRowFromTable(const string& name, const string& columnName, const string& value) {
		if (!tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		int tableIndex = getTableIndex(name);
		Table* table = database[tableIndex];

		int columnIndex = 0;
		bool hasIndex = indexes->indexExists(columnName, name);
		if (hasIndex) {
			columnIndex = indexes->getIndexValue(columnName, name);
			if (!table->columnExistsByIndex(columnIndex)) {
				cout << endl << "Error: Column with index '" << columnIndex << "' does not exist in table '" << name << "'.";
				return;
			}
		}
		else {
			if (!table->columnExists(columnName)) {
				cout << endl << "Error: Column '" << columnName << "' does not exist in table '" << name << "'.";
				return;
			}
			columnIndex = table->getColumnIndex(columnName);
		}

		ColumnType columnType = table->getColumn(columnName).getType();

		//delete the row
		for (int i = 0; i < table->getNoRows(); i++) {
			Row& row = table->getRow(i);
			switch (columnType) {
			case INT:
				if (row.getIntData(columnIndex) == stoi(value)) {
					table->deleteRow(i);
					cout << endl << "Row deleted successfully.";
					return;
				}
				break;
			case TEXT:
				if (row.getTextData(columnIndex) == value) {
					table->deleteRow(i);
					cout << endl << "Row deleted successfully.";
					return;
				}
				break;
			case FLOAT:
				if (row.getFloatData(columnIndex) == stof(value)) {
					table->deleteRow(i);
					cout << endl << "Row deleted successfully.";
					return;
				}
				break;
			default:
				cout << endl << "Error: Unsupported column type.";
			}
		}
		cout << endl << "Error: Row with value '" << value << "' not found in column '" << columnName << "'.";
	}
	void selectALL(const string& name) const {
		if (!tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		int index = getTableIndex(name);
		database[index]->displayTable();
	}
	void selectWHERE(const string& tableName, const string* columnNames, int noColumns, const string& conditionColumn, const string& value) {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		int* columnIndexes = new int[noColumns];
		int conditionColumnIndex;

		//check if all specified columns and the condition column exist in the table
		for (int i = 0; i < noColumns; i++) {
			if (indexes->indexExists(columnNames[i], tableName)) {
				columnIndexes[i] = indexes->getIndexValue(columnNames[i], tableName);
				if (!table->columnExistsByIndex(columnIndexes[i])) {
					cout << endl << "Error: Column with index '" << columnIndexes[i] << "' does not exist in table '" << tableName << "'.";
					delete[] columnIndexes;
					return;
				}
			}
			else {
				if (!table->columnExists(columnNames[i])) {
					cout << endl << "Error: Column '" << columnNames[i] << "' does not exist in table '" << tableName << "'.";
					delete[] columnIndexes;
					return;
				}
				columnIndexes[i] = table->getColumnIndex(columnNames[i]);
			}
		}

		if (indexes->indexExists(conditionColumn, tableName)) {
			conditionColumnIndex = indexes->getIndexValue(conditionColumn, tableName);
			if (!table->columnExistsByIndex(conditionColumnIndex)) {
				cout << endl << "Error: Condition column with index '" << conditionColumnIndex << "' does not exist in table '" << tableName << "'.";
				delete[] columnIndexes;
				return;
			}
		}
		else {
			if (!table->columnExists(conditionColumn)) {
				cout << endl << "Error: Condition column '" << conditionColumn << "' does not exist in table '" << tableName << "'.";
				delete[] columnIndexes;
				return;
			}
			conditionColumnIndex = table->getColumnIndex(conditionColumn);
		}

		//calculate the maximum width for each column
		int* maxWidth = new int[noColumns];
		for (int i = 0; i < noColumns; i++) {
			maxWidth[i] = columnNames[i].length();
		}

		for (int i = 0; i < table->getNoRows(); i++) {
			Row& row = table->getRow(i);
			for (int j = 0; j < noColumns; j++) {
				int currentLength = 0;
				const Column& column = table->getColumn(columnNames[j]);

				if (column.getType() == INT) {
					currentLength = to_string(row.getIntData(columnIndexes[j])).length();
				}
				else if (column.getType() == TEXT) {
					currentLength = row.getTextData(columnIndexes[j]).length();
				}
				else if (column.getType() == FLOAT) {
					float value = row.getFloatData(columnIndexes[j]);
					string floatStr = to_string(value);
					size_t dotPos = floatStr.find('.');
					if (dotPos != string::npos) {
						floatStr = floatStr.substr(0, dotPos + 3);  //keep two decimal places
					}
					currentLength = floatStr.length();
				}

				if (currentLength > maxWidth[j]) {
					maxWidth[j] = currentLength;
				}
			}
		}

		//calculate the total width of the table
		int totalWidth = 0;
		for (int i = 0; i < noColumns; i++) {
			totalWidth += maxWidth[i] + 8;  //add 8 for padding between columns
		}

		//print the separator line
		cout << endl << string(totalWidth, '-') << endl;

		//print column headers
		for (int i = 0; i < noColumns; i++) {
			cout << columnNames[i];
			for (int j = 0; j < (maxWidth[i] - columnNames[i].length()); j++) {
				cout << " ";
			}
			cout << string(8, ' ');  // 8 spaces between columns
		}
		cout << endl << string(totalWidth, '-') << endl;

		//print rows that match the condition
		bool valueFound = false;
		for (int i = 0; i < table->getNoRows(); i++) {
			Row& row = table->getRow(i);
			bool found = true;
			const Column& conditionCol = table->getColumn(conditionColumn);
			switch (conditionCol.getType()) {
			case INT:
				if (row.getIntData(conditionColumnIndex) != stoi(value)) {
					found = false;
				}
				break;
			case TEXT:
				if (row.getTextData(conditionColumnIndex) != value) {
					found = false;
				}
				break;
			case FLOAT:
				if (row.getFloatData(conditionColumnIndex) != stof(value)) {
					found = false;
				}
				break;
			default:
				found = false;
			}

			if (found) {
				valueFound = true;
				for (int j = 0; j < noColumns; j++) {
					const Column& column = table->getColumn(columnNames[j]);
					string dataToString;
					if (column.getType() == INT) {
						dataToString = to_string(row.getIntData(columnIndexes[j]));
					}
					else if (column.getType() == TEXT) {
						dataToString = row.getTextData(columnIndexes[j]);
					}
					else if (column.getType() == FLOAT) {
						float value = row.getFloatData(columnIndexes[j]);
						string floatStr = to_string(value);
						size_t dotPos = floatStr.find('.');
						if (dotPos != string::npos) {
							floatStr = floatStr.substr(0, dotPos + 3);  //keep two decimal places
						}
						dataToString = floatStr;
					}
					else {
						dataToString = "N/A";  //BOOLEAN and DATE
					}

					cout << dataToString;
					for (int k = 0; k < (maxWidth[j] - dataToString.length()); k++) {
						cout << " ";
					}
					cout << string(8, ' ');  //8 spaces between columns
				}
				cout << endl;
			}
		}

		if (!valueFound) {
			cout << "No rows found with " << conditionColumn << " = " << value << endl;
		}

		//print the separator line at the end
		cout << string(totalWidth, '-') << endl;

		delete[] columnIndexes;
		delete[] maxWidth;
	}
	void selectColumns(const string& tableName, const string* columnNames, int noColumns) {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		int* columnIndexes = new int[noColumns];

		//check if all specified columns exist in the table
		for (int i = 0; i < noColumns; i++) {
			if (indexes->indexExists(columnNames[i], tableName)) {
				columnIndexes[i] = indexes->getIndexValue(columnNames[i], tableName);
				if (!table->columnExistsByIndex(columnIndexes[i])) {
					cout << endl << "Error: Column with index '" << columnIndexes[i] << "' does not exist in table '" << tableName << "'.";
					delete[] columnIndexes;
					return;
				}
			}
			else {
				if (!table->columnExists(columnNames[i])) {
					cout << endl << "Error: Column '" << columnNames[i] << "' does not exist in table '" << tableName << "'.";
					delete[] columnIndexes;
					return;
				}
				columnIndexes[i] = table->getColumnIndex(columnNames[i]);
			}
		}

		//calculate the maximum width for each column
		int* maxWidth = new int[noColumns];
		for (int i = 0; i < noColumns; i++) {
			maxWidth[i] = columnNames[i].length();
		}

		for (int i = 0; i < table->getNoRows(); i++) {
			Row& row = table->getRow(i);
			for (int j = 0; j < noColumns; j++) {
				int currentLength = 0;
				const Column& column = table->getColumn(columnNames[j]);

				if (column.getType() == INT) {
					currentLength = to_string(row.getIntData(columnIndexes[j])).length();
				}
				else if (column.getType() == TEXT) {
					currentLength = row.getTextData(columnIndexes[j]).length();
				}
				else if (column.getType() == FLOAT) {
					float value = row.getFloatData(columnIndexes[j]);
					string floatStr = to_string(value);
					size_t dotPos = floatStr.find('.');
					if (dotPos != string::npos) {
						floatStr = floatStr.substr(0, dotPos + 3);  //keep two decimal places
					}
					currentLength = floatStr.length();
				}

				if (currentLength > maxWidth[j]) {
					maxWidth[j] = currentLength;
				}
			}
		}

		//calculate the total width of the table
		int totalWidth = 0;
		for (int i = 0; i < noColumns; i++) {
			totalWidth += maxWidth[i] + 8;  // add 8 for padding between columns
		}

		//print the separator line
		cout << endl << string(totalWidth, '-') << endl;

		//print column headers
		for (int i = 0; i < noColumns; i++) {
			cout << columnNames[i];
			for (int j = 0; j < (maxWidth[i] - columnNames[i].length()); j++) {
				cout << " ";
			}
			cout << string(8, ' ');  // add 8 spaces between columns
		}
		cout << endl << string(totalWidth, '-') << endl;

		//rows
		for (int i = 0; i < table->getNoRows(); i++) {
			Row& row = table->getRow(i);
			for (int j = 0; j < noColumns; j++) {
				const Column& column = table->getColumn(columnNames[j]);
				string dataToString;
				if (column.getType() == INT) {
					dataToString = to_string(row.getIntData(columnIndexes[j]));
				}
				else if (column.getType() == TEXT) {
					dataToString = row.getTextData(columnIndexes[j]);
				}
				else if (column.getType() == FLOAT) {
					float value = row.getFloatData(columnIndexes[j]);
					string floatStr = to_string(value);
					size_t dotPos = floatStr.find('.');
					if (dotPos != string::npos) {
						floatStr = floatStr.substr(0, dotPos + 3);  //keep two decimal places
					}
					dataToString = floatStr;
				}
				else {
					dataToString = "N/A";  //for BOOLEAN and DATE
				}

				cout << dataToString;
				for (int k = 0; k < (maxWidth[j] - dataToString.length()); k++) {
					cout << " ";
				}
				cout << string(8, ' ');  //add 8 spaces between columns
			}
			cout << endl;
		}

		//print the separator line at the end
		cout << string(totalWidth, '-') << endl;

		delete[] columnIndexes;
		delete[] maxWidth;
	}
	void updateTable(const string& tableName, const string& setColumnName, const string& setValue, const string& whereColumnName, const string& whereValue) {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		int setColumnIndex;
		int whereColumnIndex;

		bool setColumnHasIndex = indexes->indexExists(setColumnName, tableName);
		bool whereColumnHasIndex = indexes->indexExists(whereColumnName, tableName);

		if (setColumnHasIndex) {
			setColumnIndex = indexes->getIndexValue(setColumnName, tableName);
			if (!table->columnExistsByIndex(setColumnIndex)) {
				cout << endl << "Error: Column with index '" << setColumnIndex << "' does not exist in table '" << tableName << "'.";
				return;
			}
		}
		else {
			if (!table->columnExists(setColumnName)) {
				cout << endl << "Error: Column '" << setColumnName << "' does not exist in table '" << tableName << "'.";
				return;
			}
			setColumnIndex = table->getColumnIndex(setColumnName);
		}

		if (whereColumnHasIndex) {
			whereColumnIndex = indexes->getIndexValue(whereColumnName, tableName);
			if (!table->columnExistsByIndex(whereColumnIndex)) {
				cout << endl << "Error: Column with index '" << whereColumnIndex << "' does not exist in table '" << tableName << "'.";
				return;
			}
		}
		else {
			if (!table->columnExists(whereColumnName)) {
				cout << endl << "Error: Column '" << whereColumnName << "' does not exist in table '" << tableName << "'.";
				return;
			}
			whereColumnIndex = table->getColumnIndex(whereColumnName);
		}

		//check if the set value corresponds with the column type and does not exceed the maximum size
		ColumnType setColumnType = table->getColumnType(setColumnIndex);
		int setColumnSize = table->getColumnSize(setColumnIndex);

		try {
			switch (setColumnType) {
			case INT: {
				if (!isValidInt(setValue)) {
					throw invalid_argument("Set value must be a valid integer.");
				}
				if (setValue.size() > setColumnSize) {
					throw invalid_argument("Set value exceeds the maximum size for the integer column.");
				}
				break;
			}
			case FLOAT: {
				if (!isValidFloat(setValue)) {
					throw invalid_argument("Set value must be a valid float.");
				}
				if (setValue.size() > setColumnSize) {
					throw invalid_argument("Set value exceeds the maximum size for the float column.");
				}
				break;
			}
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
			cout << endl << "Error: " << e.what();
			return;
		}

		//update rows
		int updatedRows = 0;
		for (int i = 0; i < table->getNoRows(); i++) {
			Row& row = table->getRow(i);
			if (row.getTextData(whereColumnIndex) == whereValue) {
				row.setStringData(setColumnIndex, setValue);
				updatedRows++;
			}
		}

		cout << endl << "Updated " << updatedRows << " rows in table '" << tableName << "' by setting " << setColumnName << " to '" << setValue << "' where " << whereColumnName << " is '" << whereValue << "'.";
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
		//BASICALLY IF I HAVE AN INDEX ON THAT COLUMN I USE FUNCTIONS BASED ON THAT INDEX WHICH ARE FASTER
		//IF I DONT HAVE AN INDEX I USE FUNCTIONS THAT SEARCH BY NAME EVERY COLUMN
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		int columnIndex;
		bool hasIndex = indexes->indexExists(columnName, tableName);
		if (hasIndex) {
			columnIndex = indexes->getIndexValue(columnName, tableName);
			if (!table->columnExistsByIndex(columnIndex)) {
				cout << endl << "Error: Column with index '" << columnIndex << "' does not exist in table '" << tableName << "'.";
				return;
			}
		}
		else {
			if (!table->columnExists(columnName)) {
				cout << endl << "Error: Column '" << columnName << "' does not exist in table '" << tableName << "'.";
				return;
			}
			columnIndex = table->getColumnIndex(columnName);
		}

		if (hasIndex) {
			indexes->removeIndex(columnName, tableName);
			table->deleteColumnByIndex(columnIndex);
			cout << endl << "Index on column '" << columnName << "' in table '" << tableName << "' removed successfully.";
		}
		else {
			table->deleteColumn(columnName);
		}

		cout << endl << "Column '" << columnName << "' deleted from table '" << tableName << "' successfully.";
	}
	void createIndex(const string& indexName, const string& columnName, const string& tableName) {
		if (indexes->indexExistsByIndexName(indexName)) {
			cout << endl << "Error: Index '" << indexName << "' already exists.";
			return;
		}

		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		Table* table = database[getTableIndex(tableName)];
		if (!table->columnExists(columnName)) {
			cout << endl << "Error: Column '" << columnName << "' does not exist in table '" << tableName << "'.";
			return;
		}

		if (indexes->indexExists(columnName, tableName)) {
			cout << endl << "Error: Index for column '" << columnName << "' in table '" << tableName << "' already exists.";
			return;
		}

		int columnIndex = table->getColumnIndex(columnName);
		indexes->addIndex(indexName, columnIndex, columnName, tableName);

		cout << endl << "Index '" << indexName << "' created successfully on column '" << columnName << "' in table '" << tableName << "'.";
	}
	void dropIndex(const string& indexName) {
		if (!indexes->indexExistsByIndexName(indexName)) {
			cout << endl << "Error: Index '" << indexName << "' does not exist.";
			return;
		}

		string tableName = indexes->getIndexTableName(indexName);
		string columnName = indexes->getIndexColumnName(indexName);

		cout << endl << "Index '" << indexName << "' removed from table '" << tableName << "' on column '" << columnName << "'.";
		indexes->removeIndexByIndexName(indexName);
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
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		cout << endl << "Indexes for table '" << tableName << "':";
		indexes->showIndexesByTableName(tableName);
	}
	void showIndexFromAll() const {
		cout << endl << "My Indexes:";
		indexes->showAllIndexes();
	}
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
};

class Commands {
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
	void stringCommandSelectAll(const string& command) {
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

			db->selectALL(tableName);
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
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
	void stringCommandDropTable(const string& command) {
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

			db->dropTable(tableName);
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
	void stringCommandSelectColumns(const string& command) {
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

			db->selectColumns(tableName, columns, noColumns);

			delete[] columns;
		}
		catch (const invalid_argument& e) {
			cout << endl << e.what();
		}
	}
	void stringCommandSelectWhere(const string& command) {
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

			db->selectWHERE(tableName, columns, noColumns, conditionColumn, conditionValue);

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
	void stringCommandExitProgram(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if the command is "exit"
			if (commandCopy != "exit") {
				cout << endl << "Invalid command format.";
				return;
			}

			exit(0);
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
	void handleCommand(const string& command) {
		//detect and handle the command accordingly
		if (containsWord(command, "CREATE") && containsWord(command, "TABLE")) {
			stringCommandCreateTable(command);
		}
		else if (containsWord(command, "DESCRIBE")) {
			stringCommandDescribe(command);
		}
		else if (containsWord(command, "DROP") && containsWord(command, "TABLE")) {
			stringCommandDropTable(command);
		}
		else if (containsWord(command, "SELECT") && containsWord(command, "ALL")) {
			stringCommandSelectAll(command);
		}
		else if (containsWord(command, "INSERT") && containsWord(command, "INTO") && containsWord(command, "VALUES")) {
			stringCommandinsertIntoValues(command);
		}
		else if (containsWord(command, "DELETE") && containsWord(command, "FROM") && containsWord(command, "WHERE")) {
			stringCommandDeleteFromWhere(command);
		}
		else if (containsWord(command, "SELECT") && containsWord(command, "WHERE")) {
			stringCommandSelectWhere(command);
		}
		else if (containsWord(command, "SELECT") && !containsWord(command, "ALL") && !containsWord(command, "WHERE")) {
			stringCommandSelectColumns(command);
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
		else if (containsWord(command, "exit")) {
			stringCommandExitProgram(command);
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

class FilesManager {
private:
public:
	const static int MAX_COMMANDS_FILES;
	const static string START_COMMANDS_ADDRESSES[];
public:
	void readStartCommandsFromFiles(const string* filenames, int count, Commands& commands) {
		for (int i = 0; i < count; ++i) {
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
					commands.handleCommand(command);
				}
			}
			cout << endl;
			file.close();
		}
	}
};

const int FilesManager::MAX_COMMANDS_FILES = 5;
const string FilesManager::START_COMMANDS_ADDRESSES[FilesManager::MAX_COMMANDS_FILES] = {
	"D:\\VS PROJECTS\\!DBMS PROJECT\\DBMS PROJECT\\start_commands\\commands1.txt",
	"D:\\VS PROJECTS\\!DBMS PROJECT\\DBMS PROJECT\\start_commands\\commands2.txt"
};

int main() {
	Database db;
	Commands commands(&db);
	FilesManager fm;
	string userCommand;

	cout << "Use the 'help' command to view available commands and their syntax." << endl;

	//read commands from multiple files at the start
	//fm.readStartCommandsFromFiles(FilesManager::START_COMMANDS_ADDRESSES, FilesManager::MAX_COMMANDS_FILES, commands);

	//continue with console input
	while (true) {
		cout << endl << ">> ";
		getline(cin, userCommand);

		if (userCommand == "exit") {
			break;
		}

		commands.handleCommand(userCommand);
	}

	return 0;
}