#ifndef INSERTCOMMAND_H
#define INSERTCOMMAND_H

#include <iostream>
#include <string>
#include "../Headers/Command.h"
#include "../Headers/Database.h"
#include "../Headers/stringUtils.h"

using namespace std;

class InsertCommand : public Command {
private:
	string tableName;
	string* values;
	int noValues;

public:
	InsertCommand() : tableName(""), values(nullptr), noValues(0) {}
	InsertCommand(const string& tableName, const string* values, int noValues)
		: tableName(tableName), values(new string[noValues]), noValues(noValues) {
		for (int i = 0; i < noValues; ++i) {
			this->values[i] = values[i];
		}
	}
	InsertCommand(const InsertCommand& other)
		: tableName(other.tableName), values(new string[other.noValues]), noValues(other.noValues) {
		for (int i = 0; i < noValues; ++i) {
			this->values[i] = other.values[i];
		}
	}
	InsertCommand& operator=(const InsertCommand& other) {
		if (this == &other) {
			return *this;
		}

		delete[] values;

		tableName = other.tableName;
		noValues = other.noValues;
		values = new string[noValues];
		for (int i = 0; i < noValues; ++i) {
			this->values[i] = other.values[i];
		}

		return *this;
	}
	~InsertCommand() {
		delete[] values;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.insertValues(tableName, values, noValues);
	}
	static InsertCommand parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "INSERT INTO "
		if (commandCopy.find("INSERT INTO ") != 0) {
			throw std::invalid_argument("Invalid command format: Missing 'INSERT INTO'.");
		}

		// Find the position of "VALUES "
		size_t pos = commandCopy.find("VALUES ");
		if (pos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing 'VALUES'.");
		}

		// Get the table name
		string tableName = commandCopy.substr(12, pos - 12);  // 12 is the length of "INSERT INTO " with a space
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw std::invalid_argument("Invalid command format: Table name cannot be empty.");
		}

		// Find the position of the first '('
		size_t startPos = commandCopy.find("(");
		if (startPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing '('.");
		}

		// Find the position of the last ')'
		size_t endPos = commandCopy.find_last_of(")");
		if (endPos == string::npos) {
			throw std::invalid_argument("Invalid command format: Missing ')'.");
		}

		// Get the values part
		string valuesPart = commandCopy.substr(startPos + 1, endPos - startPos - 1);  // from after '(' to before ')'
		stringUtils::trim(valuesPart);

		if (valuesPart.empty()) {
			throw std::invalid_argument("Invalid command format: Values cannot be empty.");
		}

		// Split the values part into individual values
		string* values = nullptr;
		int noValues = 0;
		stringUtils::splitCommand(valuesPart, ",", values, noValues);

		if (noValues == 0) {
			throw std::invalid_argument("Invalid command format: No values specified.");
		}

		// Trim each value individually
		for (int i = 0; i < noValues; i++) {
			stringUtils::trim(values[i]);
		}

		InsertCommand insertCommand(tableName, values, noValues);
		delete[] values;
		return insertCommand;
	}
};

#endif