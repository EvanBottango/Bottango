#include "../../../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#include "I2SAudioModule.h"
#include "I2SAudEventStatusResponder.h"
#include "DataSource/SDCardUtil.h"
#include "Modules/Outgoing.h"
#include "BottangoCore.h"
#include "System/Systemstatus.h"

void I2SAudioModule::onPhase(Phase p)
{
	if (p != Phase::Input) return;

	if (playing)
	{
		updateVolume();
	}
}

void I2SAudioModule::updateVolume()
{
#ifdef DYNAMIC_VOLUME
	if (millis() - lastVolumeTime > VOLUME_READ_INTERVAL)
	{
		lastVolumeTime = millis();
		uint32_t lastVolumeRead = analogRead(VOLUME_PIN);
		float volume = VOLUME_MIN + (VOLUME_MAX - VOLUME_MIN) * ((float)lastVolumeRead / 4095.0);
	}
#else
	volume = 0.1f;
#endif

}

void I2SAudioModule::init()
{
#ifdef DYNAMIC_VOLUME
	pinMode(VOLUME_PIN, INPUT);
#endif
}

void I2SAudioModule::checkAudioSource(const char* effectorIdentifier, const char* hash)
{
	Outgoing::printOutputStringFlash(F("I2S CheckAudioSource"));
	Outgoing::printLine();

	int responderCode = 0;
	SDCardUtil::SDFileError fileError;

	// Open Hash File
	char hashBuffer[FILE_HASH_LENGTH + 1];
	char filePathBuffer[MAX_FILE_PATH_SIZE];
	
	SDCardUtil::getAudioHashFilePath(effectorIdentifier, filePathBuffer);

	File hashFile = SDCardUtil::openFile(filePathBuffer, fileError);
	
	// Open and check hash file
	// got a hash file
	if (fileError == SDCardUtil::SDFileError::ERR_NONE)
	{
		SDCardUtil::lockCard();
		size_t bytesRead = hashFile.readBytes(hashBuffer, FILE_HASH_LENGTH);
		SDCardUtil::unlockCard();

		hashBuffer[bytesRead] = '\0';

		if (strcmp(hashBuffer, hash) == 0)
		{
			responderCode = I2S_AUDIO_STATUS_READY;
		}
		else
		{
			responderCode = I2S_AUDIO_STATUS_NO_HASH_MATCH_ON_CARD;
		}
	}
	// no hash file
	else
	{
		if (fileError == SDCardUtil::SDFileError::ERR_NO_CARD)
		{
			responderCode = I2S_AUDIO_STATUS_NO_SD_CARD;
		}
		else if (fileError == SDCardUtil::ERR_FILE_NOT_FOUND || fileError == SDCardUtil::ERR_IO || !hashFile)
		{
			responderCode = I2S_AUDIO_STATUS_NO_HASH_ON_CARD;
		}
	}
	SDCardUtil::closeFile(hashFile);	

	// if we didn't get a bad card error
	// open check and init audio file itself from header
	if (fileError != SDCardUtil::SDFileError::ERR_NO_CARD)
	{
		// open audio file
		filePathBuffer[0] = '\0';
		SDCardUtil::getAudioFilePath(effectorIdentifier, filePathBuffer);
		File audioFile = SDCardUtil::openFile(filePathBuffer, fileError);
		// got audio file
		if (fileError == SDCardUtil::ERR_NONE)
		{
			bool headerOk = true;

			// Read number of channels (position 22 in WAV header)
			SDCardUtil::lockCard();
			uint16_t numChannels;
			audioFile.seek(22);
			size_t channelsRead = audioFile.read((uint8_t*)&numChannels, sizeof(numChannels));
			if (channelsRead != sizeof(numChannels))
			{
				headerOk = false;
			}
			else
			{
				// Configure I2S based on the number of channels
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
				if (numChannels == 1)
				{
					channelFormat_FromHeader = I2S_SLOT_MODE_MONO; // Mono configuration
				}
				else if (numChannels == 2)
				{
					channelFormat_FromHeader = I2S_SLOT_MODE_STEREO; // Stereo configuration
				}
				else
				{
					headerOk = false;
				}
#else
				if (numChannels == 1)
				{
					channelFormat_FromHeader = I2S_CHANNEL_FMT_ONLY_RIGHT; // Mono configuration
				}
				else if (numChannels == 2)
				{
					channelFormat_FromHeader = I2S_CHANNEL_FMT_RIGHT_LEFT; // Stereo configuration
				}
				else
				{
					headerOk = false;
				}
#endif
#endif
			}

			// Read WAV header to get the sample rate
			audioFile.seek(24); // Offset to sample rate field
			size_t rateRead = audioFile.read((uint8_t*)&sampleRate_FromHeader, sizeof(sampleRate_FromHeader));
			if (rateRead != sizeof(sampleRate_FromHeader))
			{
				headerOk = false;
			}

			audioFile.seek(34); // Offset to bits per sample field
			size_t bitsRead = audioFile.read((uint8_t*)&bitPerSample, sizeof(bitPerSample));
			if (bitsRead != sizeof(bitPerSample))
			{
				headerOk = false;
			}

			SDCardUtil::unlockCard();

			// ToDo: This is not needed. The enum already resolves to the correct value. A 8,16,24,32 from header is directly usable.
			// This applies to both versions.
			// Simply do a safety-switch case with case 8: case:16 [..] and then static_cast<i2s_data_bit_width_t>(bitPerSample)
			// To even further simplify, remove the #ifdef: Just do the plausibility check and cast to the correct enum type below in the task
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
			// Map bits per sample to I2S bits per sample type
			switch (bitPerSample)
			{
			case 8:
				BitsPerSample_FromHeader = I2S_DATA_BIT_WIDTH_8BIT;
				break;
			case 16:
				BitsPerSample_FromHeader = I2S_DATA_BIT_WIDTH_16BIT;
				break;
			case 24:
				BitsPerSample_FromHeader = I2S_DATA_BIT_WIDTH_24BIT;
				break;
			case 32:
				BitsPerSample_FromHeader = I2S_DATA_BIT_WIDTH_32BIT;
				break;
			default:
				headerOk = false;
				break;
			}

#else
			// Map bits per sample to I2S bits per sample type
			switch (BitsPerSample_Current)
			{
			case 8:
				BitsPerSample_FromHeader = I2S_BITS_PER_SAMPLE_8BIT;
				break;
			case 16:
				BitsPerSample_FromHeader = I2S_BITS_PER_SAMPLE_16BIT;
				break;
			case 24:
				BitsPerSample_FromHeader = I2S_BITS_PER_SAMPLE_24BIT;
				break;
			case 32:
				BitsPerSample_FromHeader = I2S_BITS_PER_SAMPLE_32BIT;
				break;
			default:
				headerOk = false;
				break;
			}
#endif
#endif

			if (!headerOk)
			{
				Outgoing::printOutputStringFlash(F("I2S header parse failed for effector: "));
				Outgoing::printOutputStringMem(effectorIdentifier);
				Outgoing::printLine();
			}
			else
			{
				audioSourceOK = true;
			}
		}
		// couldn't open audio file
		else
		{
			if (fileError == SDCardUtil::SDFileError::ERR_NO_CARD)
			{
				responderCode = I2S_AUDIO_STATUS_NO_SD_CARD;
			}
			else if (fileError == SDCardUtil::ERR_FILE_NOT_FOUND || fileError == SDCardUtil::ERR_IO || !hashFile)
			{
				responderCode = I2S_AUDIO_STATUS_NO_FILE_ON_CARD;
			}
		}
		SDCardUtil::closeFile(audioFile);
	}

	// report status of sd card audio file via a multi message responder
	if (BottangoCore::activeOutgoingMultimessage != nullptr)
	{
		// shouldn't have an active...
		BottangoCore::activeOutgoingMultimessage->cleanUpMultiMessage();
		BottangoCore::activeOutgoingMultimessage = nullptr;
	}
	BottangoCore::activeOutgoingMultimessage = new I2SAudEventStatusResponder(responderCode);

	BottangoCore::activeOutgoingMultimessage->initializeMultiMessage(Outgoing::get());

#if defined(USE_CODE_COMMAND_STREAM) || defined(USE_SD_CARD_COMMAND_STREAM)
	if (SystemStatus::systemStatus.ConnectionStatus == SystemStatus::eConnectionStatus::Export_Playback
		&& !(responderCode == I2S_AUDIO_STATUS_READY || responderCode == I2S_AUDIO_STATUS_NO_HASH_MATCH_ON_CARD))
	{
		BottangoCore::mMaster.getModule<AnimationPlaybackControl>(Modules::AnimPlaybackCntrl)->setInvalidState();
#ifdef EXPORTED_ANIM_LOGGING
#ifdef TOGGLE_DEBUG
		if (PersistentConfigUtil::debugEnabled() || ALWAYS_LOG_ERROR_CASE)
#endif
		{
			Outgoing::printOutputStringFlash(F("Exported Anim, Audio File SD Error: "));
			Outgoing::printOutputStringMem(effectorIdentifier);
			Outgoing::printOutputStringFlash(F(" code: "));
			Outgoing::printOutputStringMem(responderCode);
			Outgoing::printLine();
		}

#endif
	}
#endif
}

