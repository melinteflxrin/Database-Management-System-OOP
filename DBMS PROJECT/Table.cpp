#include "Table.h"
#include "Column.h"
#include "Row.h"
#include <iostream>
#include <string>

//private methods
string Table::getColumnTypeName(ColumnType type) const {     //for describe table
	switch (type) {
	case INT: return "INT";
	case TEXT: return "TEXT";
	case FLOAT: return "FLOAT";
	case BOOLEAN: return "BOOLEAN";
	case DATE: return "DATE";
	default: return "UNKNOWN";
	}
}

//public methods
//DEFAULT CONSTRUCTOR
Table::Table() {
	this->name = "";
	this->columns = nullptr;
	this->noColumns = 0;
	this->rows = nullptr;
	this->noRows = 0;
}
//CONSTRUCTOR
Table::Table(const string& name, const Column* columns, int noColumns) {
	this->setName(name);
	this->setColumns(columns, noColumns);
}
//COPY CONSTRUCTOR
Table::Table(const Table& original) : name(original.name), noColumns(original.noColumns), noRows(original.noRows) {  //I copy FROM original  //i create an object based on another object
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
Table& Table::operator=(const Table& copy) {
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
Table::~Table() {
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
void Table::setName(const string& name) {
	if (name.empty() || name.size() < 2) {
		throw invalid_argument("Name cannot be empty or less than two characters.");
	}
	this->name = name;
}
void Table::setColumns(const Column* columns, int noColumns) {   //no need for Column** because Im not passing an array of pointers; Im passing a simple array that can be accessed linearly
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
const string& Table::getName() const {
	return this->name;
}
int Table::getNoColumns() const {
	return this->noColumns;
}
int Table::getNoRows() const {
	return this->noRows;
}
const Column& Table::getColumn(int index) const {
	if (index < 0 || index >= this->noColumns) {
		throw out_of_range("Invalid column index.");
	}
	return *(this->columns[index]);
}
const Column& Table::getColumn(const string& name) const {
	for (int i = 0; i < noColumns; i++) {
		if (columns[i]->getName() == name) {
			return *columns[i];
		}
	}
	throw invalid_argument("Column not found.");
}
const ColumnType Table::getColumnType(int index) const {
	if (index < 0 || index >= this->noColumns) {
		throw out_of_range("Invalid column index.");
	}
	return this->columns[index]->getType();
}
const int Table::getColumnSize(int index) const {
	if (index < 0 || index >= this->noColumns) {
		throw out_of_range("Invalid column index.");
	}
	return this->columns[index]->getSize();
}
int Table::getColumnIndex(const string& name) const {
	for (int i = 0; i < noColumns; i++) {
		if (columns[i]->getName() == name) {
			return i;
		}
	}
	throw invalid_argument("Column not found.");
}
Row& Table::getRow(int index) const {
	if (index < 0 || index >= this->noRows) {
		throw out_of_range("Invalid row index.");
	}
	return *this->rows[index];
}

//ROWS
void Table::deleteRow(int index) {
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
void Table::addRow(const string* values) {
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
bool Table::addRowBool(const string* values) {
	if (values == nullptr) {
		cout << endl << "Error: Row values cannot be null.";
		return false; // Indicate failure
	}

	Row* newRow = new Row(this->noColumns);

	for (int i = 0; i < this->noColumns; i++) {
		const Column& column = this->getColumn(i);

		// Check if the value is UNIQUE
		if (column.isUnique()) {
			for (int j = 0; j < this->noRows; j++) {
				if (this->rows[j]->getTextData(i) == values[i]) {
					cout << endl << "Error: Value for column '" << column.getName() << "' must be unique.";
					delete newRow; // Clean up allocated memory
					return false; // Indicate failure
				}
			}
		}

		// Check if the value exceeds the maximum size
		if (values[i].size() > column.getSize()) {
			cout << endl << "Error: Value for column '" << column.getName()
				<< "' exceeds the maximum size of " << column.getSize() << ".";
			delete newRow; // Clean up allocated memory
			return false; // Indicate failure
		}

		// Set the value based on the column type
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
			newRow->setStringData(i, values[i]); // DATE AS STRING
			break;
		default:
			cout << endl << "Error: Unsupported column type.";
			delete newRow; // Clean up allocated memory
			return false; // Indicate failure
		}
	}

	// Add the new row to the table
	Row** tempRows = new Row * [this->noRows + 1];
	for (int i = 0; i < this->noRows; ++i) {
		tempRows[i] = this->rows[i];
	}
	tempRows[this->noRows] = newRow;

	delete[] this->rows;
	this->rows = tempRows;
	this->noRows++;

	cout << endl << "Values inserted into table '" << name << "' successfully.";
	return true; // Indicate success
}

void Table::addRowWithoutPrintMessage(const string* values) {
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
}

//COLUMNS
bool Table::columnExists(const string& name) const {
	for (int i = 0; i < noColumns; i++) {
		if (columns[i]->getName() == name) {
			return true;
		}
	}
	return false;
}
bool Table::columnExistsByIndex(int index) const {
	if (index < 0 || index >= noColumns) {
		return false;
	}
	return true;
}
void Table::deleteColumn(const string& name) {
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
void Table::deleteColumnByIndex(int index) {
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
void Table::addColumn(const Column& newColumn) {
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

//DISPLAY
void Table::describeTable() const {
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
void Table::displayTable() const {
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