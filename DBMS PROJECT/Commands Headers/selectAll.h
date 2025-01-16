#ifndef SELECTALL_H
#define SELECTALL_H

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "../selectCommands.h"
#include "../stringUtils.h"
#include "../Database.h"

using namespace std;

class selectAll : public selectCommands {
private:
	string name;
public:
	selectAll() : selectCommands() {
		this->name = "";
	}
	selectAll(const string& name, const string& selectCommandsAddress) : selectCommands(selectCommandsAddress) {
		this->name = name;
	}
	selectAll(const selectAll& sa) : selectCommands(sa.selectCommandsAddress) {
		this->name = sa.name;
	}
	selectAll& operator=(const selectAll& sa) {
		if (this == &sa) {
			return *this;
		}
		this->name = sa.name;
		this->selectCommandsAddress = sa.selectCommandsAddress;
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

			//find a unique file name
			string fileName;
			do {
				selectCount++;
				fileName = selectCommandsAddress + "SELECT_" + to_string(selectCount) + ".txt";
			} while (filesystem::exists(fileName));

			//redirect cout to a file
			ofstream outFile(fileName);
			if (!outFile) {
				cout << endl << "Error: Could not open file: " << fileName;
				return;
			}
			streambuf* coutBuf = cout.rdbuf(); //save old buffer
			cout.rdbuf(outFile.rdbuf()); //redirect cout to file

			//display table to file
			table.displayTable();

			//restore cout to its original state
			cout.rdbuf(coutBuf);

			//also display on the screen
			table.displayTable();
		}
		catch (const runtime_error& e) {
			cout << endl << "Error: " << e.what();
		}
	}
	static selectAll parseCommand(const string& command, const string& selectCommandsAddress) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command starts with "SELECT ALL FROM "
		if (commandCopy.find("SELECT ALL FROM ") != 0) {
			if (commandCopy.find("SELECT ALL FROM") == 0) {
				throw invalid_argument("Invalid command format.");
			}
			else {
				throw invalid_argument("Invalid command format.");
			}
		}

		//find the position of "FROM " and make sure there is a space after it
		size_t pos = commandCopy.find("FROM ") + 5;  // 5 is the length of "FROM " with the space
		if (pos >= commandCopy.length()) {
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		//get the table name
		string tableName = commandCopy.substr(pos);
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Too few arguments.");
		}

		//check for extra arguments
		size_t extraArgsPos = tableName.find(' ');
		if (extraArgsPos != string::npos) {
			throw invalid_argument("Invalid command format. Too many arguments.");
		}

		//return a new selectAll object
		return selectAll(tableName, selectCommandsAddress);
	}
};

#endif