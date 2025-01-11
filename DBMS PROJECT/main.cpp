#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "Database.h"

#include "Command.h"
#include "selectCommands.h"
#include "helpCommands.h"

#include "HelpersForSelectCommands.h"
#include "stringUtils.h"

using namespace std;

class createTable : public Command {
private:
	string name;
	Column* columns;
	int noColumns;
public:
	~createTable() {
		delete[] columns;
	}
	createTable() {
		this->name = "";
		this->columns = nullptr;
		this->noColumns = 0;
	}
	createTable(const string& name, Column* columns, int noColumns) {
		this->name = name;
		this->noColumns = noColumns;
		this->columns = new Column[noColumns];
		for (int i = 0; i < noColumns; i++) {
			this->columns[i] = columns[i];
		}
	}
	createTable(const createTable& ct) {
		this->name = ct.name;
		this->noColumns = ct.noColumns;
		this->columns = new Column[ct.noColumns];
		for (int i = 0; i < ct.noColumns; i++) {
			this->columns[i] = ct.columns[i];
		}
	}
	createTable& operator=(const createTable& ct) {
		if (this == &ct) {
			return *this;
		}
		this->name = ct.name;
		this->noColumns = ct.noColumns;
		this->columns = new Column[ct.noColumns];
		for (int i = 0; i < ct.noColumns; i++) {
			this->columns[i] = ct.columns[i];
		}

		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		//check
		if (db.tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " already exists.";
			return;
		}

		//create
		Table* newTable = new Table(name, columns, noColumns);

		//add
		db.addTableToDatabase(*newTable);

		delete newTable;

		cout << endl << "Table " << "'" << name << "'" << " created successfully.";
	}
	static createTable parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if command starts with "CREATE TABLE "
		if (commandCopy.find("CREATE TABLE ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		//find the position of the first '('
		size_t pos = commandCopy.find("(");
		if (pos == string::npos) {
			throw invalid_argument("Invalid command format. Missing '('.");
		}

		//get the table name
		string tableName = commandCopy.substr(13, pos - 13);  // 13 is the length of "CREATE TABLE "
		stringUtils::trim(tableName);

		//check if the table name is empty
		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		//check if the table name contains spaces
		if (tableName.find(' ') != string::npos) {
			throw invalid_argument("Invalid command format. Table name cannot contain spaces.");
		}

		//find the position of the last ')'
		size_t endPos = commandCopy.find_last_of(")");
		if (endPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing ')'.");
		}

		//get the columns part
		string columnsPart = commandCopy.substr(pos + 1, endPos - pos - 1);  // from after '(' to before ')'
		stringUtils::trim(columnsPart);

		if (columnsPart.empty()) {
			throw invalid_argument("Invalid command format. Columns cannot be empty.");
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
			throw invalid_argument("Invalid command format. No columns specified.");
		}

		//split the columns part into individual columns
		string* columns = new string[noColumns];
		start = 0;
		int colIndex = 0;
		while ((end = columnsPart.find("),", start)) != string::npos) {
			string column = columnsPart.substr(start, end - start + 1);
			stringUtils::trim(column);
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
		stringUtils::trim(lastColumn);
		columns[colIndex] = lastColumn;

		Column* tableColumns = new Column[noColumns];
		for (int i = 0; i < noColumns; i++) {
			string column = columns[i];
			stringUtils::trim(column);

			//remove leading and trailing parentheses
			if (!column.empty() && column.front() == '(') column.erase(0, 1);
			if (!column.empty() && column.back() == ')') column.pop_back();

			//split the column into individual parts
			string* columnParts = nullptr;
			int noParts = 0;
			stringUtils::splitCommand(column, ",", columnParts, noParts);

			if (noParts < 4 || noParts > 5) {
				delete[] columnParts;
				delete[] columns;
				delete[] tableColumns;
				throw invalid_argument("Invalid command format. Invalid column format.");
			}

			//get the column name
			string columnName = columnParts[0];
			stringUtils::trim(columnName);

			if (columnName.empty()) {
				delete[] columnParts;
				delete[] columns;
				delete[] tableColumns;
				throw invalid_argument("Invalid command format. Column name cannot be empty.");
			}

			//get the column type
			string columnType = columnParts[1];
			stringUtils::trim(columnType);

			ColumnType type;
			try {
				type = stringUtils::parseColumnType(columnType);
			}
			catch (const invalid_argument& e) {
				delete[] columnParts;
				delete[] columns;
				delete[] tableColumns;
				throw;
			}

			//size
			string columnSize = columnParts[2];
			stringUtils::trim(columnSize);

			if (columnSize.empty()) {
				delete[] columnParts;
				delete[] columns;
				delete[] tableColumns;
				throw invalid_argument("Invalid command format. Column size cannot be empty.");
			}

			//default value
			string columnDefaultValue = columnParts[3];
			stringUtils::trim(columnDefaultValue);

			//check if column is unique
			bool unique = false;
			if (noParts == 5) {
				string uniquePart = columnParts[4];
				stringUtils::trim(uniquePart);
				if (uniquePart == "UNIQUE") {
					unique = true;
				}
				else {
					delete[] columnParts;
					delete[] columns;
					delete[] tableColumns;
					throw invalid_argument("Invalid command format. Invalid column format.");
				}
			}

			//create the column
			tableColumns[i] = Column(columnName, type, stoi(columnSize), columnDefaultValue, unique);
		}

		delete[] columns;
		return createTable(tableName, tableColumns, noColumns);
	}
};
class describeTable : public Command {
private:
	string name;
public:
	describeTable() {
		this->name = "";
	}
	describeTable(const string& name) {
		this->name = name;
	}
	describeTable(const describeTable& dt) {
		this->name = dt.name;
	}
	describeTable& operator=(const describeTable& dt) {
		if (this == &dt) {
			return *this;
		}
		this->name = dt.name;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		if (!db.tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		try {
			//get a copy of the table from the database
			Table table = db.getTableByName(name);

			table.describeTable();
		}
		catch (const runtime_error& e) {
			cout << endl << "Error: " << e.what();
		}
	}
	static describeTable parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command starts with "DESCRIBE "
		if (commandCopy.find("DESCRIBE ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		//get the table name
		string tableName = commandCopy.substr(9);  // 9 is the length of "DESCRIBE " with a space after
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		//check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		//return a new describeTable object
		return describeTable(tableName);
	}
};
class dropTable : public Command {
private:
	string name;
	string tablesConfigAddress;
public:
	dropTable() {
		this->name = "";
		this->tablesConfigAddress = "";
	}
	dropTable(const string& name, const string& tablesConfigAddress) {
		this->name = name;
		this->tablesConfigAddress = tablesConfigAddress;
	}
	dropTable(const dropTable& dt) {
		this->name = dt.name;
		this->tablesConfigAddress = dt.tablesConfigAddress;
	}
	dropTable& operator=(const dropTable& dt) {
		if (this == &dt) {
			return *this;
		}
		this->name = dt.name;
		this->tablesConfigAddress = dt.tablesConfigAddress;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.dropTable(name, tablesConfigAddress);
	}
	static dropTable parseCommand(const string& command, const string& tablesConfigAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "DROP TABLE "
		if (commandCopy.find("DROP TABLE ") != 0) {
			if (commandCopy.find("DROP TABLE") == 0) {
				cout << endl << "Invalid command format.";
			}
			else {
				cout << endl << "Invalid command format.";
			}
			throw invalid_argument("Invalid command format.");
		}

		// Get the table name
		string tableName = commandCopy.substr(11);  // 11 is the length of "DROP TABLE " with a space after
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			cout << endl << "Invalid command format. Too few arguments.";
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		// Check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			cout << endl << "Invalid command format. Too many arguments.";
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		return dropTable(tableName, tablesConfigAddress);
	}
};
class InsertCommand : public Command {
private:
	string tableName;
	string* values;
	int noValues;

public:
	InsertCommand() : tableName(""), values(nullptr), noValues(0) {}
	InsertCommand(const string& tableName, const string* values, int noValues)
		: tableName(tableName), values(new string[noValues]), noValues(noValues) {
		for (int i = 0; i < noValues; ++i) {
			this->values[i] = values[i];
		}
	}
	InsertCommand(const InsertCommand& other)
		: tableName(other.tableName), values(new string[other.noValues]), noValues(other.noValues) {
		for (int i = 0; i < noValues; ++i) {
			this->values[i] = other.values[i];
		}
	}
	InsertCommand& operator=(const InsertCommand& other) {
		if (this == &other) {
			return *this;
		}

		delete[] values;

		tableName = other.tableName;
		noValues = other.noValues;
		values = new string[noValues];
		for (int i = 0; i < noValues; ++i) {
			this->values[i] = other.values[i];
		}

		return *this;
	}
	~InsertCommand() {
		delete[] values;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.insertValues(tableName, values, noValues);
	}
	static InsertCommand parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "INSERT INTO "
		if (commandCopy.find("INSERT INTO ") != 0) {
			throw std::invalid_argument("Invalid command format: Missing 'INSERT INTO'.");
		}

		// Find the position of "VALUES "
		size_t pos = commandCopy.find("VALUES ");
		if (pos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing 'VALUES'.");
		}

		// Get the table name
		string tableName = commandCopy.substr(12, pos - 12);  // 12 is the length of "INSERT INTO " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw std::invalid_argument("Invalid command format: Table name cannot be empty.");
		}

		// Find the position of the first '('
		size_t startPos = commandCopy.find("(");
		if (startPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing '('.");
		}

		// Find the position of the last ')'
		size_t endPos = commandCopy.find_last_of(")");
		if (endPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing ')'.");
		}

		// Get the values part
		string valuesPart = commandCopy.substr(startPos + 1, endPos - startPos - 1);  // from after '(' to before ')'
		stringUtils::trim(valuesPart);

		if (valuesPart.empty()) {
			throw std::invalid_argument("Invalid command format: Values cannot be empty.");
		}

		// Split the values part into individual values
		string* values = nullptr;
		int noValues = 0;
		stringUtils::splitCommand(valuesPart, ",", values, noValues);

		if (noValues == 0) {
			throw std::invalid_argument("Invalid command format: No values specified.");
		}

		// Trim each value individually
		for (int i = 0; i < noValues; i++) {
			stringUtils::trim(values[i]);
		}

		InsertCommand insertCommand(tableName, values, noValues);
		delete[] values;
		return insertCommand;
	}
};
class createIndex : public Command {
private:
	string indexName;
	string tableName;
	string columnName;
public:
	createIndex() {
		this->indexName = "";
		this->tableName = "";
		this->columnName = "";
	}
	createIndex(const string& indexName, const string& tableName, const string& columnName) {
		this->indexName = indexName;
		this->tableName = tableName;
		this->columnName = columnName;
	}
	createIndex(const createIndex& ci) {
		this->indexName = ci.indexName;
		this->tableName = ci.tableName;
		this->columnName = ci.columnName;
	}
	createIndex& operator=(const createIndex& ci) {
		if (this == &ci) {
			return *this;
		}
		this->indexName = ci.indexName;
		this->tableName = ci.tableName;
		this->columnName = ci.columnName;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.createIndex(indexName, tableName, columnName);
	}
	static createIndex parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "CREATE INDEX "
		if (commandCopy.find("CREATE INDEX ") != 0) {
			throw std::invalid_argument("Invalid command format: Missing 'CREATE INDEX'.");
		}

		// Find the position of " ON "
		size_t onPos = commandCopy.find(" ON ");
		if (onPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing 'ON'.");
		}

		// Ensure there is an index name between "CREATE INDEX " and " ON "
		if (onPos <= 13) {  // 13 is the length of "CREATE INDEX "
			throw std::invalid_argument("Invalid command format: Missing index name.");
		}

		// Extract the index name
		string indexName = commandCopy.substr(13, onPos - 13);
		stringUtils::trim(indexName);

		if (indexName.empty()) {
			throw std::invalid_argument("Invalid command format: Index name cannot be empty.");
		}

		// Extract the table name and column part
		string tableAndColumnPart = commandCopy.substr(onPos + 4);  // 4 is the length of " ON "
		stringUtils::trim(tableAndColumnPart);

		// Find the position of the opening parenthesis '('
		size_t openParenPos = tableAndColumnPart.find('(');
		if (openParenPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing '('.");
		}

		// Table name
		string tableName = tableAndColumnPart.substr(0, openParenPos);
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw std::invalid_argument("Invalid command format: Table name cannot be empty.");
		}

		// Find the position of the closing parenthesis ')'
		size_t closeParenPos = tableAndColumnPart.find(')', openParenPos);
		if (closeParenPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing closing parenthesis.");
		}

		// Column name
		string columnName = tableAndColumnPart.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
		stringUtils::trim(columnName);

		if (columnName.empty()) {
			throw std::invalid_argument("Invalid command format: Column name cannot be empty.");
		}

		// Check for extra arguments after the closing parenthesis
		if (closeParenPos + 1 < tableAndColumnPart.length()) {
			string extraArgs = tableAndColumnPart.substr(closeParenPos + 1);
			stringUtils::trim(extraArgs);
			if (!extraArgs.empty()) {
				throw std::invalid_argument("Invalid command format: Too many arguments.");
			}
		}

		return createIndex(indexName, tableName, columnName);
	}
};
class deleteWhere : public Command {
private:
	string tableName;
	string conditionColumn;
	string value;
public:
	deleteWhere() {
		this->tableName = "";
		this->conditionColumn = "";
		this->value = "";
	}
	deleteWhere(const string& tableName, const string& conditionColumn, const string& value) {
		this->tableName = tableName;
		this->conditionColumn = conditionColumn;
		this->value = value;
	}
	deleteWhere(const deleteWhere& dw) {
		this->tableName = dw.tableName;
		this->conditionColumn = dw.conditionColumn;
		this->value = dw.value;
	}
	deleteWhere& operator=(const deleteWhere& dw) {
		if (this == &dw) {
			return *this;
		}
		this->tableName = dw.tableName;
		this->conditionColumn = dw.conditionColumn;
		this->value = dw.value;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.deleteRowFromTable(tableName, conditionColumn, value);
	}
	static deleteWhere parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "DELETE FROM "
		if (commandCopy.find("DELETE FROM ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Find the position of "WHERE "
		size_t pos = commandCopy.find("WHERE ");
		if (pos == string::npos) {
			throw invalid_argument("Invalid command format. Missing 'WHERE'.");
		}

		// Make sure there is a space before "WHERE "
		if (commandCopy[pos - 1] != ' ') {
			throw invalid_argument("Invalid command format. Missing space before 'WHERE'.");
		}

		// Get the table name
		string tableName = commandCopy.substr(12, pos - 13);  // 12 is the length of "DELETE FROM " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		// Get the where part
		string wherePart = commandCopy.substr(pos + 6);  // 6 is the length of "WHERE " with a space
		stringUtils::trim(wherePart);

		if (wherePart.empty()) {
			throw invalid_argument("Invalid command format. Where part cannot be empty.");
		}

		// Split the where part into individual parts
		string* whereParts = nullptr;
		int noParts = 0;
		stringUtils::splitCommand(wherePart, "=", whereParts, noParts);

		if (noParts != 2) {
			delete[] whereParts;
			throw invalid_argument("Invalid command format. Invalid where part.");
		}

		// Get the column name
		string columnName = whereParts[0];
		stringUtils::trim(columnName);

		if (columnName.empty()) {
			delete[] whereParts;
			throw invalid_argument("Invalid command format. Column name cannot be empty.");
		}

		// Get the value
		string value = whereParts[1];
		stringUtils::trim(value);

		if (value.empty()) {
			delete[] whereParts;
			throw invalid_argument("Invalid command format. Value cannot be empty.");
		}

		delete[] whereParts;

		return deleteWhere(tableName, columnName, value);
	}
};
class updateTable : public Command {
private:
	string tableName;
	string setColumnName;
	string setValue;
	string whereColumnName;
	string whereValue;
public:
	updateTable() {
		this->tableName = "";
		this->setColumnName = "";
		this->setValue = "";
		this->whereColumnName = "";
		this->whereValue = "";
	}
	updateTable(const string& tableName, const string& setColumnName, const string& setValue, const string& whereColumnName, const string& whereValue) {
		this->tableName = tableName;
		this->setColumnName = setColumnName;
		this->setValue = setValue;
		this->whereColumnName = whereColumnName;
		this->whereValue = whereValue;
	}
	updateTable(const updateTable& ut) {
		this->tableName = ut.tableName;
		this->setColumnName = ut.setColumnName;
		this->setValue = ut.setValue;
		this->whereColumnName = ut.whereColumnName;
		this->whereValue = ut.whereValue;
	}
	updateTable& operator=(const updateTable& ut) {
		if (this == &ut) {
			return *this;
		}
		this->tableName = ut.tableName;
		this->setColumnName = ut.setColumnName;
		this->setValue = ut.setValue;
		this->whereColumnName = ut.whereColumnName;
		this->whereValue = ut.whereValue;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.updateTable(tableName, setColumnName, setValue, whereColumnName, whereValue);
	}
	static updateTable parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "UPDATE "
		if (commandCopy.find("UPDATE ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Find the position of " SET "
		size_t setPos = commandCopy.find(" SET ");
		if (setPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing 'SET'.");
		}

		// Find the position of " WHERE "
		size_t wherePos = commandCopy.find(" WHERE ");
		if (wherePos == string::npos) {
			throw invalid_argument("Invalid command format. Missing 'WHERE'.");
		}

		// Extract the table name
		string tableName = commandCopy.substr(7, setPos - 7);  // 7 is the length of "UPDATE "
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		// Extract the set part
		string setPart = commandCopy.substr(setPos + 5, wherePos - (setPos + 5));  // 5 is the length of " SET "
		stringUtils::trim(setPart);

		if (setPart.empty()) {
			throw invalid_argument("Invalid command format. Set part cannot be empty.");
		}

		// Split the set part into column and value
		size_t equalPos = setPart.find('=');
		if (equalPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing '=' in set part.");
		}

		string setColumnName = setPart.substr(0, equalPos);
		stringUtils::trim(setColumnName);
		string setValue = setPart.substr(equalPos + 1);
		stringUtils::trim(setValue);

		if (setColumnName.empty() || setValue.empty()) {
			throw invalid_argument("Invalid command format. Set column or value cannot be empty.");
		}

		// Extract the condition part
		string conditionPart = commandCopy.substr(wherePos + 7);  // 7 is the length of " WHERE "
		stringUtils::trim(conditionPart);

		if (conditionPart.empty()) {
			throw invalid_argument("Invalid command format. Condition cannot be empty.");
		}

		// Split the condition part into column and value
		size_t conditionEqualPos = conditionPart.find('=');
		if (conditionEqualPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing '=' in condition part.");
		}

		string whereColumnName = conditionPart.substr(0, conditionEqualPos);
		stringUtils::trim(whereColumnName);
		string whereValue = conditionPart.substr(conditionEqualPos + 1);
		stringUtils::trim(whereValue);

		if (whereColumnName.empty() || whereValue.empty()) {
			throw invalid_argument("Invalid command format. Condition column or value cannot be empty.");
		}

		return updateTable(tableName, setColumnName, setValue, whereColumnName, whereValue);
	}
};
class alterAddColumn : public Command {
private:
	string tableName;
	Column newColumn;
public:
	alterAddColumn() {
		this->tableName = "";
		this->newColumn = Column();
	}
	alterAddColumn(const string& tableName, const Column& newColumn) {
		this->tableName = tableName;
		this->newColumn = newColumn;
	}
	alterAddColumn(const alterAddColumn& aac) {
		this->tableName = aac.tableName;
		this->newColumn = aac.newColumn;
	}
	alterAddColumn& operator=(const alterAddColumn& aac) {
		if (this == &aac) {
			return *this;
		}
		this->tableName = aac.tableName;
		this->newColumn = aac.newColumn;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.alterTableAddColumn(tableName, newColumn);
	}
	static alterAddColumn parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "ALTER TABLE "
		if (commandCopy.find("ALTER TABLE ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Find the position of " ADD "
		size_t addPos = commandCopy.find(" ADD ");
		if (addPos == string::npos) {
			throw invalid_argument("Invalid command format.");
		}

		// Check for '(' after " ADD "
		size_t openParenPos = commandCopy.find("(", addPos);
		if (openParenPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing '('.");
		}

		// Extract the table name
		string tableName = commandCopy.substr(12, addPos - 12);  // 12 is the length of "ALTER TABLE "
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		// Extract the column definition part
		string columnDef = commandCopy.substr(openParenPos + 1);  // 1 is the length of "("
		if (columnDef.back() != ')') {
			throw invalid_argument("Invalid command format. Missing ')'.");
		}
		columnDef.pop_back();  // Remove the closing parenthesis
		stringUtils::trim(columnDef);

		// Split the column definition into parts
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
			throw invalid_argument("Invalid command format. Column definition must have 4 parameters.");
		}

		// Trim each part
		for (int i = 0; i < 4; i++) {
			stringUtils::trim(columnParts[i]);
		}

		// Extract column details
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
			throw invalid_argument("Invalid column type.");
		}

