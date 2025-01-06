#include <iostream>
#include <string.h>
#include <string>

using namespace std;

enum ColumnType { INT, TEXT, FLOAT, BOOLEAN, DATE };

class Column {
private:
	string name = "";
	ColumnType type = TEXT;
	int size = 0;
	string defaultValue = "";
	bool unique = false;

public:
	//SETTERS
	void setName(const string& name) {
		if (name.empty() || name.size() < 2) {
			throw invalid_argument("Name cannot be empty or less than two characters.");
		}
		this->name = name;
	}
	void setType(ColumnType type) {
		this->type = type;
	}
	void setSize(int size) {
		if (size <= 0) {
			throw invalid_argument("Size must be larger than zero.");
		}
		this->size = size;
	}
	void setDefaultValue(const string& defaultValue) {
		switch (this->type) {
		case TEXT:
			this->defaultValue = defaultValue;
			break;
		case INT:
			try {
				stoi(defaultValue);
				this->defaultValue = defaultValue;
			}
			catch (const invalid_argument&) {
				throw invalid_argument("Default value must be a valid integer.");
			}
			break;
		case FLOAT:
			try {
				stof(defaultValue);
				this->defaultValue = defaultValue;
			}
			catch (const invalid_argument&) {
				throw invalid_argument("Default value must be a valid float.");
			}
			break;
		default:
			throw invalid_argument("Invalid column type.");
		}
	}
	//GETTERS
	const string& getName() const {
		return this->name;
	}
	ColumnType getType() const {
		return this->type;
	}
	int getSize() const {
		return this->size;
	}
	const string& getDefaultValue() const {
		return this->defaultValue;
	}
	const bool isUnique() const {
		return this->unique;
	}
	//DEFAULT CONSTRUCTOR
	Column() {
		this->name = "";
		this->type = TEXT;
		this->size = 0;
		this->defaultValue = "";
		this->unique = false;
	}
	//CONSTRUCTOR
	Column(const string& name, ColumnType type, int size, const string& defaultValue) {
		this->setName(name);
		this->setType(type);
		this->setSize(size);
		this->setDefaultValue(defaultValue);
		this->unique = false;
	}
	//CONSTRUCTOR FOR UNIQUE
	Column(const string& name, ColumnType type, int size, const string& defaultValue, bool unique) {
		this->setName(name);
		this->setType(type);
		this->setSize(size);
		this->setDefaultValue(defaultValue);
		this->unique = unique;
	}
};

