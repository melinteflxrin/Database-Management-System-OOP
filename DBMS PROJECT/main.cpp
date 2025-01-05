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
	//CONSTRUCTOR
	Column(const string& name, ColumnType type, int size, const string& defaultValue) {
		this->setName(name);
		this->setType(type);
		this->setSize(size);
		this->setDefaultValue(defaultValue);
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
	//CONSTRUCTOR
	TableNames() {
		names = new string[noNames];
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
	//DESTRUCTOR
	~TableNames() {
		delete[] names;
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
	//CONSTRUCTOR
	Table(const string& name, const Column* columns, int noColumns) : name(name), noColumns(noColumns) {
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
		cout << endl << "Column\t\tType\t\tSize\t\tDefault value";
		cout << endl << "-------------------------------------------------------------";

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
				cout << getColumn(i).getDefaultValue() << endl;
			}
			else if (getColumn(i).getType() == TEXT) {
				cout << getColumn(i).getDefaultValue() << endl;
			}
			else if (getColumn(i).getType() == FLOAT) {
				cout << getColumn(i).getDefaultValue() << endl;
			}
			else {
				cout << "N/A" << endl;
			}
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
	//CONSTRUCTOR
	Database() {
		tableNames = new TableNames();
	}
	//DESTRUCTOR
	~Database() {
		for (int i = 0; i < noTables; i++) {
			delete database[i];
		}
		delete[] database;
		delete tableNames;
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
	//--------------------------------------------------
	void describeTable(const string& name) const {
		if (!tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		int index = getTableIndex(name);
		database[index]->describeTable();
	}
	bool tableExists(const string& name) const {
		if (tableNames->nameExists(name)) {
			return true;
		}
		return false;
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
	//--------------------------------------------------
};

int main() {
	//fix display later
	Database db;
	db.createTable("Products", new Column[3]{ Column("ID", INT, 5, "0"), Column("Name", TEXT, 20, ""), Column("Price", INT, 10, "0.0f") }, 3);
	db.createTable("Products", new Column[3]{ Column("ID", INT, 5, "0"), Column("Name", TEXT, 20, ""), Column("Price", INT, 10, "0.0f") }, 3);
	return 0;
}