void I2SAudioModule::play(const char* effectorIdentifier, uint32_t offsetMS)
{
	char filePath[MAX_FILE_PATH_SIZE];
	filePath[0] = '\0';
	SDCardUtil::getAudioFilePath(effectorIdentifier, filePath);

	int totalOffset = 44; // Assuming 44-byte standard WAV header
	if (offsetMS > 0)
	{
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
		byte numChannels = (channelFormat_FromHeader == I2S_SLOT_MODE_STEREO) ? 2 : 1;
#else
		byte numChannels = (channelFormat_FromHeader == I2S_CHANNEL_FMT_RIGHT_LEFT) ? 2 : 1;
#endif
#endif

		uint32_t bytesPerSample = (BitsPerSample_Current / 8) * numChannels;
		uint32_t samplesToProgress = (sampleRate_FromHeader * offsetMS) / 1000;
		uint32_t byteOffset = samplesToProgress * bytesPerSample;

		// Skip the WAV header (typically the first 44 bytes) and offset
		totalOffset += byteOffset;
	}

	// ############################################################
	// Copy from I2SHelper
	// ############################################################
	
	// Update volume once before starting playback
	updateVolume();


#ifdef PIN_ON_AUDIO_PLAY
	pinMode(AUDIO_ENABLE_PIN, OUTPUT);
	digitalWrite(AUDIO_ENABLE_PIN, LOW);
#endif

	bool configChanged = false;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
	// Did the audio format actually change?
	if (sampleRate_Current != sampleRate_FromHeader ||
		BitsPerSample_Current != BitsPerSample_FromHeader ||
		channelFormat_Current != channelFormat_FromHeader)
	{
		configChanged = true;
	}

	// Store new audio config from WAV header
	sampleRate_Current = sampleRate_FromHeader;
	BitsPerSample_Current = BitsPerSample_FromHeader;
	channelFormat_Current = channelFormat_FromHeader;
#else


	// Did the audio format actually change?
	if (i2sConfig.sample_rate != sampleRate_FromHeader ||
		i2sConfig.bits_per_sample != BitsPerSample_FromHeader ||
		i2sConfig.channelFormat_FromHeader != channelFormat_FromHeader)
	{
		configChanged = true;
	}

	// generate config based on header read at init
	i2sConfig = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
		.sample_rate = sampleRate_FromHeader,
		.bits_per_sample = BitsPerSample_FromHeader,
		.channel_format = channelFormat_FromHeader,
		.communication_format = I2S_COMM_FORMAT_STAND_I2S,
		.intr_alloc_flags = 0,
		.dma_buf_count = 16,
		.dma_buf_len = 256 };
