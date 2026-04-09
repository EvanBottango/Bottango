// CmdParser.h

#ifndef _CmdParser_h
#define _CmdParser_h

#include <Arduino.h>
#include "../DataSource/DataSource.h"
#include "../Module Handling/ModuleLoop.h"

/**
 * @brief Decodes commands from a data source and provides them to the parser. The CommandDecoder is responsible for reading raw command strings from the data source,
*  splitting them into components, and making them available for consumption by the parser.
 */
class CommandDecoder : public LoopModule
{
public:
	struct SplitCommandData
	{
		char* splitCommandBuffer[COMMANDS_PARAMS_SIZE];
		char* stringToSplit;
#ifdef ALLOW_SYNC_COMMANDS
		//char* currentCommand = nullptr;
		char* syncCommandToSplit = nullptr;
		char* commandEnd = nullptr;
		char* currentFrameStart = nullptr;
		char* nextFrameStart = nullptr;
		bool expectNewCommand = true;
		bool syncCommandInProgress = false;
#endif // ALLOW_SYNC_COMMANDS
	};

	/**
	 * @brief Returns true if there is a valid command available to consume. Includes both primary and secondary data sources.
	 * @return true if a command is available, false otherwise.
	 */
	virtual bool hasCommand();

	/**
	 * @brief Try to consume a command from the decoder. Includes both primary and secondary data sources.
	 * @return Pointer to the command parameters array if a command is available, nullptr otherwise.
	 */
	virtual char** tryConsumeCommand();

	/**
	 * @brief Set the data source for the command decoder.
	 * @param src Pointer to the data source.
	 */
	virtual void setDataSource(DataSource* src);

#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
	/**
	 * @brief Set the secondary data source for the command decoder.
	 * @param src Pointer to the secondary data source.
	 */
	virtual void setSecondaryDataSource(DataSource* src);
#endif

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	/**
	 * @brief Set the offline data source for the command decoder.
	 * @param src Pointer to the secondary data source.
	 */
	virtual void setOfflineDataSource(DataSource* src);
#endif

	/**
	 * @brief Splits a command string into components in-place.
	 * @param stringToSplit Pointer to a writable, null-terminated string containing the command to split. Must be non-null; the function will modify the buffer.
	 */
	virtual bool splitCommand(SplitCommandData* data) const
	{
		return false;
	}

	bool isSourceUsbSerial() const
	{
		return _sourceIsUsbSerial;
	}

#ifdef ALLOW_SYNC_COMMANDS
	/**
	 * @brief Initializes the sync command parsing state with a new command string.
	 * @param stringToSplit Pointer to a writable, null-terminated string containing the sync command to parse. Must be non-null; the function will modify the buffer.
	 */
	virtual void beginSyncCommand(SplitCommandData* data) const {};

	/**
	 * @brief Retrieves the next frame from the current sync command.
	 * @return Pointer to the next frame string, or nullptr if no more frames are available.
	 */
	virtual void getNextFrame(SplitCommandData* data) const {};

	/**
	 * @brief Checks whether additional frames from a multi-frame are available.
	 * @return true if there are more frames available; false otherwise.
	 */
	virtual bool hasMoreFrames(SplitCommandData* data) const
	{
		return false;
	}
#endif // ALLOW_SYNC_COMMANDS

protected:
	bool _validCommandAvailable = false;
	bool _sourceIsUsbSerial = false;

	/**
	 * @brief Decode data from the data source.
	 */
	virtual void decode() {};

	/**
	 * @brief The primary source of data. Usally the default Arduino Serial connection. Is ALWAYS active, to give
	 * the user the chance, to connect Bottango and change the config
	 */
	DataSource* _source;


#if defined(RELAY_SUPPORTED) || defined(USE_ESP32_WIFI)
	/**
	 * @brief The secondary source of data. Like ESP-Now, RS485 or something else
	 */
	DataSource* _secondarySource = nullptr;
#endif

#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	/**
	 * @brief The offline source of data. Like SD-Card, or export to code.
	 */
	DataSource* _offlineSource = nullptr;
#endif

	SplitCommandData _splitData;
};

#endif