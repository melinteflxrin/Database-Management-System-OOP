#ifndef SHOWTABLES_H
#define SHOWTABLES_H

#include <string>
#include "../Command.h"
#include "../stringUtils.h"
#include "../Database.h"

using namespace std;

class showTables : public Command {
private:
public:
	void execute(Database& db) override {
		db.showTables();
	}
	static showTables parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command is "SHOW TABLES"
		if (commandCopy != "SHOW TABLES") {
			throw invalid_argument("Invalid command format.");
		}

		return showTables();
	}
};

#endif