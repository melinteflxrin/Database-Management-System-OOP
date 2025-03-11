#ifndef SHOWINDEXFROMTABLE_H
#define SHOWINDEXFROMTABLE_H

#include <string>
#include "../Headers/Command.h"
#include "../Headers/stringUtils.h"
#include "../Headers/Database.h"

using namespace std;

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

#endif