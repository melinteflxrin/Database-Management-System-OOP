#ifndef SHOWINDEXFROMALL_H
#define SHOWINDEXFROMALL_H

#include <string>
#include "../Command.h"
#include "../stringUtils.h"
#include "../Database.h"

using namespace std;

class showIndexFromAll : public Command {
private:
public:
	void execute(Database& db) override {
		db.showIndexFromAll();
	}
	static showIndexFromAll parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command is "SHOW INDEX FROM ALL"
		if (commandCopy != "SHOW INDEX FROM ALL") {
			throw invalid_argument("Invalid command format.");
		}

		return showIndexFromAll();
	}
};

#endif