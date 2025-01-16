#ifndef READSETTINGSFILE_H
#define READSETTINGSFILE_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "stringUtils.h"

using namespace std;

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

#endif