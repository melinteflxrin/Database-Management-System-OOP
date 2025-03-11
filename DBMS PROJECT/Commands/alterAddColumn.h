#ifndef ALTERADDCOLUMN_H
#define ALTERADDCOLUMN_H

#include <string>
#include "../Headers/Command.h"
#include "../Headers/Column.h"
#include "../Headers/stringUtils.h"
#include "../Headers/Database.h"

using namespace std;

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

#endif