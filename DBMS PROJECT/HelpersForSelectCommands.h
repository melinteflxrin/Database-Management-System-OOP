#ifndef HELPERSFORSELECTCOMMANDS_H
#define HELPERSFORSELECTCOMMANDS_H

#include <iostream>
#include <string>
#include "Row.h"
#include "Table.h"
#include "Column.h"

class HelpersForSelectCommands {
public:
	static bool rowMatchesCondition(const Row& row, const Column& conditionColumn, int conditionColumnIndex, const std::string& value);
	static void printRow(const Row& row, const Table* table, const int* columnIndexes, int noColumns, const int* maxWidth);
	static int* calculateMaxWidths(const Table* table, const std::string* columnNames, int noColumns, const int* columnIndexes);
	static bool validateColumn(const Table* table, const std::string& columnName, int& columnIndex);
	static bool validateColumns(const Table* table, const std::string* columnNames, int noColumns, int* columnIndexes);
	static void printHeader(const std::string* columnNames, const int* maxWidth, int noColumns);
	static void printSeparator(const int* maxWidth, int noColumns);
};

#endif // HELPERSFORSELECTCOMMANDS_H