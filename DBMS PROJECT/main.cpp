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

class startCommands {
public:
	static void processCommandFiles(commandParser& parser, Database& db, const string filePaths[], int fileCount) {
		for (int i = 0; i < fileCount; ++i) {
			if (filePaths[i].substr(filePaths[i].find_last_of(".") + 1) != "txt") {
				cout << "File is not a .txt file: " << filePaths[i] << endl;
				continue;
			}

			if (!std::filesystem::exists(filePaths[i])) {
				cout << "File does not exist: " << filePaths[i] << endl;
				continue;
			}

			ifstream file(filePaths[i]);
			if (!file.is_open()) {
				cout << "Error opening file: " << filePaths[i] << endl;
				continue;
			}

			string command;
			while (getline(file, command)) {
				try {
					Command* cmd = parser.handleCommand(command);
					cmd->execute(db);
					delete cmd;
				}
				catch (const invalid_argument& e) {
					cout << "Error in file " << filePaths[i] << ": " << e.what() << endl;
				}
				catch (const exception& e) {
					cout << "Error in file " << filePaths[i] << ": " << e.what() << endl;
				}
				catch (...) {
					cout << "An error occurred in file " << filePaths[i] << endl;
				}
			}

			file.close();
		}
	}
};

class readSettingsFile {
public:
	static void readSettings(string& tablesConfigAddress, string& selectCommandsAddress, string& csvFilesPath, char& csvDelimiter) {
		string exePath = std::filesystem::current_path().string();
		string settingsFilePath = exePath + "\\settings.txt";

		if (!std::filesystem::exists(settingsFilePath) || settingsFilePath.substr(settingsFilePath.find_last_of(".") + 1) != "txt") {
			cout << "Settings file does not exist or is not a .txt file: " << settingsFilePath << endl;
			return;
		}

		ifstream settingsFile(settingsFilePath);
		if (!settingsFile.is_open()) {
			cout << "Error opening settings file: " << settingsFilePath << endl;
			return;
		}

		string line;
		int lineCount = 0;
		while (getline(settingsFile, line)) {
			stringUtils::trim(line);

			switch (lineCount) {
			case 0:
				tablesConfigAddress = line;
				break;
			case 1:
				selectCommandsAddress = line;
				break;
			case 2:
				csvFilesPath = line;
				break;
			case 3:
				if (!line.empty()) {
					csvDelimiter = line[0];
				}
				break;
			default:
				cout << "Unexpected extra line in settings file: " << line << endl;
				break;
			}
			lineCount++;
		}

		settingsFile.close();
	}
};

int main(int argc, char* argv[]) {
	Database db;
	string tablesConfigAddress;
	string selectCommandsAddress;
	string csvFilesPath;
	char csvDelimiter;

	readSettingsFile::readSettings(tablesConfigAddress, selectCommandsAddress, csvFilesPath, csvDelimiter);

	commandParser parser(selectCommandsAddress, tablesConfigAddress, csvFilesPath, csvDelimiter);
	string userCommand;

	try {
		db.loadDatabase(tablesConfigAddress, selectCommandsAddress);
	}
	catch (const exception& e) {
		cout << "Error loading database: " << e.what() << endl;
		return 1;
	}

	cout << "Use the 'help' command to view available commands and their syntax." << endl;
	cout << "Use the 'exit' command to close the program and save your changes." << endl;

	//read files passed as arguments
	const int maxFiles = 5;
	string commandFiles[maxFiles];
	int fileCount = 0;
	for (int i = 1; i < argc && fileCount < maxFiles; ++i) {
		commandFiles[fileCount++] = argv[i];
	}
	startCommands::processCommandFiles(parser, db, commandFiles, fileCount);

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

	try {
		db.saveDatabase(tablesConfigAddress);
	}
	catch (const exception& e) {
		cout << "Error saving database: " << e.what() << endl;
	}

	return 0;
}