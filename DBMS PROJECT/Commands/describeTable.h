#ifndef DESCRIBETABLE_H
#define DESCRIBETABLE_H

#include <iostream>
#include <string>
#include "../Headers/Command.h"
#include "../Headers/Database.h"
#include "../Headers/stringUtils.h"

using namespace std;

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

#endif