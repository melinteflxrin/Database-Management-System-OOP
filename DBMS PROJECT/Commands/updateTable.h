#ifndef UPDATETABLE_H
#define UPDATETABLE_H

#include <string>
#include "../Headers/Command.h"
#include "../Headers/Database.h"
#include "../Headers/stringUtils.h"

using namespace std;

class updateTable : public Command {
private:
	string tableName;
	string setColumnName;
	string setValue;
	string whereColumnName;
	string whereValue;
public:
	updateTable() {
		this->tableName = "";
		this->setColumnName = "";
		this->setValue = "";
		this->whereColumnName = "";
		this->whereValue = "";
	}
	updateTable(const string& tableName, const string& setColumnName, const string& setValue, const string& whereColumnName, const string& whereValue) {
		this->tableName = tableName;
		this->setColumnName = setColumnName;
		this->setValue = setValue;
		this->whereColumnName = whereColumnName;
		this->whereValue = whereValue;
	}
	updateTable(const updateTable& ut) {
		this->tableName = ut.tableName;
		this->setColumnName = ut.setColumnName;
		this->setValue = ut.setValue;
		this->whereColumnName = ut.whereColumnName;
		this->whereValue = ut.whereValue;
	}
	updateTable& operator=(const updateTable& ut) {
		if (this == &ut) {
			return *this;
		}
		this->tableName = ut.tableName;
		this->setColumnName = ut.setColumnName;
		this->setValue = ut.setValue;
		this->whereColumnName = ut.whereColumnName;
		this->whereValue = ut.whereValue;
		return *this;
	}
	//--------------------------------------------------
	void execute(Database& db) override {
		db.updateTable(tableName, setColumnName, setValue, whereColumnName, whereValue);
	}
	static updateTable parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		// Check if the command starts with "UPDATE "
		if (commandCopy.find("UPDATE ") != 0) {
			throw invalid_argument("Invalid command format.");
		}

		// Find the position of " SET "
		size_t setPos = commandCopy.find(" SET ");
		if (setPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing 'SET'.");
		}

		// Find the position of " WHERE "
		size_t wherePos = commandCopy.find(" WHERE ");
		if (wherePos == string::npos) {
			throw invalid_argument("Invalid command format. Missing 'WHERE'.");
		}

		// Extract the table name
		string tableName = commandCopy.substr(7, setPos - 7);  // 7 is the length of "UPDATE "
		stringUtils::trim(tableName);

		if (tableName.empty()) {
			throw invalid_argument("Invalid command format. Table name cannot be empty.");
		}

		// Extract the set part
		string setPart = commandCopy.substr(setPos + 5, wherePos - (setPos + 5));  // 5 is the length of " SET "
		stringUtils::trim(setPart);

		if (setPart.empty()) {
			throw invalid_argument("Invalid command format. Set part cannot be empty.");
		}

		// Split the set part into column and value
		size_t equalPos = setPart.find('=');
		if (equalPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing '=' in set part.");
		}

		string setColumnName = setPart.substr(0, equalPos);
		stringUtils::trim(setColumnName);
		string setValue = setPart.substr(equalPos + 1);
		stringUtils::trim(setValue);

		if (setColumnName.empty() || setValue.empty()) {
			throw invalid_argument("Invalid command format. Set column or value cannot be empty.");
		}

		// Extract the condition part
		string conditionPart = commandCopy.substr(wherePos + 7);  // 7 is the length of " WHERE "
		stringUtils::trim(conditionPart);

		if (conditionPart.empty()) {
			throw invalid_argument("Invalid command format. Condition cannot be empty.");
		}

		// Split the condition part into column and value
		size_t conditionEqualPos = conditionPart.find('=');
		if (conditionEqualPos == string::npos) {
			throw invalid_argument("Invalid command format. Missing '=' in condition part.");
		}

		string whereColumnName = conditionPart.substr(0, conditionEqualPos);
		stringUtils::trim(whereColumnName);
		string whereValue = conditionPart.substr(conditionEqualPos + 1);
		stringUtils::trim(whereValue);

		if (whereColumnName.empty() || whereValue.empty()) {
			throw invalid_argument("Invalid command format. Condition column or value cannot be empty.");
		}

		return updateTable(tableName, setColumnName, setValue, whereColumnName, whereValue);
	}
};

#endif