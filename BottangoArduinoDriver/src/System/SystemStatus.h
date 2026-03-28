// SystemStatus.h

#ifndef _SystemStatus_h
#define _SystemStatus_h

#include <Arduino.h>

namespace SystemStatus
{
	enum class eSignal
	{
		Off,
		SDError,
		OfflinePlayback,
		OfflineReady
	};

	enum class eConnectionStatus
	{
		Off,
		Has_Connection,
		No_Connection_Peer,
		No_Connection_Serial,
		Export_Playback,
		Red
	};

	enum class ePowerStatus
	{
		Off,
		Ok,
		nOk
	};

	enum class eUserLED
	{
		Off
	};

	enum class eCommandStatus
	{
		Idle,
		NewCommand
	};

	enum class ePlaybackStatus
	{
		NotPlaying,
		PlayingStartAnimation,
		PlayingIdleAnimation,
		PlayingOtherAnimation
	};

	/*enum eLightMode
	{
		MODE_PULSE,
		MODE_BLINK
	};*/

	struct sSystemStatus {
		eSignal Signal = eSignal::Off;
		eConnectionStatus ConnectionStatus = eConnectionStatus::Off;
		ePowerStatus PowerStatus = ePowerStatus::Off;
		eUserLED UserLED = eUserLED::Off;
		eCommandStatus CommandStatus = eCommandStatus::Idle;
		ePlaybackStatus PlaybackStatus = ePlaybackStatus::NotPlaying;
		bool initialized = false;
		bool handshake = false;
		//bool resetPreferences = false;
	};

	extern sSystemStatus systemStatus;
}

#endif

