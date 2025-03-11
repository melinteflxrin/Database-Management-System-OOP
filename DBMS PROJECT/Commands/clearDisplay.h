#ifndef CLEARDISPLAY_H
#define CLEARDISPLAY_H

#include <iostream>
#include <string>
#include "../Headers/Command.h"
#include "../Headers/stringUtils.h"

using namespace std;

class clearDisplay : public Command {
public:
	void execute(Database& db) override {
		system("cls");
	}
	static clearDisplay parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command is "clear"
		if (commandCopy != "clear") {
			throw invalid_argument("Invalid command format.");
		}

		return clearDisplay();
	}
};

#endif