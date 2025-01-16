#ifndef HELPMENU_H
#define HELPMENU_H

#include <iostream>
#include <string>
#include "../Database.h"
#include "../helpCommands.h"
#include "../stringUtils.h"

using namespace std;

class helpMenu : public helpCommands {
public:
	void execute(Database& db) override {
		system("cls");

		string commandGroups[] = {
			"\x1B[97m  Basic Operations:\033[0m\t\t",
			"\x1B[97m  Data Manipulation:\033[0m\t\t",
			"\x1B[97m  Index Operations:\033[0m\t\t",
			"\x1B[97m  Select Commands:\033[0m\t\t",
			"\x1B[97m  Database Information:\033[0m\t\t",
			"\x1B[97m  Utility Commands:\033[0m\t\t",
		};

		string commands[] = {
			// Table Operations
			"CREATE TABLE",
			"DESCRIBE TABLE",
			"DROP TABLE",

			// Data Manipulation
			"INSERT",
			"DELETE FROM",
			"UPDATE TABLE",
			"ALTER ADD",
			"ALTER DROP",
			"IMPORT",

			// Index Operations
			"CREATE INDEX",
			"DROP INDEX",

			// Data Retrieval
			"SELECT ALL",
			"SELECT ALL WHERE",
			"SELECT COLUMNS",
			"SELECT WHERE",

			// Database Information
			"SHOW TABLES",
			"SHOW INDEX FROM TABLE",
			"SHOW INDEX FROM ALL",

			// Utility Commands
			"HELP MENU",
			"SYNTAX MENU",
			"CLEAR",
			"EXIT"
		};

		string descriptions[] = {
			// Table Operations
			" Create a new table with specified columns and constraints.",
			" Show the structure of an existing table.",
			" Delete an existing table from the database.",

			// Data Manipulation
			" Insert a new row into a table.",
			" Delete rows from a table based on a condition.",
			" Update existing rows in a table based on a condition.",
			" Add a new column to an existing table.",
			" Drop an existing column from a table.",
			" Import data into a table from a file.",

			// Index Operations
			"Create an index on a specified column of a table.",
			"Drop an existing index from a table.",

			// Data Retrieval
			"Select all rows from a table.",
			"Select all rows from a table based on a condition.",
			"Select specific columns from a table.",
			"Select specific columns from a table based on a condition.",

			// Database Information
			"Show all the tables in the database.",
			"Show indexes associated with a specific table.",
			"Show all indexes in the database.",

			// Utility Commands
			"Display this help menu.",
			"Show syntax details for all commands.",
			"Clear the console screen.",
			"Exit the program."
		};

		cout << "============ \x1B[97mHELP MENU\033[0m ===========\n";
		int commandIndex = 0;
		for (int group = 0; group < 6; ++group) {
			cout << endl;
			cout << commandGroups[group] << "\n";
			cout << endl;

			// Determine the number of commands in each group dynamically
			int groupSize = 0;
			if (group == 0) groupSize = 3; // Table Operations
			else if (group == 1) groupSize = 6; // Data Manipulation
			else if (group == 2) groupSize = 2; // Index Operations
			else if (group == 3) groupSize = 4; // Data Retrieval
			else if (group == 4) groupSize = 3; // Database Information
			else if (group == 5) groupSize = 4; // Utility Commands

			for (int i = 0; i < groupSize; ++i, ++commandIndex) {
				cout << commandIndex + 1 << ". " << commands[commandIndex];
				int spaces = 30 - commands[commandIndex].length();
				for (int j = 0; j < spaces; ++j) {
					cout << " "; // Align descriptions
				}
				cout << descriptions[commandIndex] << "\n\n";
			}
		}

		cout << "==================================\n";
		cout << "Type \x1B[33m'help 2'\033[0m to see each command's syntax.\n";
	}

	static helpMenu parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command is "help"
		if (commandCopy != "help") {
			throw invalid_argument("Invalid command format.");
		}

		return helpMenu();
	}
};

#endif