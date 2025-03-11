#include "../Headers/stringUtils.h"
#include <iostream>

ColumnType stringUtils::parseColumnType(const std::string& type) {
	//convert string to ColumnType for the column constructor
	if (type == "INT") return INT;
	if (type == "TEXT") return TEXT;
	if (type == "FLOAT") return FLOAT;
	if (type == "BOOLEAN") return BOOLEAN;
	if (type == "DATE") return DATE;
	throw std::invalid_argument("Invalid column type: " + type);
}

bool stringUtils::parseUnique(const std::string& unique) {
	//convert string to bool for the column constructor
	if (unique == "UNIQUE") return true;
	return false;
}

void stringUtils::trim(std::string& str) {
	//find the first non-space character
	size_t start = str.find_first_not_of(" ");
	//find the last non-space character
	size_t end = str.find_last_not_of(" ");

	//if the string is only spaces set it to an empty string
	if (start == std::string::npos) {
		str = "";
	}
	else {
		//make the trimmed string
		str = str.substr(start, end - start + 1);
	}
}

void stringUtils::splitCommand(const std::string& command, const std::string& delimiter, std::string*& tokens, int& tokenCount) {
	std::string commandCopy = command;
	trim(commandCopy);
	tokenCount = 0;
	size_t pos = 0;
	std::string token;
	while ((pos = commandCopy.find(delimiter)) != std::string::npos) {
		token = commandCopy.substr(0, pos);
		if (!token.empty()) {
			tokenCount++;
		}
		commandCopy.erase(0, pos + delimiter.length());
	}
	if (!commandCopy.empty()) {
		tokenCount++;
	}

	tokens = new std::string[tokenCount];
	commandCopy = command;
	pos = 0;
	int i = 0;
	while ((pos = commandCopy.find(delimiter)) != std::string::npos) {
		token = commandCopy.substr(0, pos);
		if (!token.empty()) {
			tokens[i++] = token;
		}
		commandCopy.erase(0, pos + delimiter.length());
	}
	if (!commandCopy.empty()) {
		tokens[i] = commandCopy;
	}
}