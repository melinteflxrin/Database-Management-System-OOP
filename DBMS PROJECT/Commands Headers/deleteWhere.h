#ifndef DELETEWHERE_H
#define DELETEWHERE_H

#include <string>
#include <stdexcept>
#include "../Command.h"
#include "../Database.h"
#include "../stringUtils.h"

using namespace std;

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

#endif