#ifndef ALTERDROPCOLUMN_H
#define ALTERDROPCOLUMN_H

#include <string>
#include "../Headers/Command.h"
#include "../Headers/stringUtils.h"
#include "../Headers/Database.h"

using namespace std;

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

#endif