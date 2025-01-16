#ifndef SELECTWHERE_H
#define SSELECTWHERE_H

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "../selectCommands.h"
#include "../stringUtils.h"
#include "../Database.h"
#include "../HelpersForSelectCommands.h"

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

#endif