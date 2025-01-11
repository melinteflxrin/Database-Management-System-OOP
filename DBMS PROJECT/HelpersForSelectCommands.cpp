#include "HelpersForSelectCommands.h"

bool HelpersForSelectCommands::rowMatchesCondition(const Row& row, const Column& conditionColumn, int conditionColumnIndex, const std::string& value) {
	// Assuming the Row class has a method to get the value of a column by index
	std::string rowValue = row.getTextData(conditionColumnIndex);

	// Compare the row value with the given value
	return rowValue == value;
}

void HelpersForSelectCommands::printRow(const Row& row, const Table* table, const int* columnIndexes, int noColumns, const int* maxWidth) {
	for (int i = 0; i < noColumns; ++i) {
		int columnIndex = columnIndexes[i];
		std::string value = row.getTextData(columnIndex); // Assuming Row has a getValue method
		std::cout << value;
		if (value.length() < maxWidth[i]) {
			std::cout << std::string(maxWidth[i] - value.length(), ' ');
		}
		std::cout << " | ";
	}
	std::cout << std::endl;
}

int* HelpersForSelectCommands::calculateMaxWidths(const Table* table, const std::string* columnNames, int noColumns, const int* columnIndexes) {
	int* maxWidth = new int[noColumns];

	// Initialize maxWidth with the length of the column names
	for (int i = 0; i < noColumns; ++i) {
		maxWidth[i] = columnNames[i].length();
	}

	// Iterate through each row in the table
	for (int i = 0; i < table->getNoRows(); ++i) {
		const Row& row = table->getRow(i);

		// Update maxWidth based on the length of the data in each column
		for (int j = 0; j < noColumns; ++j) {
			int columnIndex = columnIndexes[j];
			std::string value = row.getTextData(columnIndex); // Assuming Row has a getValue method
			if (value.length() > maxWidth[j]) {
				maxWidth[j] = value.length();
			}
		}
	}

	return maxWidth;
}

bool HelpersForSelectCommands::validateColumn(const Table* table, const std::string& columnName, int& columnIndex) {
	for (int i = 0; i < table->getNoColumns(); ++i) {
		if (table->getColumn(i).getName() == columnName) {
			columnIndex = i;
			return true;
		}
	}
	std::cout << "Error: Column '" << columnName << "' does not exist in table '" << table->getName() << "'." << std::endl;
	return false;
}

bool HelpersForSelectCommands::validateColumns(const Table* table, const std::string* columnNames, int noColumns, int* columnIndexes) {
	for (int i = 0; i < noColumns; ++i) {
		if (!validateColumn(table, columnNames[i], columnIndexes[i])) {
			std::cout << "Error: Column '" << columnNames[i] << "' does not exist in table '" << table->getName() << "'." << std::endl;
			return false;
		}
	}
	return true;
}

void HelpersForSelectCommands::printHeader(const std::string* columnNames, const int* maxWidth, int noColumns) {
	for (int i = 0; i < noColumns; ++i) {
		std::cout << columnNames[i];
		if (columnNames[i].length() < maxWidth[i]) {
			std::cout << std::string(maxWidth[i] - columnNames[i].length(), ' ');
		}
		std::cout << " | ";
	}
	std::cout << std::endl;
}

void HelpersForSelectCommands::printSeparator(const int* maxWidth, int noColumns) {
	for (int i = 0; i < noColumns; ++i) {
		std::cout << std::string(maxWidth[i], '-') << "-+-";
	}
	std::cout << std::endl;
}