#ifndef COMMAND_H
#define COMMAND_H

class Database;

class Command {
public:
	virtual void execute(Database& db) = 0;
	virtual ~Command() = default;
};

#endif // COMMAND_H