#ifndef STARTCOMMANDS_H
#define STARTCOMMANDS_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "commandParser.h"
#include "Database.h"

using namespace std;

class startCommands {
public:
	static void processCommandFiles(commandParser& parser, Database& db, const string filePaths[], int fileCount) {
		for (int i = 0; i < fileCount; ++i) {
			if (filePaths[i].substr(filePaths[i].find_last_of(".") + 1) != "txt") {
				cout << "File is not a .txt file: " << filePaths[i] << endl;
				continue;
			}

			if (!std::filesystem::exists(filePaths[i])) {
				cout << "File does not exist: " << filePaths[i] << endl;
				continue;
			}

			ifstream file(filePaths[i]);
			if (!file.is_open()) {
				cout << "Error opening file: " << filePaths[i] << endl;
				continue;
			}

			string command;
			while (getline(file, command)) {
				try {
					Command* cmd = parser.handleCommand(command);
					cmd->execute(db);
					delete cmd;
				}
				catch (const invalid_argument& e) {
					cout << "Error in file " << filePaths[i] << ": " << e.what() << endl;
				}
				catch (const exception& e) {
					cout << "Error in file " << filePaths[i] << ": " << e.what() << endl;
				}
				catch (...) {
					cout << "An error occurred in file " << filePaths[i] << endl;
				}
			}

			file.close();
		}
	}
};

#endif