#pragma once

#include "AbstractCommandStreamDataSource.h"

class CommandStream
{

public:
	CommandStream(AbstractCommandStreamDataSource* dataSource);

	void getNextCommand(char* output);
	bool readyForNextCommand();
	bool complete();
	void setShouldLoop();
	void updateOnLoop();
	~CommandStream();

private:
	AbstractCommandStreamDataSource* dataSource = nullptr;
	bool shouldLoop = false;
	unsigned long timeOfNextCommand = 0;
	unsigned long msEndOfLatestCommand = 0;
};