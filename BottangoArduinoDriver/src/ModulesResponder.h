#ifndef BOTTANGOARDUINO_MODULESRESPONDER_H
#define BOTTANGOARDUINO_MODULESRESPONDER_H
#include <Arduino.h>
#include "../BottangoArduinoModules.h"
#include "AbstractMultiMessageOutgoingSource.h"

#if defined(ENABLE_DYNAMIC_ANIMATION_SOURCE_SWITCH) || defined(RELAY_SUPPORTED) || defined(REPORT_UID)
#include "PersistentConfigUtil.h"
#include "Modules/RelayComs/UDIDHelper.h"
#endif

#define MODULES_COUNT 8 // number of possible modules to report

inline const char MODULES_RESPONSE_PREFIX[] PROGMEM = "MOD";
inline const char MODULES_PARAM_DELINIATOR[] PROGMEM = ",";
inline const char END_OF_MODULES[] PROGMEM = "EoM";

// Module 0, UID
#ifdef REPORT_UID
inline const char UID_PREFIX[] PROGMEM = "UID";
#endif

// module 1, named board
#ifdef NAMED_BOARD
inline const char NAMED_BOARD_PREFIX[] PROGMEM = "BOARD";
#endif

// module 2, command source
// param, 0 == accepting commands, 1 == listen only mode
inline const char COMMAND_SOURCE_PREFIX[] PROGMEM = "CMD_SRC";

// module 3, command configuration
// param, max message length as int
// param, curve buffer count as int
// param, sync curve supported (0 == false, 1 == true)
// param, motor signal max (16bit,32bit)
inline const char COMMAND_CONFIG_PREFIX[] PROGMEM = "CMD_CFG";

// module 4, Relay support
#ifdef RELAY_SUPPORTED
inline const char RELAY_PREFIX[] PROGMEM = "RLY";
#endif

// module 5, PCA driver
#ifdef USE_ADAFRUIT_PWM_LIBRARY
inline const char PCA_PREFIX[] PROGMEM = "9685";
#endif

// module 6, I2S Audio
#ifdef AUDIO_SD_I2S
inline const char AUDIO_I2S_PREFIX[] PROGMEM = "I2S";
#endif

// module 7, Stop Button
// param, Stop behavior (0 == pause only, 1 == shut down)
// param, dynamic (0 == static, 1 == switchable)
#ifdef STOP_BUTTON_SUPPORTED
inline const char STOP_BTN_PREFIX[] PROGMEM = "STP_BTN";
#endif

// end of line for now

class ModulesResponder : public AbstractMultiMessageOutgoingSource
{

public:
	virtual void cleanUpMultiMessage() override;

private:
	void onMultiMessageStart() override;
	bool emitNextChunk() override;

	uint8_t iterator = 0;
	bool sendUIDResponse();
	//bool sendBoardIDResponse();
	bool sendNamedBoardResponse();
	bool sendCommandSourceResponse();
	bool sendCommandConfigResponse();
	bool sendPCAResponse();
	bool sendI2SResponse();
	bool sendStopButtonResponse();
	bool sendRelayResponse();
	void sendClosingModuleResponse();

}; // namespace ModulesResponder

#endif // BOTTANGOARDUINO_MODULESRESPONDER_H