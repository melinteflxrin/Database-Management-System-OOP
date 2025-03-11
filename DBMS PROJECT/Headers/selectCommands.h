#ifndef SELECTCOMMANDS_H
#define SELECTCOMMANDS_H

#include <string>
#include "Command.h"
#include "Database.h"

class selectCommands : public Command {
protected:
	std::string selectCommandsAddress;
	static int selectCount;
public:
	selectCommands();
	selectCommands(const std::string& selectCommandsAddress);
	selectCommands(const selectCommands& sc);
	selectCommands& operator=(const selectCommands& sc);
	virtual void execute(Database& db) = 0;
	virtual ~selectCommands() = default;
};

#endif // SELECTCOMMANDS_H