#pragma once

#include <Arduino.h>
#include "../Modules/AnimationConfiguration.h"
#include "../DataSource/DataSource.h"

class OfflineDataSource : public DataSource
{
public:
	/**
	 * @brief Open the setup file and starts the process of reading setup commands.
	 * @return True if the file was successfully opened, false otherwise.
	 */
	virtual bool openSetup()
	{
		return false;
	}

	/**
	 * @brief Open the animation file for the given index and starts the process of reading animation commands
	 * @param animIndex The index of the animation to open, typically corresponding to a file on the SD card or an entry in generated code
	 * @param loop True, if the animation should loop, false otherwise. This may affect which file is opened (e.g. normal animation file vs loop file) or how the data is read.
	 * @return True if the file was successfully opened, false otherwise.
	 */
	virtual bool openAnimation(uint8_t animIndex, bool loop)
	{
		return false;
	}

	/**
	 * @brief Prepares the next command for the parser. This may involve reading the next line from a file, or preparing the next entry in generated code. This is called to advance the internal state of the data source.
	 */
	virtual void prepareNextCommand() {};

	/**
	 * @brief Peeks the next command without consuming it. This can be used to retrieve timing information for the next command without advancing the buffer state.
	 * @param out A char array to store the peeked command. Should be large enough to hold a full command (e.g. MAX_COMMAND_LENGTH).
	 * @return True, if a command was successfully peeked, false if no command is available or an error occurred.
	 */
	virtual bool peekNextCommand(char* out)
	{
		return false;
	}

	/**
	 * @brief Opens the animation config for the given index and parses it into the provided config struct.
	 * @param animIndex Animation index to retrieve the configuration for
	 * @param config A pointer to an AnimationConfiguration struct to be filled with the retrieved configuration. Should not be null.
	 * @return True if the configuration was successfully retrieved and parsed, false if an error occurred (e.g. file not found, parse error) or if no configuration exists for the given index.
	 */
	virtual bool getConfigurationForAnimation(uint8_t animIndex, AnimationConfiguration* config)
	{
		return false;
	}

	/**
	 * @brief Indicates whether the data source has completed reading all data.
	 * @return True if all data has been read and there is no more data to consume, false otherwise.
	 */
	virtual bool dataComplete() const
	{
		return _dataComplete;
	}

protected:
	bool _dataComplete = false;
};