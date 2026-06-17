#include "CommandStreamProvider.h"
#include "../../BottangoArduinoConfig.h"
#include "../System/BottangoCore.h"
#include "../Communication/Outgoing.h"

#ifdef ENABLE_STATUS_LIGHTS
#include "../Services/StatusLights.h"
#endif

CommandStreamProvider::CommandStreamProvider()
{}

void CommandStreamProvider::runSetup()
{
	if (streamIsInProgress())
	{
		stop();
	}

#ifdef USE_CODE_COMMAND_STREAM
	commandStream = GeneratedCodeAnimations::GenerateSetupCommandStream();
#elif defined(USE_SD_CARD_COMMAND_STREAM)
	SDCardCommandStreamDataSource* dataSource = new SDCardCommandStreamDataSource();
	if (dataSource->isValid)
	{
		commandStream = new CommandStream(dataSource);
	}
	else
	{
		delete dataSource;
		commandStream = nullptr;
	}

#endif

	// run setup

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif
	{
		Outgoing::printOutputStringFlash(F("Start setup"));
		Outgoing::printLine();
	}
#endif

	commandStreamIsSetup = true;

	runInProgressCommand();

	// parse configs
	// will also run start animation if any
#if defined(USE_SD_CARD_COMMAND_STREAM) || defined(USE_CODE_COMMAND_STREAM)
	ExportedAnimationPlaybackControl::initialize();
#endif
}

void CommandStreamProvider::startCommandStream(byte streamID, bool loop)
{
	if (!BottangoCore::isOffline())
	{
		return;
	}

	if (invalidState)
	{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::printOutputStringFlash(F("cmd stream invalid state, play abort"));
			Outgoing::printLine();
		}
#endif

#ifdef ENABLE_STATUS_LIGHTS
		StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_EXPORT_SD_ERROR);
#endif
		return;
	}

	// can't start an animation if setup is not complete
	if (streamIsInProgress() && commandStreamIsSetup)
	{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif
		{
			Outgoing::printOutputStringFlash(F("Can't Play, Setup Incomplete"));
			Outgoing::printLine();
		}
#endif
		return;
	}

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
	if (PersistentConfigUtil::debugEnabled())
#endif
	{
		Outgoing::printOutputStringFlash(F("Start anim: "));
		Outgoing::printOutputStringMem(streamID);
		Outgoing::printLine();
	}
#endif

	stop();

#ifdef ENABLE_STATUS_LIGHTS
	StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_OFFLINEPLAY);
#endif

#ifdef USE_CODE_COMMAND_STREAM
	commandStream = GeneratedCodeAnimations::GenerateCommandStreamByIndex(streamID);
#elif defined(USE_SD_CARD_COMMAND_STREAM)
	commandStream = new CommandStream(new SDCardCommandStreamDataSource(streamID, loop));
#endif

	if (commandStream != nullptr)
	{
		if (loop)
		{
			commandStream->setShouldLoop();
		}

#ifdef RELAY_SUPPORTED
		if (BottangoCore::isRelayBridge)
		{
			Time::stopTime();
			BottangoCore::relayPool->stopTimeOnConnectedPeers();
			startingPeerCommandsSent = false;
		}
		else
		{
			Time::syncTime(0);
		}

#else
		Time::syncTime(0);
#endif
	}
	else
	{
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::printOutputStringFlash(F("error starting anim"));
			Outgoing::printLine();
		}

#endif
	}
}

void CommandStreamProvider::runInProgressCommand()
{
#ifdef RELAY_SUPPORTED
	// bridge needs to wait till all peers are connected to start time
	// but not during setup
	bool allPeersConnected = BottangoCore::isRelayBridge &&
		BottangoCore::relayPool->bridgeIsConnectedToAllPeers();
	bool resumePeerTime = BottangoCore::isRelayBridge &&
		!commandStreamIsSetup &&
		!startingPeerCommandsSent &&
		allPeersConnected;

	if (BottangoCore::isRelayBridge && waitingForAllPeers && allPeersConnected)
	{
		waitingForAllPeers = false;
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled())
#endif
		{
			Outgoing::printOutputStringFlash(F("All connected"));
			Outgoing::printLine();
		}
#endif

#ifdef ENABLE_STATUS_LIGHTS
		if (streamIsInProgress() && !commandStreamIsSetup)
		{
			StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_OFFLINEPLAY);
		}
		else
		{
			StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_OFFLINEREADY);
		}
#endif
	}

