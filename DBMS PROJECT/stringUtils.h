#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include "Column.h"

class stringUtils {
public:
	static ColumnType parseColumnType(const std::string& type);
	static bool parseUnique(const std::string& unique);
	static void trim(std::string& str);
	static void splitCommand(const std::string& command, const std::string& delimiter, std::string*& tokens, int& tokenCount);
};

#endif // STRINGUTILS_H