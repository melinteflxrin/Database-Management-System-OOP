#ifndef SELECTALLWHERE_H
#define SELECTALLWHERE_H

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "../Headers/selectCommands.h"
#include "../Headers/stringUtils.h"
#include "../Headers/Database.h"
#include "../Headers/HelpersForSelectCommands.h"

using namespace std;

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

#endif