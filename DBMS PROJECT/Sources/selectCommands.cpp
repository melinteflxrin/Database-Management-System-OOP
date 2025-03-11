#include "../Headers/selectCommands.h"

int selectCommands::selectCount = 0;

selectCommands::selectCommands() {
	this->selectCommandsAddress = "";
}

selectCommands::selectCommands(const std::string& selectCommandsAddress) {
	this->selectCommandsAddress = selectCommandsAddress;
}

selectCommands::selectCommands(const selectCommands& sc) {
	this->selectCommandsAddress = sc.selectCommandsAddress;
}

selectCommands& selectCommands::operator=(const selectCommands& sc) {
	if (this == &sc) {
		return *this;
	}
	this->selectCommandsAddress = sc.selectCommandsAddress;
	return *this;
}