#endif

	if (streamIsInProgress())
	{
		while (commandStream->readyForNextCommand())
		{

#ifdef RELAY_SUPPORTED
			if (BottangoCore::isRelayBridge)
			{
				// need to wait for the to peer queue to clear out?
				if (BottangoCore::relayPool->toPeerQueueFull())
				{
					return;
				}

				// in setup, as a bridge, and we have a non register control command cached
				if (commandStreamIsSetup && cachedPostControlRegisterCommand[0] != '\0')
				{
					// ready to execute that cached command?
					if (BottangoCore::relayPool->bridgeIsConnectedToAllPeers())
					{
						BottangoCore::executeCommand(cachedPostControlRegisterCommand, false);
						cachedPostControlRegisterCommand[0] = '\0';

						if (BottangoCore::relayPool->toPeerQueueFull())
						{
							return;
						}
					}
					// keep waiting for all peers to be connected
					else
					{
						if (!waitingForAllPeers)
						{
							waitingForAllPeers = true;
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
							if (PersistentConfigUtil::debugEnabled())
#endif
							{
								Outgoing::printOutputStringFlash(F("Starting wait"));
								Outgoing::printLine();
							}
#endif

#ifdef ENABLE_STATUS_LIGHTS
							StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_YELLOW);
#endif
						}
						return;
					}
				}
			}

#endif
			char commandBuffer[MAX_COMMAND_LENGTH];
			commandBuffer[0] = '\0';

			commandStream->getNextCommand(commandBuffer);
			if (commandBuffer[0] != '\0')
			{

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
				if (PersistentConfigUtil::debugEnabled())
#endif
				{
					Outgoing::printOutputStringFlash(F("Exe exported cmd: "));
					Outgoing::printOutputStringMem(commandBuffer);
					Outgoing::printLine();
				}
#endif

#ifdef RELAY_SUPPORTED
				// wait to send any setup to peer commands until all peers are connected
				if (BottangoCore::isRelayBridge && commandStreamIsSetup)
				{
					// not connected yet?
					if (!BottangoCore::relayPool->bridgeIsConnectedToAllPeers())
					{
						// haven't cached anything, and we hit a command that doesn't start with regsiter relay command
						// so cache it
						if (cachedPostControlRegisterCommand[0] == '\0' && strncmp(commandBuffer, BasicCommands::REGISTER_RELAY, sizeof(BasicCommands::REGISTER_RELAY) - 1) != 0)
						{
							if (!waitingForAllPeers)
							{
								waitingForAllPeers = true;
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
								if (PersistentConfigUtil::debugEnabled())
#endif
								{
									Outgoing::printOutputStringFlash(F("Starting wait"));
									Outgoing::printLine();
								}
#endif
#ifdef ENABLE_STATUS_LIGHTS
								StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_YELLOW);
#endif
							}
							strcpy(cachedPostControlRegisterCommand, commandBuffer);
							return; // and break out for now
						}
					}
				}
#endif
				BottangoCore::executeCommand(commandBuffer, false);
			}
		}

		if (commandStream->complete())
		{

#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
			if (PersistentConfigUtil::debugEnabled())
#endif
			{
				Outgoing::printOutputStringFlash(F("stream complete"));
				Outgoing::printLine();
			}
#endif

			if (commandStreamIsSetup)
			{
				executeStop(true); // stop with permission to destroy setup
			}
			else
			{
				stop();
			}
			commandStreamIsSetup = false;
		}
	}

#ifdef RELAY_SUPPORTED
	// after all initial curves are sent, set time back to start
	if (resumePeerTime)
	{
		BottangoCore::relayPool->resumeTimeConnectedPeers(false);
		Time::syncTime(0);
		startingPeerCommandsSent = true;
	}
#endif
}

void CommandStreamProvider::updateOnLoop()
{
#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
	runInProgressCommand();

	if (commandStream != nullptr)
	{
		commandStream->updateOnLoop();
	}

	if (streamIsInProgress() && commandStreamIsSetup)
	{
		return;
	}

	ExportedAnimationPlaybackControl::updatePlaybackStatus();
#endif
}

void CommandStreamProvider::stop()
{
	executeStop(false);
}

void CommandStreamProvider::forceStopForTeardown()
{
	executeStop(true);
}

void CommandStreamProvider::executeStop(bool allowSetupStop)
{
	if (!BottangoCore::isOffline())
	{
		return;
	}

	if (commandStream != nullptr)
	{
		if (commandStreamIsSetup && !allowSetupStop)
		{
			// don't exit out of setup unless explicitly allowed
			return;
		}

		delete commandStream;
	}
	commandStream = nullptr;

	BottangoCore::effectorPool.clearAllCurves();

#ifdef RELAY_SUPPORTED
	waitingForAllPeers = false;
	if (BottangoCore::isRelayBridge)
	{
		BottangoCore::relayPool->clearCurvesOnConnectedPeers();
	}
#endif

#ifdef ENABLE_STATUS_LIGHTS
	StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_OFFLINEREADY);
#endif
}

bool CommandStreamProvider::streamIsInProgress()
{
	if (!BottangoCore::isOffline())
	{
		return false;
	}
	return commandStream != nullptr;
}

void CommandStreamProvider::setInvalidState()
{
	invalidState = true;
#ifdef ENABLE_STATUS_LIGHTS
	StatusLights::setDesiredColor(SIGNAL_STATUS_LIGHT, STATUS_COLOR_SIGNAL_EXPORT_SD_ERROR);
#endif
}