class Row {
private:
	string* data = nullptr;
	int noColumns = 0;

public:
	//DESTRUCTOR
	~Row() {
		delete[] data;
	}
	//DEFAULT CONSTRUCTOR
	Row() {
		this->data = nullptr;
		this->noColumns = 0;
	}
	//CONSTRUCTOR
	Row(int noColumns) {
		this->noColumns = noColumns;
		data = new string[noColumns];
	}
	//COPY CONSTRUCTOR
	Row(const Row& original) : noColumns(original.noColumns) {
		data = new string[noColumns];
		for (int i = 0; i < noColumns; i++) {
			data[i] = original.data[i];  // deep copy
		}
	}
	//ASSIGNMENT OPERATOR
	Row& operator=(const Row& original) {
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
	void setIntData(int columnIndex, int value) {
		if (columnIndex < 0 || columnIndex >= noColumns)
			throw out_of_range("Column index out of range.");

		data[columnIndex] = to_string(value);
	}
	void setStringData(int columnIndex, const string& value) {
		if (columnIndex < 0 || columnIndex >= noColumns)
			throw out_of_range("Column index out of range.");

		data[columnIndex] = value;
	}
	void setFloatData(int columnIndex, float value) {
		if (columnIndex < 0 || columnIndex >= noColumns)
			throw out_of_range("Column index out of range.");

		data[columnIndex] = to_string(value);
	}
	//GETTERS
	int getIntData(int columnIndex) const {
		if (columnIndex < 0 || columnIndex >= noColumns)
			throw out_of_range("Column index out of range.");
		try {
			return stoi(data[columnIndex]);
		}
		catch (const invalid_argument&) {
			throw invalid_argument("Data at the specified index is not a valid integer.");
		}
	}
	string getTextData(int columnIndex) const {
		if (columnIndex < 0 || columnIndex >= noColumns)
			throw out_of_range("Column index out of range.");

		return data[columnIndex];
	}
	float getFloatData(int columnIndex) const {
		if (columnIndex < 0 || columnIndex >= noColumns)
			throw out_of_range("Column index out of range.");
		try {
			return stof(data[columnIndex]);
		}
		catch (const invalid_argument&) {
			throw invalid_argument("Data at the specified index is not a valid float.");
		}
	}
};

class TableNames {            //to check if a table name already exists
private:
	string* names = nullptr;
	int noNames = 0;
public:
	//DESTRUCTOR
	~TableNames() {
		delete[] names;
	}
	//DEFAULT CONSTRUCTOR
	TableNames() {
		this->names = nullptr;
		this->noNames = 0;
		this->names = new string[noNames];
	}
	//ASSIGNMENT OPERATOR
	TableNames& operator=(const TableNames& original) {
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
	TableNames(const TableNames& original) : noNames(original.noNames) {
		names = new string[noNames];
		for (int i = 0; i < noNames; i++) {
			names[i] = original.names[i];
		}
	}
	//**************************************
	void addName(const string& name) {
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
	bool nameExists(const string& name) const {
		for (int i = 0; i < noNames; i++) {
			if (names[i] == name) {
				return true;
			}
		}
		return false;
	}
	int getSize() const {
		return noNames;
	}
	//**************************************
	void removeName(const string& name) {
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
};

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
				newRow->setIntData(i, stoi(values[i]));
				break;
			case TEXT:
				newRow->setStringData(i, values[i]);
				break;
			case FLOAT:
				newRow->setFloatData(i, stof(values[i]));
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
				updatedRow->setIntData(noColumns - 1, stoi(newColumn.getDefaultValue()));
				break;
			case TEXT:
				updatedRow->setStringData(noColumns - 1, newColumn.getDefaultValue());
				break;
			case FLOAT:
				updatedRow->setFloatData(noColumns - 1, stof(newColumn.getDefaultValue()));
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
		//display borders and stuff
		cout << endl << "Table name: " << this->getName();
		cout << endl << "Column\t\tType\t\tSize\t\tDefault value\t\tUnique";
		cout << endl << "--------------------------------------------------------------------------";

		for (int i = 0; i < noColumns; i++) {
			cout << endl;

			//NAME
			cout << getColumn(i).getName() << "\t\t";

			//TYPE
			switch (getColumn(i).getType()) { // Display type from enum
			case INT: cout << "INT"; break;
			case TEXT: cout << "TEXT"; break;
			case FLOAT: cout << "FLOAT"; break;
			case BOOLEAN: cout << "BOOLEAN"; break;
			case DATE: cout << "DATE"; break;
			}

			//SIZE
			cout << "\t\t" << getColumn(i).getSize() << "\t\t";

			//DEFAULT VALUE
			if (getColumn(i).getType() == INT) {
				cout << getColumn(i).getDefaultValue();
			}
			else if (getColumn(i).getType() == TEXT) {
				cout << getColumn(i).getDefaultValue();
			}
			else if (getColumn(i).getType() == FLOAT) {
				cout << getColumn(i).getDefaultValue();
			}
			else {
				cout << "N/A";
			}

			//UNIQUE
			cout << "\t\t" << (getColumn(i).isUnique() ? "Yes" : "No") << endl;
		}
		cout << endl << "-------------------------------------------------------------" << endl;
	}

	//DISPLAY CONTENTS AND EVERYTHING (columns and rows)
	void displayTable() const {
		cout << endl << "Table: " << this->getName();
		cout << endl << "-------------------------------------------------------------" << endl;

		int* maxWidth = new int[this->noColumns];
		for (int i = 0; i < this->noColumns; i++) {
			maxWidth[i] = this->getColumn(i).getName().length();  //get length of each column to determine the max width
		}

		for (int i = 0; i < this->noRows; i++) {
			Row* row = this->rows[i];  //current row
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
					currentLength = to_string(row->getFloatData(j)).length();
				}

				if (currentLength > maxWidth[j]) {
					maxWidth[j] = currentLength;  //update max width if needed(if data too big)
				}
			}
		}

		//column names
		for (int i = 0; i < this->noColumns; i++) {
			cout << this->getColumn(i).getName();
			for (int j = 0; j < (maxWidth[i] - this->getColumn(i).getName().length()); j++) {
				cout << " ";
			}
			cout << "\t\t";
		}
		cout << endl << "-------------------------------------------------------------" << endl;

		//rows
		for (int i = 0; i < this->noRows; i++) {
			Row* row = this->rows[i];  //current row
			for (int j = 0; j < this->noColumns; j++) {
				const Column& column = this->getColumn(j);  //column for current row

				if (j > 0) {
					cout << "\t\t";  //add tab before all but the first column
				}

				if (column.getType() == INT) {
					cout << row->getIntData(j);
				}
				else if (column.getType() == TEXT) {
					cout << row->getTextData(j);
				}
				else if (column.getType() == FLOAT) {
					cout << row->getFloatData(j);
				}
				else {
					cout << "N/A";  //for bool and date
				}

				//spaces to align with the max column width
				string dataToString;
				if (column.getType() == INT) {
					dataToString = to_string(row->getIntData(j));
				}
				else if (column.getType() == TEXT) {
					dataToString = row->getTextData(j);
				}
				else if (column.getType() == FLOAT) {
					dataToString = to_string(row->getFloatData(j));
				}

				for (int k = 0; k < (maxWidth[j] - dataToString.length()); k++) {
					cout << " ";
				}
			}
			cout << endl;
		}

