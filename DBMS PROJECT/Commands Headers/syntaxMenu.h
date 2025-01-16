#ifndef SYNTAXMENU_H
#define SYNTAXMENU_H

#include <iostream>
#include <string>
#include "../Database.h"
#include "../helpCommands.h"
#include "../stringUtils.h"

using namespace std;

class syntaxMenu : public helpCommands {
public:
	void execute(Database& db) override {
		system("cls");

		std::cout << "========================== \x1B[97mSYNTAX MENU\033[0m ==========================\n";
		cout << endl;
		// Table Operations
		std::cout << "  \x1B[97mBasic Operations:\033[0m\n";
		cout << endl;
		std::cout << "1. CREATE TABLE table_name ((column_name1, column_type, size, default_value),(column_name2, ...))\n";
		cout << endl;
		std::cout << "2. DESCRIBE table_name\n";
		cout << endl;
		std::cout << "3. DROP TABLE table_name\n";
		cout << endl << endl;

		// Data Manipulation
		std::cout << "  \x1B[97mData Manipulation:\033[0m\n";
		cout << endl;
		std::cout << "4. INSERT INTO table_name VALUES (value1, value2, ...)\n";
		cout << endl;
		std::cout << "5. DELETE FROM table_name WHERE condition\n";
		cout << endl;
		std::cout << "6. UPDATE table_name SET column = value WHERE condition\n";
		cout << endl;
		std::cout << "7. ALTER TABLE table_name ADD (column_name, column_type, size, default_value)\n";
		cout << endl;
		std::cout << "8. ALTER TABLE table_name DROP COLUMN column_name\n";
		cout << endl;
		std::cout << "9. IMPORT INTO table_name file_name.csv\n";
		cout << endl << endl;

		// Index Operations
		std::cout << "  \x1B[97mIndex Operations:\033[0m\n";
		cout << endl;
		std::cout << "10. CREATE INDEX index_name ON table_name (column_name)\n";
		cout << endl;
		std::cout << "11. DROP INDEX index_name\n";
		cout << endl << endl;

		// Data Retrieval
		std::cout << "  \x1B[97mSelect Commands:\033[0m\n";
		cout << endl;
		std::cout << "12. SELECT ALL FROM table_name\n";
		cout << endl;
		std::cout << "13. SELECT ALL FROM table_name WHERE condition\n";
		cout << endl;
		std::cout << "14. SELECT column1, column2, ... FROM table_name\n";
		cout << endl;
		std::cout << "15. SELECT column1, column2, ... FROM table_name WHERE condition\n";
		cout << endl << endl;

		// Database Information
		std::cout << "  \x1B[97mDatabase Information:\033[0m\n";
		cout << endl;
		std::cout << "16. SHOW TABLES\n";
		cout << endl;
		std::cout << "17. SHOW INDEX FROM table_name\n";
		cout << endl;
		std::cout << "18. SHOW INDEX FROM ALL\n";
		cout << endl << endl;

		// Utility Commands
		std::cout << "  \x1B[97mUtility Commands:\033[0m\n";
		cout << endl;
		std::cout << "19. help\n";
		cout << endl;
		std::cout << "20. help 2\n";
		cout << endl;
		std::cout << "21. clear\n";
		cout << endl;
		std::cout << "22. exit\n";
		cout << endl;

		std::cout << "================================================================\n";
	}

	static syntaxMenu parseCommand(const string& command) {
		string commandCopy = command;
		stringUtils::trim(commandCopy);

		//check if the command is "help 2" or "help2"
		if (commandCopy != "help 2" && commandCopy != "help2") {
			throw invalid_argument("Invalid command format.");
		}

		return syntaxMenu();
	}
};

#endif