#ifndef SELECTCOLUMNS_H
#define SELECTCOLUMNS_H

#include <iostream>
#include <string>
#include <fstream>
#include "../Database.h"
#include "../selectCommands.h"
#include "../HelpersForSelectCommands.h"
#include "../stringUtils.h"

using namespace std;

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

#endif