#endif
#endif

	// queue new file for playback
	strncpy(pendingFilePath, filePath, sizeof(pendingFilePath));
	pendingByteOffset = totalOffset;
	fileOnDeck = true;

	// only ask the task to reconfigure if:
	//  - the task is already running, AND
	//  - the format actually changed
	if (i2sTaskHandle != nullptr && configChanged)
	{
		reconfigureRequested = true;
	}

	// ensure the I2S task is running (in case init() wasn't called)
	if (i2sTaskHandle == nullptr)
	{
		// This will start I2S using the *current* config (which we just updated),
		// and the task will immediately begin streaming (zeros until the file kicks in).
		init_I2S_Task();
	}
	else if (playing)
	{
		// interrupt current playback so the new file can take over
		stopPlaybackRequested = true;
	}
}

void I2SAudioModule::init_I2S_Task()
{
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)

	// If nothing configured yet, pick a default "silence" format
	if (sampleRate_Current == 0)
	{
		sampleRate_Current = I2S_INIT_SAMPLE_RATE;
		BitsPerSample_Current = I2S_DATA_BIT_WIDTH_16BIT;
		channelFormat_Current = I2S_SLOT_MODE_STEREO;
	}
#else
	// If nothing configured yet, pick a default "silence" format
	if (i2sConfig.sample_rate == 0)
	{
		i2sConfig.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
		i2sConfig.sample_rate = I2S_INIT_SAMPLE_RATE;
		i2sConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
		i2sConfig.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
		i2sConfig.communication_format = I2S_COMM_FORMAT_STAND_I2S;
		i2sConfig.intr_alloc_flags = 0;
		i2sConfig.dma_buf_count = I2S_DMA_BUFF_COUNT;
		i2sConfig.dma_buf_len = I2S_DMA_BUFF_LEN;
	}
