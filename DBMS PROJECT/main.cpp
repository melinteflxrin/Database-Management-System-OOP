#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#include "Database.h"
#include "Command.h"
#include "commandParser.h"

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

class readSettingsFile {
public:
	static void readSettings(string& tablesConfigAddress, string& selectCommandsAddress, string& csvFilesPath, char& csvDelimiter) {
		string exePath = std::filesystem::current_path().string();
		string settingsFilePath = exePath + "\\settings.txt";

		if (!std::filesystem::exists(settingsFilePath) || settingsFilePath.substr(settingsFilePath.find_last_of(".") + 1) != "txt") {
			cout << "Settings file does not exist or is not a .txt file: " << settingsFilePath << endl;
			return;
		}

		ifstream settingsFile(settingsFilePath);
		if (!settingsFile.is_open()) {
			cout << "Error opening settings file: " << settingsFilePath << endl;
			return;
		}

		string line;
		int lineCount = 0;
		while (getline(settingsFile, line)) {
			stringUtils::trim(line);

			switch (lineCount) {
			case 0:
				tablesConfigAddress = line;
				break;
			case 1:
				selectCommandsAddress = line;
				break;
			case 2:
				csvFilesPath = line;
				break;
			case 3:
				if (!line.empty()) {
					csvDelimiter = line[0];
				}
				break;
			default:
				cout << "Unexpected extra line in settings file: " << line << endl;
				break;
			}
			lineCount++;
		}

		settingsFile.close();
	}
};

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

	cout << "Use the 'help' command to view available commands and their syntax." << endl;
	cout << "Use the 'exit' command to close the program and save your changes." << endl;

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