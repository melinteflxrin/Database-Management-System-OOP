#include <iostream>
#include <string.h>
#include <string>
//#include <stdexcept> //FOR EXCEPTIONS invalid_argument, out_of_range

using namespace std;

enum ColumnType { INT, TEXT, FLOAT, BOOLEAN, DATE };

class Column {
private:
	string name = "";
	ColumnType type = TEXT;
	int size = 0;
	string defaultValue;

public:
	//SETTERS
	void setName(const string name) {
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
	void setDefaultValue(const string defaultValue) {                  //DO DEFAULT VALUE ACCORDING TO TYPE LATER
		this->defaultValue = defaultValue;
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
	//COPY CONSTRUCTOR
	Row(const Row& original) : noColumns(original.noColumns) {
		data = new string[noColumns];
		for (int i = 0; i < noColumns; ++i) {
			data[i] = original.data[i];  // deep copy
		}
	}
	//ASSIGNMENT OPERATOR
	Row& operator=(const Row& original) {
		if (this == &original) return *this; // Self-assignment check

		// Free existing resources
		delete[] data;

		noColumns = original.noColumns;
		data = new string[noColumns];
		for (int i = 0; i < noColumns; ++i) {
			data[i] = original.data[i];
		}
		return *this;
	}
	//CONSTRUCTOR
	Row(int noColumns) {
		this->noColumns = noColumns;
		data = new string[noColumns];
	}
	//DESTRUCTOR
	~Row() {
		if (data != nullptr) {
			delete[] data;
		}
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

		return stoi(data[columnIndex]);
	}
	string getTextData(int columnIndex) const {
		if (columnIndex < 0 || columnIndex >= noColumns)
			throw out_of_range("Column index out of range.");

		return data[columnIndex];
	}
	float getFloatData(int columnIndex) const {
		if (columnIndex < 0 || columnIndex >= noColumns)
			throw out_of_range("Column index out of range.");

		return stof(data[columnIndex]);
	}
};

class Index {  //create an index only on a single column
private:
	string name = "";
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
};

class Table {
private:
	string name = "";
	Column** columns = nullptr;           // array of pointers to Column objects so i can use their constructors because i kept getting the no default constructor error
	int noColumns = 0;                    //cant use the actual column because i dont have a column constructor inside the table class; thats why Column**
	Row** rows = nullptr;
	int noRows = 0;

	TableNames* tableNames = nullptr;

public:
	//CONSTRUCTOR
	Table(const string& name, const Column* columns, int noColumns, TableNames* tableNames) : name(name), noColumns(noColumns), tableNames(tableNames) {
		this->setName(name);
		this->setNoColumns(noColumns);
		this->setColumns(columns, noColumns);

		if (tableNames != nullptr && tableNames->nameExists(this->getName())) {
			cout << "Error: Cannot create table. Table with this name already exists." << endl;
			return;
		}

		//add the table name to TableNames
		tableNames->addName(name);
	}

	//COPY CONSTRUCTOR
	Table(const Table& original) : name(original.name), noColumns(original.noColumns), noRows(original.noRows), tableNames(original.tableNames) {  //I copy FROM original  //i create an object based on another object
		if (original.tableNames) {
			this->tableNames->addName(original.name);
		}
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
		//
		if (copy.tableNames) {
			this->tableNames = new TableNames(*copy.tableNames);
		}
		else {
			this->tableNames = nullptr;
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

	//SETTERS
	void setName(const string& name) {
		if (name.empty() || name.size() < 2) {
			throw invalid_argument("Name cannot be empty or less than two characters.");
		}
		this->name = name;
	}
	void setNoColumns(int noColumns) {
		if (noColumns < 1) {
			throw invalid_argument("Number of columns must be larger than zero.");
		}
		this->noColumns = noColumns;
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
	//--------------------------------------------------
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
	const Row& getRow(int index) const {
		if (index < 0 || index >= this->noRows) {
			throw out_of_range("Invalid row index.");
		}
		return *this->rows[index];
	}
	//--------------------------------------------------

	void addRow() {   //with default values
		Row* newRow = new Row(this->noColumns);  //create a new row with the same number of columns as the table

		for (int i = 0; i < this->noColumns; ++i) {  //set default values for each column
			Column& column = *this->columns[i];

			switch (column.getType()) {
			case INT:
				newRow->setIntData(i, stoi(column.getDefaultValue()));
				break;
			case TEXT:
				newRow->setStringData(i, column.getDefaultValue());
				break;
			case FLOAT:
				newRow->setFloatData(i, stof(column.getDefaultValue()));
				break;
			case BOOLEAN:
				newRow->setStringData(i, column.getDefaultValue() == "true" ? "true" : "false");
				break;
			case DATE:
				newRow->setStringData(i, column.getDefaultValue()); //DATE AS STRING
				break;
			default:
				throw invalid_argument("Unsupported column type.");
			}
		}

		Row** tempRows = new Row * [this->noRows + 1];  //actually adding the new row to the table

		for (int i = 0; i < this->noRows; ++i) {  //copy the existing rows
			tempRows[i] = this->rows[i];
		}

		tempRows[this->noRows] = newRow;  //adding the new row

		delete[] this->rows;
		this->rows = tempRows;
		this->noRows++;
	}

	void addRowWithValues(const string* values) {
		if (values == nullptr) {
			cout << "Error: row values cannot be null." << endl;
			return;
		}
		Row* newRow = new Row(this->noColumns);

		for (int i = 0; i < this->noColumns; i++) {
			const Column& column = this->getColumn(i);

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
				cout << "Error: Unsupported column type." << endl;
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
	//ADD ROW AND FREE MEMORY
	void addRow(string* values) {
		this->addRowWithValues(values);
		delete[] values;
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

//class Database {          //many tables
//private:
//	Table** database = nullptr;
//	int noTables = 0;
//public:
//	//DESTRUCTOR
//	~Database() {
//		for (int i = 0; i < noTables; i++) {
//			delete database[i];
//		}
//		delete[] database;
//	}
//
//	//GETTERS
//	int getTableIndex(const string name) const {
//		for (int i = 0; i < noTables; i++) {
//			if (database[i]->getName() == name) {
//				return i;
//			}
//		}
//		return -1;
//	}
//	bool tableExists(const string name)const {
//		for (int i = 0; i < noTables; i++) {
//			if (database[i]->getName() == name) {
//				return true;
//			}
//		}
//		return false;
//	}
//	//**************************************
//	void addTableToDatabase(const Table& table) {
//		if (tableExists(table.getName())) {
//			cout << endl << "Table " << table.getName() << " already exists.";
//			return;
//		}
//
//		Table** newDatabase = new Table * [noTables + 1];
//
//		for (int i = 0; i < noTables; i++) {
//			newDatabase[i] = new Table(*database[i]);     //i am copying pointers from database to newDatabase, not the actual Table objects
//		}
//		newDatabase[noTables] = new Table(table); // add new table at the end        //i need a custom copy constructor to perform deep copy or else the default one performs shallow copy
//		for (int i = 0; i < noTables; i++) {
//			delete database[i];  // Delete the table objects to free memory
//		}
//		delete[] database;
//		database = newDatabase;  //point to the new array with increased size
//		noTables++;
//		cout << endl << "Table " << table.getName() << " added successfully.";
//	}
//	void displayTables() const {
//		for (int i = 0; i < noTables; i++) {
//			database[i]->displayTable();
//		}
//	}
//	void describeTables() const {
//		for (int i = 0; i < noTables; i++) {
//			database[i]->describeTable();
//		}
//	}
//	void dropTable(const string name) {
//		if (tableExists(name) == false) {
//			cout << endl << "Table " << name << " not found.";
//			return;
//		}
//
//		delete database[getTableIndex(name)];
//
//		for (int i = getTableIndex(name); i < noTables; i++) {
//			database[i] = database[i + 1];    // move all elements to the left with 1 because we removed one
//		}
//		noTables--;
//		database[noTables] = nullptr;
//	}
//};

int main() {
	TableNames* tableNames = new TableNames();

	Column columns[] = {
		Column("ID", INT, 5, "0"),
		Column("Name", TEXT, 20, ""),
		Column("Price", INT, 10, "0.0f")
	};

	Table table("Products", columns, 3, tableNames);
	Table table2("Products", columns, 3, tableNames);

	table.addRow(new string[3]{ "1", "Product 1", "10" });
	table.addRow(new string[3]{ "2", "Product 2", "100" });
	table.addRow();

	table.describeTable();
	table.displayTable();

	table.addColumn(Column("Weight", FLOAT, 10, "0.5f"));
	table.addColumn(Column("Available", INT, 10, "0"));         //daca e numele prea lung se strica si la describeTable

	table.describeTable();
	table.displayTable();

	//Table table1("Products", columns, 3, tableNames);

	delete tableNames;

	//convert to int and check if its actually int
	//In computer programming, an integer can be thought of as a value with nothing but 0 after the decimal point. If round(x,0) = x, then x is an integer.
	return 0;
}