#endif
#endif
	start_I2S_Task();
}

void I2SAudioModule::start_I2S_Task()
{
	// only initialize once
	if (i2sTaskHandle != nullptr)
	{
		return;
	}

	stopTask = false;
	stopPlaybackRequested = false;
	//cachedBytes = 0;

#ifdef PIN_ON_AUDIO_PLAY
	digitalWrite(AUDIO_ENABLE_PIN, HIGH);
#endif

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
	i2s.setPins(I2S_BCLK, I2S_LRC, I2S_DOUT);
	// use whatever config is currently stored (default or from last startPlaying)
	i2s.begin(I2S_MODE_STD, sampleRate_Current, BitsPerSample_Current, channelFormat_Current);
#else
	// use whatever config is currently stored (default or from last startPlaying)
	i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);
	i2s_set_pin(I2S_NUM_0, &pinConfig);
#endif
#endif

	xTaskCreate(
		i2s_Task,       // Task function
		"i2SPlay",     // Name of the task
		4096,          // Stack size in words
		this,          // Task input parameter
		1,             // Priority of the task
		&i2sTaskHandle // Task handle
	);
}

void I2SAudioModule::stop()
{
	// request current playback to stop; task will switch to zeros
	stopPlaybackRequested = true;
}

bool I2SAudioModule::isPlaying() const
{
	return playing;
}

