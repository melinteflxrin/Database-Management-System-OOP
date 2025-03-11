#ifndef IMPORTCSV_H
#define IMPORTCSV_H

#include <string>
#include "../Headers/Command.h"
#include "../Headers/stringUtils.h"
#include "../Headers/Database.h"
#include <filesystem>

using namespace std;

class importCSV : public Command {
private:
	string tableName;
	string csvDirectory;
	string fileName;
	char delimiter;
public:
	importCSV() {
		this->tableName = "";
		this->csvDirectory = "";
		this->fileName = "";
		this->delimiter = ',';
	}
	importCSV(const string& tableName, const string& csvDirectory, const string& fileName, char delimiter)
		: tableName(tableName), csvDirectory(csvDirectory), fileName(fileName), delimiter(delimiter) {}
	importCSV(const importCSV& ic) {
		this->tableName = ic.tableName;
		this->csvDirectory = ic.csvDirectory;
		this->fileName = ic.fileName;
		this->delimiter = ic.delimiter;
	}
	importCSV& operator=(const importCSV& ic) {
		if (this == &ic) {
			return *this;
		}
		this->tableName = ic.tableName;
		this->csvDirectory = ic.csvDirectory;
		this->fileName = ic.fileName;
		this->delimiter = ic.delimiter;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.importCSV(tableName, csvDirectory, fileName, delimiter);
	}
	static importCSV parseCommand(const string& command, const string& csvFilePath, char delimiter) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "IMPORT INTO "
		if (commandCopy.find("IMPORT INTO ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Find the position of the space after "IMPORT INTO "
		size_t pos = commandCopy.find(" ", 12); // 12 is the length of "IMPORT INTO "
		if (pos == string::npos) {
			throw invalid_argument("Invalid command format. Missing table name or file name.");
		}

		// Get the table name
		string tableName = commandCopy.substr(12, pos - 12);
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		// Get the file name
		string fileName = commandCopy.substr(pos + 1);
		stringUtils::trim(fileName);

		if (fileName.empty()) {
			throw invalid_argument("Invalid command format. File name cannot be empty.");
		}

		// Check for valid file extension
		if (fileName.find(".csv") == string::npos && fileName.find(".txt") == string::npos) {
			throw invalid_argument("Invalid file format. Only .csv or .txt files are allowed.");
		}

		// Check if the file exists
		if (!filesystem::exists(csvFilePath)) {
			throw runtime_error("Error: File '" + csvFilePath + "' does not exist.");
		}

		// Import the CSV file into the table
		return importCSV(tableName, csvFilePath, fileName, delimiter);
	}
};

#endif