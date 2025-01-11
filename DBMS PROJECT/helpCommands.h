#ifndef HELPCOMMANDS_H
#define HELPCOMMANDS_H

#include "Command.h"
#include "Database.h"

class helpCommands : public Command {
public:
	virtual void execute(Database& db) = 0;
	virtual ~helpCommands() = default;
};

#endif // HELPCOMMANDS_H