void i2s_Task(void* param)
{
	I2SAudioModule* module = static_cast<I2SAudioModule*>(param);

	SDCardUtil::SDFileError err = SDCardUtil::SDFileError::ERR_NONE;
	File audioFile;
	uint32_t cachedBytes = 0;
	//bool fileOnDeck = false;
	//char pendingFilePath[MAX_FILE_PATH_SIZE] = { 0 };
	//uint32_t pendingByteOffset = 0;
	char currentFilePath[MAX_FILE_PATH_SIZE] = { 0 };
	//uint32_t currentByteOffset = 0;
	uint8_t buffer[I2S_BUFFER_SIZE];
	uint8_t cacheBuffer[I2S_BUFFER_SIZE];
	uint8_t volumeBuffer[I2S_BUFFER_SIZE];

	memset(buffer, 0, sizeof(buffer));
	memset(cacheBuffer, 0, sizeof(cacheBuffer));
	memset(volumeBuffer, 0, sizeof(volumeBuffer));

	while (!module->stopTask)
	{
		// handle external request to stop current playback
		if (module->stopPlaybackRequested)
		{
			module->stopPlaybackRequested = false;
			module->playing = false;
			cachedBytes = 0;
			SDCardUtil::closeFile(audioFile);
			audioFile = File();
		}

		// handle pending I2S reconfiguration (e.g., first real audio after silence)
		if (module->reconfigureRequested)
		{
			module->reconfigureRequested = false;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
			module->i2s.end();
			module->i2s.begin(I2S_MODE_STD, module->sampleRate_Current, module->BitsPerSample_Current, module->channelFormat_Current);
#else
			// map channel_format to mono/stereo for clock
			i2s_channel_t ch = I2S_CHANNEL_STEREO;
			if (i2sConfig.channel_format == I2S_CHANNEL_FMT_ONLY_RIGHT)
			{
				ch = I2S_CHANNEL_MONO;
			}
			i2s_set_clk(I2S_NUM_0, i2sConfig.sample_rate, i2sConfig.bits_per_sample, ch);
#endif
#endif
		}

		// handle a new file queued for playback
		if (!module->playing && module->fileOnDeck)
		{
			SDCardUtil::closeFile(audioFile);
			audioFile = File();

			// openFile already does its own lockCard/unlockCard.
			audioFile = SDCardUtil::openFile(module->pendingFilePath, err);
			if (err == SDCardUtil::SDFileError::ERR_NONE && audioFile)
			{
				SDCardUtil::lockCard();
				audioFile.seek(module->pendingByteOffset);
				SDCardUtil::unlockCard();

				strncpy(currentFilePath, module->pendingFilePath, sizeof(currentFilePath));
				//currentByteOffset = module->pendingByteOffset;
				module->playing = true;
			}
			else
			{
				module->playing = false;
			}

			module->fileOnDeck = false;
		}

		size_t bytesRead = 0;

		// normal audio playback path
		if (module->playing && audioFile)
		{
			SDCardUtil::lockCard();
			bool hasMore = (audioFile.available() || cachedBytes > 0);
			SDCardUtil::unlockCard();

			if (!hasMore)
			{
				// end of file
				module->playing = false;
				SDCardUtil::closeFile(audioFile);
				audioFile = File();
			}
			else
			{
				// restore cached to main buffer
				if (cachedBytes > 0)
				{
					bytesRead = cachedBytes;
					memcpy(buffer, cacheBuffer, cachedBytes); // Copy cached data to the primary buffer
					cachedBytes = 0;                          // Clear cache
				}

				SDCardUtil::lockCard();
				bool canRead = audioFile.available() && bytesRead < I2S_BUFFER_SIZE;
				SDCardUtil::unlockCard();

				// fill remaining buffer with read from SD
				if (canRead)
				{
					SDCardUtil::lockCard();
					size_t additionalBytes = audioFile.read(volumeBuffer, I2S_BUFFER_SIZE - bytesRead);
					SDCardUtil::unlockCard();

/*#ifdef DYNAMIC_VOLUME
					float min = VOLUME_MIN;
					float max = VOLUME_MAX;
					float volume = min + (max - min) * ((float)module->lastVolumeRead / 4095.0);
#else
					float volume = 0.1f; // Volume scale factor (0.0 to 1.0)
#endif*/

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
					switch (module->BitsPerSample_Current)
					{
					case I2S_DATA_BIT_WIDTH_8BIT:
#else
					switch (i2sConfig.bits_per_sample)
					{
					case I2S_BITS_PER_SAMPLE_8BIT:
#endif
#endif
					{
						uint8_t* sampleBuffer8 = volumeBuffer;
						for (size_t i = 0; i < additionalBytes; i++)
						{
							sampleBuffer8[i] = static_cast<uint8_t>(static_cast<float>(sampleBuffer8[i]) * module->volume);
						}
						break;
					}
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
					case I2S_DATA_BIT_WIDTH_16BIT:
#else
					case I2S_BITS_PER_SAMPLE_16BIT:
#endif
#endif
					{
						int16_t* sampleBuffer16 = reinterpret_cast<int16_t*>(volumeBuffer);
						for (size_t i = 0; i < additionalBytes / 2; i++)
						{
							sampleBuffer16[i] = static_cast<int16_t>(static_cast<float>(sampleBuffer16[i]) * module->volume);
						}
						break;
					}
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
					case I2S_DATA_BIT_WIDTH_24BIT:
#else
					case I2S_BITS_PER_SAMPLE_24BIT:
#endif
#endif
					{
						uint8_t* byteBuffer24 = volumeBuffer;
						for (size_t i = 0; i < additionalBytes / 3; i++)
						{
							size_t base = i * 3;
							int32_t sample = (int32_t)((byteBuffer24[base + 0]) |
								(byteBuffer24[base + 1] << 8) |
								(byteBuffer24[base + 2] << 16));
							if (sample & 0x00800000)
							{
								sample |= static_cast<int32_t>(0xFF000000);
							}
							sample = static_cast<int32_t>(static_cast<float>(sample) * module->volume);
							byteBuffer24[base + 0] = sample & 0xFF;
							byteBuffer24[base + 1] = (sample >> 8) & 0xFF;
							byteBuffer24[base + 2] = (sample >> 16) & 0xFF;
						}
						break;
					}
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
					case I2S_DATA_BIT_WIDTH_32BIT:
#else
					case I2S_BITS_PER_SAMPLE_32BIT:
#endif
#endif
					{
						int32_t* sampleBuffer32 = reinterpret_cast<int32_t*>(volumeBuffer);
						for (size_t i = 0; i < additionalBytes / 4; i++)
						{
							sampleBuffer32[i] = static_cast<int32_t>(static_cast<float>(sampleBuffer32[i]) * module->volume);
						}
						break;
					}
					}

					memcpy(buffer + bytesRead, volumeBuffer, additionalBytes);
					bytesRead += additionalBytes; // Update bytesRead with the newly read data
					}
				}
			}

		// if no audio data was produced this iteration, stream zeros
		if (bytesRead == 0)
		{
			memset(buffer, 0, I2S_BUFFER_SIZE);
			bytesRead = I2S_BUFFER_SIZE;
		}

		if (bytesRead > 0)
		{
			size_t bytesWritten = 0;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
			bytesWritten = module->i2s.write(buffer, bytesRead);
			if (bytesWritten < bytesRead)
			{
#else
			esp_err_t result = i2s_write(I2S_NUM_0, buffer, bytesRead, &bytesWritten, I2S_WRITE_TIMEOUT);
			if (result != ESP_OK || bytesWritten < bytesRead)
			{
#endif
#endif
				size_t unwrittenBytes = bytesRead - bytesWritten;
				memcpy(cacheBuffer, buffer + bytesWritten, unwrittenBytes);
				cachedBytes = unwrittenBytes;
			}
			}
		}

	// full teardown requested
	module->playing = false;
#ifdef PIN_ON_AUDIO_PLAY
	digitalWrite(AUDIO_ENABLE_PIN, LOW);
#endif
	SDCardUtil::closeFile(audioFile);

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
	module->i2s.end();
#else
	i2s_driver_uninstall(I2S_NUM_0);
#endif
#endif

	module->stopTask = false;
	module->i2sTaskHandle = nullptr;

	vTaskDelete(NULL);
}


#endif