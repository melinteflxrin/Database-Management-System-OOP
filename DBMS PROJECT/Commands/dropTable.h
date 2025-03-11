#ifndef DROPTABLE_H
#define DROPTABLE_H

#include <iostream>
#include <string>
#include "../Headers/Command.h"
#include "../Headers/Database.h"
#include "../Headers/stringUtils.h"

using namespace std;

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

#endif 