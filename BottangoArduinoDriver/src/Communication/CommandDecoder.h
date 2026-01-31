// CmdParser.h

#ifndef _CmdParser_h
#define _CmdParser_h

#include <Arduino.h>
#include "../DataSource/DataSource.h"
#include "../Module Handling/ModuleLoop.h"

class CommandDecoder : public LoopModule
{
public:
	/**
	 * @brief Decode data from the data source.
	 */
	virtual void decode() {};

	/**
	 * @brief Returns true if there is a valid command available to consume.
	 * @return true if a command is available, false otherwise.
	 */
	virtual bool hasCommand();

	/**
	 * @brief Try to consume a command from the decoder.
	 * @return Pointer to the command parameters array if a command is available, nullptr otherwise.
	 */
	virtual char** tryConsumeCommand();

	/**
	 * @brief Set the data source for the command decoder.
	 * @param src Pointer to the data source.
	 */
	virtual void setDataSource(DataSource* src);

protected:
	char* splitCommandBuffer[COMMANDS_PARAMS_SIZE];
	bool validCommandAvailable = false;
	DataSource* source;
};

#endif