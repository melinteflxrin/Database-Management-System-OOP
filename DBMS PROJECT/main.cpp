#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "Database.h"
#include "Command.h"
#include "commandParser.h"
#include "startCommands.h"
#include "readSettingsFile.h"

using namespace std;

int main(int argc, char* argv[]) {
	Database db;
	string tablesConfigAddress;
	string selectCommandsAddress;
	string csvFilesPath;
	char csvDelimiter;

	readSettingsFile::readSettings(tablesConfigAddress, selectCommandsAddress, csvFilesPath, csvDelimiter);

	commandParser parser(selectCommandsAddress, tablesConfigAddress, csvFilesPath, csvDelimiter);
	string userCommand;

	try {
		db.loadDatabase(tablesConfigAddress, selectCommandsAddress);
	}
	catch (const exception& e) {
		cout << "Error loading database: " << e.what() << endl;
		return 1;
	}

	cout << "Use the \x1B[97m'help'\033[0m command to view available commands and their syntax." << endl;
	cout << "Use the \x1B[97m'exit'\033[0m command to close the program and save your changes." << endl;

	//read files passed as arguments
	const int maxFiles = 5;
	string commandFiles[maxFiles];
	int fileCount = 0;
	for (int i = 1; i < argc && fileCount < maxFiles; ++i) {
		commandFiles[fileCount++] = argv[i];
	}
	startCommands::processCommandFiles(parser, db, commandFiles, fileCount);

	//continue with console input
	while (true) {
		cout << endl << ">> ";
		getline(cin, userCommand);

		if (userCommand == "exit") {
			break;
		}

		try {
			Command* cmd = parser.handleCommand(userCommand);
			cmd->execute(db);
			delete cmd;
		}
		catch (const invalid_argument& e) {
			cout << e.what() << endl;
		}
		catch (const exception& e) {
			cout << e.what() << endl;
		}
		catch (...) {
			cout << "An error occurred." << endl;
		}
	}

	try {
		db.saveDatabase(tablesConfigAddress);
	}
	catch (const exception& e) {
		cout << "Error saving database: " << e.what() << endl;
	}

	return 0;
}