		Column newColumn(columnName, columnType, columnSize, defaultValue);

		return alterAddColumn(tableName, newColumn);
	}
};
class alterDropColumn : public Command {
private:
	string tableName;
	string columnName;
public:
	alterDropColumn() {
		this->tableName = "";
		this->columnName = "";
	}
	alterDropColumn(const string& tableName, const string& columnName) {
		this->tableName = tableName;
		this->columnName = columnName;
	}
	alterDropColumn(const alterDropColumn& adc) {
		this->tableName = adc.tableName;
		this->columnName = adc.columnName;
	}
	alterDropColumn& operator=(const alterDropColumn& adc) {
		if (this == &adc) {
			return *this;
		}
		this->tableName = adc.tableName;
		this->columnName = adc.columnName;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.alterTableDeleteColumn(tableName, columnName);
	}
	static alterDropColumn parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "ALTER TABLE "
		if (commandCopy.find("ALTER TABLE ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Find the position of " DROP COLUMN "
		size_t dropPos = commandCopy.find(" DROP COLUMN ");
		if (dropPos == string::npos) {
			throw invalid_argument("Invalid command format.");
		}

		// Extract the table name
		string tableName = commandCopy.substr(12, dropPos - 12);  // 12 is the length of "ALTER TABLE "
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		// Extract the column name
		string columnName = commandCopy.substr(dropPos + 13);  // 13 is the length of " DROP COLUMN "
		stringUtils::trim(columnName);

		if (columnName.empty()) {
			throw invalid_argument("Invalid command format. Column name cannot be empty.");
		}

		return alterDropColumn(tableName, columnName);
	}
};
class dropIndex : public Command {
private:
	string indexName;
public:
	dropIndex() {
		this->indexName = "";
	}
	dropIndex(const string& indexName) {
		this->indexName = indexName;
	}
	dropIndex(const dropIndex& di) {
		this->indexName = di.indexName;
	}
	dropIndex& operator=(const dropIndex& di) {
		if (this == &di) {
			return *this;
		}
		this->indexName = di.indexName;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.dropIndex(indexName);
	}
	static dropIndex parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "DROP INDEX "
		if (commandCopy.find("DROP INDEX ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Extract the index name
		string indexName = commandCopy.substr(11);  // 11 is the length of "DROP INDEX "
		stringUtils::trim(indexName);

		if (indexName.empty()) {
			throw invalid_argument("Invalid command format. Index name cannot be empty.");
		}

		// Check for extra arguments
		size_t extraArgsPos = indexName.find(' ');
		if (extraArgsPos != string::npos) {
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		return dropIndex(indexName);
	}
};
class importCSV : public Command {
private:
	string tableName;
	string csvDirectory;
	string fileName;
	char delimiter;
public:
	importCSV() {
		this->tableName = "";
		this->csvDirectory = "";
		this->fileName = "";
		this->delimiter = ',';
	}
	importCSV(const string& tableName, const string& csvDirectory, const string& fileName, char delimiter)
		: tableName(tableName), csvDirectory(csvDirectory), fileName(fileName), delimiter(delimiter) {}
	importCSV(const importCSV& ic) {
		this->tableName = ic.tableName;
		this->csvDirectory = ic.csvDirectory;
		this->fileName = ic.fileName;
		this->delimiter = ic.delimiter;
	}
	importCSV& operator=(const importCSV& ic) {
		if (this == &ic) {
			return *this;
		}
		this->tableName = ic.tableName;
		this->csvDirectory = ic.csvDirectory;
		this->fileName = ic.fileName;
		this->delimiter = ic.delimiter;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.importCSV(tableName, csvDirectory, fileName, delimiter);
	}
	static importCSV parseCommand(const string& command, const string& csvFilePath, char delimiter) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "IMPORT INTO "
		if (commandCopy.find("IMPORT INTO ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Find the position of the space after "IMPORT INTO "
		size_t pos = commandCopy.find(" ", 12); // 12 is the length of "IMPORT INTO "
		if (pos == string::npos) {
			throw invalid_argument("Invalid command format. Missing table name or file name.");
		}

		// Get the table name
		string tableName = commandCopy.substr(12, pos - 12);
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		// Get the file name
		string fileName = commandCopy.substr(pos + 1);
		stringUtils::trim(fileName);

		if (fileName.empty()) {
			throw invalid_argument("Invalid command format. File name cannot be empty.");
		}

		// Check for valid file extension
		if (fileName.find(".csv") == string::npos && fileName.find(".txt") == string::npos) {
			throw invalid_argument("Invalid file format. Only .csv or .txt files are allowed.");
		}

		// Check if the file exists
		if (!filesystem::exists(csvFilePath)) {
			throw runtime_error("Error: File '" + csvFilePath + "' does not exist.");
		}

		// Import the CSV file into the table
		return importCSV(tableName, csvFilePath, fileName, delimiter);
	}
};

class showTables : public Command {
private:
public:
	void execute(Database& db) override {
		db.showTables();
	}
	static showTables parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command is "SHOW TABLES"
		if (commandCopy != "SHOW TABLES") {
			throw invalid_argument("Invalid command format.");
		}

		return showTables();
	}
};
class showIndexFromTable : public Command {
private:
	string tableName;
public:
	showIndexFromTable() {
		this->tableName = "";
	}
	showIndexFromTable(const string& tableName) {
		this->tableName = tableName;
	}
	showIndexFromTable(const showIndexFromTable& sit) {
		this->tableName = sit.tableName;
	}
	showIndexFromTable& operator=(const showIndexFromTable& sit) {
		if (this == &sit) {
			return *this;
		}
		this->tableName = sit.tableName;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.showIndexFromTable(tableName);
	}
	static showIndexFromTable parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "SHOW INDEX FROM "
		if (commandCopy.find("SHOW INDEX FROM ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Extract the table name
		string tableName = commandCopy.substr(16);  // 16 is the length of "SHOW INDEX FROM "
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		// Check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		return showIndexFromTable(tableName);
	}
};
class showIndexFromAll : public Command {
private:
public:
	void execute(Database& db) override {
		db.showIndexFromAll();
	}
	static showIndexFromAll parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command is "SHOW INDEX FROM ALL"
		if (commandCopy != "SHOW INDEX FROM ALL") {
			throw invalid_argument("Invalid command format.");
		}

		return showIndexFromAll();
	}
};

class selectAll : public selectCommands {
private:
	string name;
public:
	selectAll() : selectCommands() {
		this->name = "";
	}
	selectAll(const string& name, const string& selectCommandsAddress) : selectCommands(selectCommandsAddress) {
		this->name = name;
	}
	selectAll(const selectAll& sa) : selectCommands(sa.selectCommandsAddress) {
		this->name = sa.name;
	}
	selectAll& operator=(const selectAll& sa) {
		if (this == &sa) {
			return *this;
		}
		this->name = sa.name;
		this->selectCommandsAddress = sa.selectCommandsAddress;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		if (!db.tableExists(name)) {
			cout << endl << "Error: Table: " << "'" << name << "'" << " does not exist.";
			return;
		}

		try {
			//get a copy of the table from the database
			Table table = db.getTableByName(name);

			//find a unique file name
			string fileName;
			do {
				selectCount++;
				fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
			} while (filesystem::exists(fileName));

			//redirect cout to a file
			ofstream outFile(fileName);
			if (!outFile) {
				cout << endl << "Error: Could not open file: " << fileName;
				return;
			}
			streambuf* coutBuf = cout.rdbuf(); //save old buffer
			cout.rdbuf(outFile.rdbuf()); //redirect cout to file

			//display table to file
			table.displayTable();

			//restore cout to its original state
			cout.rdbuf(coutBuf);

			//also display on the screen
			table.displayTable();
		}
		catch (const runtime_error& e) {
			cout << endl << "Error: " << e.what();
		}
	}
	static selectAll parseCommand(const string& command, const string& selectCommandsAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command starts with "SELECT ALL FROM "
		if (commandCopy.find("SELECT ALL FROM ") != 0) {
			if (commandCopy.find("SELECT ALL FROM") == 0) {
				throw invalid_argument("Invalid command format.");
			}
			else {
				throw invalid_argument("Invalid command format.");
			}
		}

		//find the position of "FROM " and make sure there is a space after it
		size_t pos = commandCopy.find("FROM ") + 5;  // 5 is the length of "FROM " with the space
		if (pos >= commandCopy.length()) {
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		//get the table name
		string tableName = commandCopy.substr(pos);
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		//check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		//return a new selectAll object
		return selectAll(tableName, selectCommandsAddress);
	}
};
class selectWHERE : public selectCommands {
private:
	string tableName;
	string* columnNames;
	int noColumns;
	string conditionColumn;
	string value;
public:
	selectWHERE() : selectCommands(), columnNames(nullptr), noColumns(0) {}
	//param constructor
	selectWHERE(const string& tableName, const string* columnNames, int noColumns, const string& conditionColumn, const string& value, const string& selectCommandsAddress)
		: selectCommands(selectCommandsAddress), tableName(tableName), noColumns(noColumns), conditionColumn(conditionColumn), value(value) {
		this->columnNames = new string[noColumns];
		for (int i = 0; i < noColumns; ++i) {
			this->columnNames[i] = columnNames[i];
		}
	}
	//cpy constructor
	selectWHERE(const selectWHERE& sw)
		: selectCommands(sw.selectCommandsAddress), tableName(sw.tableName), noColumns(sw.noColumns), conditionColumn(sw.conditionColumn), value(sw.value) {
		this->columnNames = new string[sw.noColumns];
		for (int i = 0; i < sw.noColumns; ++i) {
			this->columnNames[i] = sw.columnNames[i];
		}
	}
	//assignment operator
	selectWHERE& operator=(const selectWHERE& sw) {
		if (this == &sw) {
			return *this;
		}
		this->tableName = sw.tableName;
		this->noColumns = sw.noColumns;
		this->conditionColumn = sw.conditionColumn;
		this->value = sw.value;
		this->selectCommandsAddress = sw.selectCommandsAddress;

		delete[] this->columnNames;
		this->columnNames = new string[sw.noColumns];
		for (int i = 0; i < sw.noColumns; ++i) {
			this->columnNames[i] = sw.columnNames[i];
		}
		return *this;
	}
	//destructor
	~selectWHERE() {
		delete[] columnNames;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		selectCount++;

		// Validate table existence
		if (!db.tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		try {
			// Get a copy of the table from the database
			Table table = db.getTableByName(tableName);

			int conditionColumnIndex = table.getColumnIndex(conditionColumn);
			if (conditionColumnIndex == -1) {
				cout << "Error: Column '" << conditionColumn << "' does not exist in table: " << tableName << endl;
				return;
			}

			int* rows = nullptr;
			int rowCount = 0;

			try {
				// Use the method directly from the Database class
				Index index = db.getIndexByColumnAndTable(conditionColumn, tableName);

				// Use the index to get the rows
				cout << "Using index for column: " << conditionColumn << endl;
				rowCount = index.getRowCount(value);
				if (rowCount > 0) {
					rows = new int[rowCount];
					index.getKeys(value, rows, rowCount);
				}
			}
			catch (const std::runtime_error& e) {
				// Handle the case where the index is not found
				cout << "Scanning table for column '" << conditionColumn << "'." << endl;

				// No index, scan the table
				rowCount = table.getNoRows();
				rows = new int[rowCount];
				int rowIndex = 0;
				for (int i = 0; i < rowCount; i++) {
					if (table.getRow(i).getTextData(conditionColumnIndex) == value) {
						rows[rowIndex++] = i;
					}
				}
				rowCount = rowIndex; // Update count to matched rows
			}

			if (rowCount == 0) {
				cout << "No rows found with " << conditionColumn << " = " << value << endl;
				delete[] rows;
				return;
			}

			// Validate column existence and initialize indexes
			int* columnIndexes = new int[noColumns];
			if (!HelpersForSelectCommands::validateColumns(&table, columnNames, noColumns, columnIndexes)) {
				delete[] columnIndexes;
				delete[] rows;
				return;
			}

			// Calculate column widths
			int* maxWidth = HelpersForSelectCommands::calculateMaxWidths(&table, columnNames, noColumns, columnIndexes);

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
			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
			HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

			for (int i = 0; i < rowCount; i++) {
				const Row& row = table.getRow(rows[i]);
				HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
			}

			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

			// Restore cout
			cout.rdbuf(coutBuf);

			// Print headers and rows to screen
			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
			HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

			for (int i = 0; i < rowCount; i++) {
				const Row& row = table.getRow(rows[i]);
				HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
			}

			HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

			// Cleanup
			delete[] columnIndexes;
			delete[] maxWidth;
			delete[] rows;
		}
		catch (const runtime_error& e) {
			cout << "Error: " << e.what() << endl;
		}
	}
	static selectWHERE parseCommand(const string& command, const string& selectCommandsAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command starts with "SELECT "
		if (commandCopy.find("SELECT ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		//find the position of "FROM "
		size_t fromPos = commandCopy.find("FROM ");
		if (fromPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing 'FROM'.");
		}

		//make sure there is a space before "FROM "
		if (commandCopy[fromPos - 1] != ' ') {
			throw invalid_argument("Invalid command format. Missing space before 'FROM'.");
		}

		//get the columns part
		string columnsPart = commandCopy.substr(7, fromPos - 8);  // 7 is the length of "SELECT " with a space
		stringUtils::trim(columnsPart);

		if (columnsPart.empty()) {
			throw invalid_argument("Invalid command format. Columns cannot be empty.");
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
			stringUtils::trim(columns[index]);
			start = end + 1;
			end = columnsPart.find(',', start);
			index++;
		}

		//add the last column
		columns[index] = columnsPart.substr(start);
		stringUtils::trim(columns[index]);

		if (noColumns == 0) {
			delete[] columns;
			throw invalid_argument("Invalid command format. No columns specified.");
		}

		//find the position of "WHERE "
		size_t wherePos = commandCopy.find(" WHERE ");
		if (wherePos == string::npos) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Missing 'WHERE' or missing space before 'WHERE'.");
		}

		//get the table name
		string tableName = commandCopy.substr(fromPos + 5, wherePos - (fromPos + 5));  // 5 is the length of "FROM " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		//get the condition part
		string conditionPart = commandCopy.substr(wherePos + 7);  // 7 is the length of " WHERE " with spaces
		stringUtils::trim(conditionPart);

		if (conditionPart.empty()) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Condition cannot be empty.");
		}

		//split the condition part into column and value
		size_t equalPos = conditionPart.find('=');
		if (equalPos == string::npos) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Missing '=' in condition.");
		}

		string conditionColumn = conditionPart.substr(0, equalPos);
		stringUtils::trim(conditionColumn);
		string conditionValue = conditionPart.substr(equalPos + 1);
		stringUtils::trim(conditionValue);

		if (conditionColumn.empty() || conditionValue.empty()) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Condition column or value cannot be empty.");
		}

		selectWHERE selectCommand(tableName, columns, noColumns, conditionColumn, conditionValue, selectCommandsAddress);
		delete[] columns;
		return selectCommand;
	}
};
class selectColumns : public selectCommands {
private:
	string tableName;
	string* columnNames;
	int noColumns;
public:
	selectColumns() : selectCommands() {
		this->tableName = "";
		this->columnNames = nullptr;
		this->noColumns = 0;
	}
	selectColumns(const string& tableName, const string* columnNames, int noColumns, const string& selectCommandsAddress)
		: selectCommands(selectCommandsAddress) {
		this->tableName = tableName;
		this->columnNames = new string[noColumns];
		for (int i = 0; i < noColumns; ++i) {
			this->columnNames[i] = columnNames[i];
		}
		this->noColumns = noColumns;
	}
	selectColumns(const selectColumns& sc) : selectCommands(sc.selectCommandsAddress) {
		this->tableName = sc.tableName;
		this->noColumns = sc.noColumns;
		this->columnNames = new string[sc.noColumns];
		for (int i = 0; i < sc.noColumns; ++i) {
			this->columnNames[i] = sc.columnNames[i];
		}
	}
	selectColumns& operator=(const selectColumns& sc) {
		if (this == &sc) {
			return *this;
		}
		this->tableName = sc.tableName;
		this->selectCommandsAddress = sc.selectCommandsAddress;
		this->noColumns = sc.noColumns;
		delete[] this->columnNames;
		this->columnNames = new string[sc.noColumns];
		for (int i = 0; i < sc.noColumns; ++i) {
			this->columnNames[i] = sc.columnNames[i];
		}
		return *this;
	}
	~selectColumns() {
		delete[] columnNames;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		selectCount++;

		// Validate table existence
		if (!db.tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		// Get a copy of the table from the database
		Table table = db.getTableByName(tableName);

		// Validate column existence and initialize indexes
		int* columnIndexes = new int[noColumns];
		if (!HelpersForSelectCommands::validateColumns(&table, columnNames, noColumns, columnIndexes)) {
			delete[] columnIndexes;
			return;
		}

		// Calculate column widths
		int* maxWidth = HelpersForSelectCommands::calculateMaxWidths(&table, columnNames, noColumns, columnIndexes);

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
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
		HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Print rows
		for (int i = 0; i < table.getNoRows(); i++) {
			const Row& row = table.getRow(i);
			HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
		}

		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Restore cout
		cout.rdbuf(coutBuf);

		// Print headers and rows to screen
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
		HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		for (int i = 0; i < table.getNoRows(); i++) {
			const Row& row = table.getRow(i);
			HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
		}

		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Cleanup
		delete[] columnIndexes;
		delete[] maxWidth;
	}
	static selectColumns parseCommand(const string& command, const string& selectCommandsAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command starts with "SELECT "
		if (commandCopy.find("SELECT ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		//find the position of "FROM "
		size_t pos = commandCopy.find("FROM ");
		if (pos == string::npos) {
			throw invalid_argument("Invalid command format. Missing 'FROM'.");
		}

		//make sure there is a space before "FROM "
		if (commandCopy[pos - 1] != ' ') {
			throw invalid_argument("Invalid command format. Missing space before 'FROM'.");
		}

		//get the columns part
		string columnsPart = commandCopy.substr(7, pos - 8);  // 7 is the length of "SELECT " with a space
		stringUtils::trim(columnsPart);

		if (columnsPart.empty()) {
			throw invalid_argument("Invalid command format. Columns cannot be empty.");
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
			stringUtils::trim(columns[index]);
			start = end + 1;
			end = columnsPart.find(',', start);
			index++;
		}

		//add the last column
		columns[index] = columnsPart.substr(start);
		stringUtils::trim(columns[index]);

		if (noColumns == 0) {
			delete[] columns;
			throw invalid_argument("Invalid command format. No columns specified.");
		}

		//get the table name
		string tableName = commandCopy.substr(pos + 5);  // 5 is the length of "FROM " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		//check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			delete[] columns;
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		selectColumns sc(tableName, columns, noColumns, selectCommandsAddress);
		delete[] columns;
		return sc;
	}
};
class selectAllWhere : public selectCommands {
private:
	string tableName;
	string conditionColumn;
	string value;
public:
	selectAllWhere() : selectCommands() {
		this->tableName = "";
		this->conditionColumn = "";
		this->value = "";
	}
	selectAllWhere(const string& tableName, const string& conditionColumn, const string& value, const string& selectCommandsAddress)
		: selectCommands(selectCommandsAddress), tableName(tableName), conditionColumn(conditionColumn), value(value) {}
	selectAllWhere(const selectAllWhere& saw)
		: selectCommands(saw.selectCommandsAddress), tableName(saw.tableName), conditionColumn(saw.conditionColumn), value(saw.value) {}
	selectAllWhere& operator=(const selectAllWhere& saw) {
		if (this == &saw) {
			return *this;
		}
		this->tableName = saw.tableName;
		this->conditionColumn = saw.conditionColumn;
		this->value = saw.value;
		this->selectCommandsAddress = saw.selectCommandsAddress;
		return *this;
	}

	void execute(Database& db) override {
		selectCount++;

		// Validate table existence
		if (!db.tableExists(tableName)) {
			cout << "Error: Table '" << tableName << "' does not exist." << endl;
			return;
		}

		// Get a copy of the table from the database
		Table table = db.getTableByName(tableName);

		int conditionColumnIndex = table.getColumnIndex(conditionColumn);
		if (conditionColumnIndex == -1) {
			cout << "Error: Column '" << conditionColumn << "' does not exist in table: " << tableName << endl;
			return;
		}

		int* rows = nullptr;
		int rowCount = 0;

		try {
			// Check if there is an index on the condition column
			Index index = db.getIndexByColumnAndTable(conditionColumn, tableName);

			// Use the index to get the rows
			cout << "Using index for column: " << conditionColumn << endl;
			rowCount = index.getRowCount(value);
			if (rowCount > 0) {
				rows = new int[rowCount];
				index.getKeys(value, rows, rowCount);
			}
		}
		catch (const std::runtime_error& e) {
			// Handle the case where the index is not found
			cout << "Scanning table for column '" << conditionColumn << "'." << endl;

			// No index, scan the table
			rowCount = table.getNoRows();
			rows = new int[rowCount];
			int rowIndex = 0;
			for (int i = 0; i < rowCount; i++) {
				if (table.getRow(i).getTextData(conditionColumnIndex) == value) {
					rows[rowIndex++] = i;
				}
			}
			rowCount = rowIndex; // Update count to matched rows
		}

		if (rowCount == 0) {
			cout << "No rows found with " << conditionColumn << " = " << value << endl;
			delete[] rows;
			return;
		}

		// Get all column names
		int noColumns = table.getNoColumns();
		string* columnNames = new string[noColumns];
		for (int i = 0; i < noColumns; i++) {
			columnNames[i] = table.getColumn(i).getName();
		}

		// Initialize column indexes
		int* columnIndexes = new int[noColumns];
		for (int i = 0; i < noColumns; i++) {
			columnIndexes[i] = i;
		}

		// Calculate column widths
		int* maxWidth = HelpersForSelectCommands::calculateMaxWidths(&table, columnNames, noColumns, columnIndexes);

		// Create the file path
		string fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
		ofstream outFile(fileName);
		if (!outFile.is_open()) {
			cout << "Error: Could not open file: " << fileName << endl;
			delete[] columnNames;
			delete[] columnIndexes;
			delete[] maxWidth;
			delete[] rows;
			return;
		}

		// Redirect cout to a file
		streambuf* coutBuf = cout.rdbuf();
		cout.rdbuf(outFile.rdbuf());

		// Print headers and rows to file
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
		HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		for (int i = 0; i < rowCount; i++) {
			const Row& row = table.getRow(rows[i]);
			HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
		}

		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Restore cout
		cout.rdbuf(coutBuf);

		// Print headers and rows to screen
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);
		HelpersForSelectCommands::printHeader(columnNames, maxWidth, noColumns);
		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		for (int i = 0; i < rowCount; i++) {
			const Row& row = table.getRow(rows[i]);
			HelpersForSelectCommands::printRow(row, &table, columnIndexes, noColumns, maxWidth);
		}

		HelpersForSelectCommands::printSeparator(maxWidth, noColumns);

		// Cleanup
		delete[] columnNames;
		delete[] columnIndexes;
		delete[] maxWidth;
		delete[] rows;
	}
	static selectAllWhere parseCommand(const string& command, const string& selectCommandsAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "SELECT ALL "
		if (commandCopy.find("SELECT ALL ") != 0) {
			cout << endl << "Invalid command format.";
			return selectAllWhere();
		}

		// Find the position of "FROM "
		size_t fromPos = commandCopy.find("FROM ");
		if (fromPos == string::npos) {
			cout << endl << "Invalid command format. Missing 'FROM'.";
			return selectAllWhere();
		}

		// Make sure there is a space before "FROM "
		if (commandCopy[fromPos - 1] != ' ') {
			cout << endl << "Invalid command format. Missing space before 'FROM'.";
			return selectAllWhere();
		}

		// Get the table name
		size_t wherePos = commandCopy.find(" WHERE ");
		if (wherePos == string::npos) {
			cout << endl << "Invalid command format. Missing 'WHERE' or missing space before 'WHERE'.";
			return selectAllWhere();
		}

		string tableName = commandCopy.substr(fromPos + 5, wherePos - (fromPos + 5));  // 5 is the length of "FROM " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			cout << endl << "Invalid command format. Table name cannot be empty.";
			return selectAllWhere();
		}

		// Get the condition part
		string conditionPart = commandCopy.substr(wherePos + 7);  // 7 is the length of " WHERE " with spaces
		stringUtils::trim(conditionPart);

		if (conditionPart.empty()) {
			cout << endl << "Invalid command format. Condition cannot be empty.";
			return selectAllWhere();
		}

		// Split the condition part into column and value
		size_t equalPos = conditionPart.find('=');
		if (equalPos == string::npos) {
			cout << endl << "Invalid command format. Missing '=' in condition.";
			return selectAllWhere();
		}

		string conditionColumn = conditionPart.substr(0, equalPos);
		stringUtils::trim(conditionColumn);
		string conditionValue = conditionPart.substr(equalPos + 1);
		stringUtils::trim(conditionValue);

		if (conditionColumn.empty() || conditionValue.empty()) {
			cout << endl << "Invalid command format. Condition column or value cannot be empty.";
			return selectAllWhere();
		}

		return selectAllWhere(tableName, conditionColumn, conditionValue, selectCommandsAddress);
	}
};

class helpMenu : public helpCommands {
public:
	void execute(Database& db) override {
		system("cls");

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
	static helpMenu parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command is "help"
		if (commandCopy != "help") {
			throw invalid_argument("Invalid command format.");
		}

		return helpMenu();
	}
};
class syntaxMenu : public helpCommands {
public:
	void execute(Database& db) override {
		system("cls");

		std::cout << "========================== SYNTAX MENU ==========================\n";
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
	static syntaxMenu parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command is "help 2" or "help2"
		if (commandCopy != "help 2" && commandCopy != "help2") {
			throw invalid_argument("Invalid command format.");
		}

		return syntaxMenu();
	}
};
class clearDisplay : public Command {
public:
	void execute(Database& db) override {
		system("cls");
	}
	static clearDisplay parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command is "clear"
		if (commandCopy != "clear") {
			throw invalid_argument("Invalid command format.");
		}

		return clearDisplay();
	}
};

class commandParser; //declaration ahead of commandParser to avoid errors

// struct to hold command keyword and corresponding handler
struct CommandMapping {
	const char* commandKeywords[3];
	Command* (commandParser::* handlerFunction)(const string&);  // member function pointer to command handler
};

class commandParser {
private:
	static CommandMapping commandMappings[];
	string selectCommandsAddress;
	string tablesConfigAddress;
	string csvFilesDirectory;
	char csvDelimiter;

	Command* handleCreateTable(const string& command) {
		return new createTable(createTable::parseCommand(command));
	}
	Command* handleDescribeTable(const string& command) {
		return new describeTable(describeTable::parseCommand(command));
	}
	Command* handleDropTable(const string& command) {
		return new dropTable(dropTable::parseCommand(command, tablesConfigAddress));
	}
	Command* handleInsertCommand(const string& command) {
		return new InsertCommand(InsertCommand::parseCommand(command));
	}
	Command* handleCreateIndex(const string& command) {
		return new createIndex(createIndex::parseCommand(command));
	}
	Command* handleDeleteWhere(const string& command) {
		return new deleteWhere(deleteWhere::parseCommand(command));
	}
	Command* handleUpdateTable(const string& command) {
		return new updateTable(updateTable::parseCommand(command));
	}
	Command* handleAlterAddColumn(const string& command) {
		return new alterAddColumn(alterAddColumn::parseCommand(command));
	}
	Command* handleAlterDropColumn(const string& command) {
		return new alterDropColumn(alterDropColumn::parseCommand(command));
	}
	Command* handleDropIndex(const string& command) {
		return new dropIndex(dropIndex::parseCommand(command));
	}
	Command* handleImportCsv(const string& command) {
		return new importCSV(importCSV::parseCommand(command, csvFilesDirectory, csvDelimiter));
	}
	//--------------------------------------------------
	Command* handleSelectAll(const string& command) {
		return new selectAll(selectAll::parseCommand(command, selectCommandsAddress));
	}
	Command* handleSelectWHERE(const string& command) {
		return new selectWHERE(selectWHERE::parseCommand(command, selectCommandsAddress));
	}
	Command* handleSelectColumns(const string& command) {
		return new selectColumns(selectColumns::parseCommand(command, selectCommandsAddress));
	}
	Command* handleSelectAllWhere(const string& command) {
		return new selectAllWhere(selectAllWhere::parseCommand(command, selectCommandsAddress));
	}
	//--------------------------------------------------
	Command* handleShowTables(const string& command) {
		return new showTables(showTables::parseCommand(command));
	}
	Command* handleShowIndexFromTable(const string& command) {
		return new showIndexFromTable(showIndexFromTable::parseCommand(command));
	}
	Command* handleShowIndexFromAll(const string& command) {
		return new showIndexFromAll(showIndexFromAll::parseCommand(command));
	}
	//--------------------------------------------------
	Command* handleHelpMenu(const string& command) {
		return new helpMenu(helpMenu::parseCommand(command));
	}
	Command* handleSyntaxMenu(const string& command) {
		return new syntaxMenu(syntaxMenu::parseCommand(command));
	}
	Command* handleClearDisplay(const string& command) {
		return new clearDisplay(clearDisplay::parseCommand(command));
	}

public:
	commandParser(const string& selectCommandsAddress, const string& tablesConfigAddress, const string& csvFilesDirectory, char csvDelimiter) {
		this->selectCommandsAddress = selectCommandsAddress;
		this->tablesConfigAddress = tablesConfigAddress;
		this->csvFilesDirectory = csvFilesDirectory;
		this->csvDelimiter = csvDelimiter;
	}
	Command* handleCommand(const string& command) {
		for (int i = 0; commandMappings[i].commandKeywords[0] != nullptr; i++) {
			bool match = true;
			for (int j = 0; j < 3 && commandMappings[i].commandKeywords[j] != nullptr; j++) {
				if (command.find(commandMappings[i].commandKeywords[j]) == string::npos) {
					match = false;
					break;
				}
			}
			if (match) {
				// pass the command to the member function pointer
				return (this->*commandMappings[i].handlerFunction)(command);
			}
		}
		throw invalid_argument("Unknown command.");
	}
};

CommandMapping commandParser::commandMappings[] = {
	{{"CREATE", "TABLE", nullptr}, &commandParser::handleCreateTable},
	{{"DESCRIBE", nullptr , nullptr}, &commandParser::handleDescribeTable},
	{{"ALTER", "DROP", nullptr}, &commandParser::handleAlterDropColumn},
	{{"DROP", "TABLE", nullptr}, &commandParser::handleDropTable},
	{{"INSERT", "INTO", nullptr}, &commandParser::handleInsertCommand},
	{{"CREATE", "INDEX", nullptr}, &commandParser::handleCreateIndex},
	{{"DELETE", nullptr, nullptr}, &commandParser::handleDeleteWhere},
	{{"UPDATE", nullptr , nullptr}, &commandParser::handleUpdateTable},
	{{"ALTER", "ADD", nullptr}, &commandParser::handleAlterAddColumn},
	{{"DROP", "INDEX", nullptr}, &commandParser::handleDropIndex},
	{{"IMPORT", nullptr, nullptr}, &commandParser::handleImportCsv},
	//--------------------------------------------------
	{{"SELECT", "ALL", "WHERE"}, &commandParser::handleSelectAllWhere},
	{{"SELECT", "ALL" , nullptr}, &commandParser::handleSelectAll},
	{{"SELECT", "WHERE" , nullptr}, &commandParser::handleSelectWHERE},
	{{"SELECT", nullptr , nullptr}, &commandParser::handleSelectColumns},
	//--------------------------------------------------
	{{"SHOW", "TABLES", nullptr}, &commandParser::handleShowTables},
	{{"SHOW", "INDEX", "ALL"}, &commandParser::handleShowIndexFromAll},
	{{"SHOW", "INDEX", nullptr}, &commandParser::handleShowIndexFromTable},
	//--------------------------------------------------
	{{"help", "2", nullptr}, &commandParser::handleSyntaxMenu},
	{{"help", nullptr, nullptr}, &commandParser::handleHelpMenu},
	{{"clear", nullptr, nullptr}, &commandParser::handleClearDisplay},
	{{nullptr, nullptr, nullptr}, nullptr}  // end marker for the array
};

int main() {
	Database db;
	string selectCommandsAddress = "D:\\VS PROJECTS\\!!DBMS PROJECT OG\\DBMS PROJECT\\select_commands\\";
	string tablesConfigAddress = "D:\\VS PROJECTS\\!!DBMS PROJECT OG\\DBMS PROJECT\\tables_config\\";
	string csvFilesPath = "D:\\VS PROJECTS\\!!DBMS PROJECT OG\\DBMS PROJECT\\csv_files\\";
	char csvDelimiter = ',';

	commandParser parser(selectCommandsAddress, tablesConfigAddress, csvFilesPath, csvDelimiter);
	string userCommand;

	cout << "Use the 'help' command to view available commands and their syntax." << endl;
	cout << "Use the 'exit' command to close the program and save your changes." << endl;

	try {
		db.loadDatabase(tablesConfigAddress, selectCommandsAddress);
	}
	catch (const exception& e) {
		cout << "Error loading database: " << e.what() << endl;
		return 1;
	}

	//read commands from multiple files at the start

	//continue with console input
	while (true) {
		cout << endl << ">> ";
		getline(cin, userCommand);

		if (userCommand == "exit") {
			break;
		}

		try {
			Command* cmd = parser.handleCommand(userCommand);
			cmd->execute(db);
			delete cmd;
		}
		catch (const invalid_argument& e) {
			cout << e.what() << endl;
		}
		catch (const exception& e) {
			cout << e.what() << endl;
		}
		catch (...) {
			cout << "An error occurred." << endl;
		}
	}

	db.saveDatabase(tablesConfigAddress);

	return 0;
}