		delete[] maxWidth;
	}
};

class Database {          //many tables
private:
	Table** database = nullptr;
	int noTables = 0;

	TableNames* tableNames = nullptr;
public:
	//DEFAULT CONSTRUCTOR
	Database() {
		this->database = nullptr;
		this->noTables = 0;
		this->tableNames = nullptr;
		this->tableNames = new TableNames();
	}
	//DESTRUCTOR
	~Database() {
		for (int i = 0; i < noTables; i++) {
			delete database[i];
		}
		delete[] database;
		delete tableNames;  //i need to delete because i used 'new' in the constructor
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

		cout << endl << "Values inserted into table '" << name << "' successfully.";
	}
	void deleteColumnFromTable(const string& tableName, const string& columnName) {
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

		table->deleteColumn(columnName);

		cout << endl << "Column '" << columnName << "' deleted from table '" << tableName << "' successfully.";
	}
	void selectALL(const string& name) const {
		if (!tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		int index = getTableIndex(name);
		database[index]->displayTable();
	}
	void selectWHERE(const string& tableName, const string& columnName) {
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

		ColumnType columnType = table->getColumn(columnName).getType();
		int columnIndex = table->getColumnIndex(columnName);

		cout << endl << columnName;
		cout << endl << "-----------------" << endl;

		for (int i = 0; i < table->getNoRows(); i++) {
			const Row& row = table->getRow(i);
			switch (columnType) {
			case INT:
				cout << row.getIntData(columnIndex) << endl;
				break;
			case TEXT:
				cout << row.getTextData(columnIndex) << endl;
				break;
			case FLOAT:
				cout << row.getFloatData(columnIndex) << endl;
				break;
			default:
				cout << endl << "Error: Unsupported column type.";
			}
		}
	}
	void selectColumns(const string& tableName, const string* columnNames, int noColumns) {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		cout << endl;
		for (int i = 0; i < noColumns; i++) {
			cout << columnNames[i] << "\t\t";
		}
		cout << endl << "-------------------------------------------------------------" << endl;

		for (int i = 0; i < table->getNoRows(); i++) {
			Row& row = table->getRow(i);
			for (int j = 0; j < noColumns; j++) {
				const Column& column = table->getColumn(columnNames[j]);
				switch (column.getType()) {
				case INT:
					cout << row.getIntData(table->getColumnIndex(columnNames[j])) << "\t\t";
					break;
				case TEXT:
					cout << row.getTextData(table->getColumnIndex(columnNames[j])) << "\t\t";
					break;
				case FLOAT:
					cout << row.getFloatData(table->getColumnIndex(columnNames[j])) << "\t\t";
					break;
				default:
					cout << "N/A" << "\t\t";
				}
			}
			cout << endl;
		}
	}
	void updateTable(const string& tableName, const string& setColumnName, const string& setValue, const string& whereColumnName, const string& whereValue) {
		if (!tableExists(tableName)) {
			cout << endl << "Error: Table '" << tableName << "' does not exist.";
			return;
		}

		int tableIndex = getTableIndex(tableName);
		Table* table = database[tableIndex];

		int setColumnIndex = table->getColumnIndex(setColumnName);
		int whereColumnIndex = table->getColumnIndex(whereColumnName);

		if (!table->columnExists(setColumnName)) {
			cout << endl << "Error: Column '" << setColumnName << "' does not exist in table '" << tableName << "'.";
			return;
		}

		if (!table->columnExists(whereColumnName)) {
			cout << endl << "Error: Column '" << whereColumnName << "' does not exist in table '" << tableName << "'.";
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

		cout << endl << "Updated " << updatedRows << " rows in table '" << tableName << "' by setting " << setColumnName << " to '" << setValue << "' where " << whereColumnName << " is '" << whereValue << "'.";;
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
	//--------------------------------------------------
};

class Commands {
private:
	Database* db = nullptr;
public:
	//DEFAULT CONSTRUCTOR
	Commands() {
		this->db = nullptr;
	}
	//CONSTRUCTOR only this will be public later
	Commands(Database* database) : db(database) {}
	//HELPER FUNCTIONS
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
	//--------------------------------------------------
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
public:
	void stringCommandSelectAll(const string& command) {
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
	void stringCommandCreateTable(const string& command) {
		try {
			string commandCopy = command;
			trim(commandCopy);

			//check if command starts with "CREATE TABLE "
			if (commandCopy.find("CREATE TABLE ") != 0) {
				//or without a space after "TABLE"
				if (commandCopy.find("CREATE TABLE") == 0) {
					cout << endl << "Invalid command format.";
				}
				else {
					cout << endl << "Invalid command format.";
				}
				return;
			}

			//find the position of the first '('
			size_t pos = commandCopy.find("(");
			if (pos == string::npos) {  //if it reached the end of the string and did not find '('
				cout << endl << "Invalid command format. Missing '('.";
				return;
			}

			//get the table name
			string tableName = commandCopy.substr(13, pos - 13);  // 13 is the length of "CREATE TABLE " with a space after
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

			//find the position of the last ')'
			size_t endPos = commandCopy.find_last_of(")");
			if (endPos == string::npos) {
				cout << endl << "Invalid command format. Missing ')'.";
				return;
			}
			//--------------------------------------------------
			//GET THE COLUMNS
			string columnsPart = commandCopy.substr(pos + 1, endPos - pos - 1);  // -1 to exclude the last ')' // from after '(' to before ')'
			trim(columnsPart);

			if (columnsPart.empty()) {
				cout << endl << "Invalid command format. Columns cannot be empty.";
				return;
			}

			//split the columns part into individual columns
			string* columns = nullptr;
			int noColumns = 0;
			splitCommand(columnsPart, ",", columns, noColumns);

			if (noColumns == 0) {
				cout << endl << "Invalid command format. No columns specified.";
				return;
			}

			Column* tableColumns = new Column[noColumns];
			for (int i = 0; i < noColumns; i++) {
				string column = columns[i];
				trim(column);

				//split the column into individual parts
				string* columnParts = nullptr;
				int noParts = 0;
				splitCommand(column, " ", columnParts, noParts);

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

				//get column size
				string columnSize = columnParts[2];
				trim(columnSize);

				if (columnSize.empty()) {
					cout << endl << "Invalid command format. Column size cannot be empty.";
					delete[] columnParts;
					delete[] columns;
					delete[] tableColumns;
					return;
				}

				//get column default value
				string columnDefaultValue = columnParts[3];
				trim(columnDefaultValue);

				if (columnDefaultValue.empty()) {
					cout << endl << "Invalid command format. Column default value cannot be empty.";
					delete[] columnParts;
					delete[] columns;
					delete[] tableColumns;
					return;
				}

				//check for extra arguments
				size_t extraArgsPos = columnDefaultValue.find(' ');
				if (extraArgsPos != string::npos) {
					cout << endl << "Invalid command format. Too many arguments.";
					delete[] columnParts;
					delete[] columns;
					delete[] tableColumns;
					return;
				}

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
		catch (const invalid_argument& e) {  //handle any error like invalid column type or size etc.
			cout << endl << e.what();
		}
	}
	void stringCommandDropTable(const string& command) {
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
	void stringCommandDescribe(const string& command) {
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
};

//HANDLE ERRORS IN EACH FUNCTION

int main() {
	//fix display later
	Database db;
	Commands commands(&db);

	//db.createTable("Products", new Column[3]{ Column("ID", INT, 5, "0", true), Column("Name", TEXT, 20, ""), Column("Price", FLOAT, 10, "0.0f") }, 3);
	//db.createTable("Products", new Column[3]{ Column("ID", INT, 5, "0", true), Column("Name", TEXT, 20, ""), Column("Price", FLOAT, 10, "0.0f") }, 3);
	//db.insertIntoTable("Products", new string[3]{ "1", "Laptop", "999.99" }, 3);
	//db.insertIntoTable("Products", new string[3]{ "2", "Mouse", "129.99" }, 3);
	//db.insertIntoTable("Products", new string[3]{ "2", "Mouse", "129.99" }, 3); //error because not unique id
	//db.insertIntoTable("Products", new string[3]{ "2", "Mouseddddddddddddddddddd", "129.99" }, 3); //error because name too long
	//db.deleteColumnFromTable("Products", "Price");
	//db.selectALL("Products");
	//db.describeTable("Products");
	//db.selectWHERE("Products", "Name");
	//db.updateTable("Products", "Name", "test", "Name", "Laptop");
	//db.alterTableAddColumn("Products", Column("Stock", INT, 5, "0"));
	//db.selectALL("Products");
	//db.selectColumns("Products", new string[2]{ "ID", "Name" }, 2);
	//db.dropTable("Products");

	string create = "CREATE TABLE Products (ID INT 5 0 UNIQUE, Name TEXT 20 unknown, Price FLOAT 10 0.0f)";
	commands.stringCommandCreateTable(create);
	string select = "SELECT ALL FROM Products";
	commands.stringCommandSelectAll(select);
	string describe = "DESCRIBE Products";
	commands.stringCommandDescribe(describe);
	//string drop = "DROP TABLE Products";
	//commands.stringCommandDropTable(drop);

	return 0;
}