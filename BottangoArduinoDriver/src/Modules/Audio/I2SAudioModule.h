#include "../../../BottangoArduinoModules.h"

#ifdef AUDIO_SD_I2S
#ifndef _AudioModule_h
#define _AudioModule_h

#include <Arduino.h>
#include "Module Handling/ModuleMaster.h"
#include "../BottangoArduinoConfig.h"

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
#include "ESP_I2S.h"
#else
#include "driver/i2s.h"
#endif // ESP_ARDUINO_VERSION
#endif // ESP_ARDUINO_VERSION_MAJOR

void i2s_Task(void* param);

class I2SAudioModule : public LoopModule
{
public:
    void onPhase(Phase p) override;
    void init() override;
	void updateVolume();

	void checkAudioSource(const char* effectorIdentifier, const char* hash);
	bool isAudioSourceOK() const { return audioSourceOK; }
	void play(const char* effectorIdentifier, uint32_t offsetMS);
	void stop();
	bool isPlaying() const;
	void init_I2S_Task();	

	/**
	 * @brief Task handle for i2S task
	 */
	TaskHandle_t i2sTaskHandle = nullptr;

	/**
	 * @brief Request to stop current playback but keep I2S/task alive
	 */
	volatile bool stopPlaybackRequested = false;
	/**
	 * @brief Request to reconfigure I2S clock/format to new header values
	 */
	volatile bool reconfigureRequested = false;

	/**
	 * @brief Request to fully stop the I2S task
	 */
	volatile bool stopTask = false;

	/**
	 * @brief Indicated whether audio is currently playing
	 */
	volatile bool playing = false;

	volatile float volume = 0.1f;

#ifdef DYNAMIC_VOLUME
	unsigned long lastVolumeTime = 0;
#endif

	// I2S Task variables
	bool fileOnDeck = false;
	char pendingFilePath[MAX_FILE_PATH_SIZE] = { 0 };
	uint32_t pendingByteOffset = 0;

	
#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
	I2SClass i2s;
	uint32_t sampleRate_Current;
	i2s_data_bit_width_t BitsPerSample_Current;
	i2s_slot_mode_t channelFormat_Current;
#else
	i2s_config_t i2sConfig;
	i2s_pin_config_t pinConfig = {
		.mck_io_num = I2S_PIN_NO_CHANGE,
		.bck_io_num = I2S_BCLK,
		.ws_io_num = I2S_LRC,
		.data_out_num = I2S_DOUT,
		.data_in_num = I2S_PIN_NO_CHANGE };
#endif // ESP_ARDUINO_VERSION
#endif // ESP_ARDUINO_VERSION_MAJOR
private:
	uint32_t sampleRate_FromHeader = 0;
	bool audioSourceOK = false;

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
	uint16_t bitPerSample;
	i2s_data_bit_width_t BitsPerSample_FromHeader;
	i2s_slot_mode_t channelFormat_FromHeader;
#else
	i2s_bits_per_sample_t BitsPerSample_FromHeader;
	i2s_channel_fmt_t channelFormat_FromHeader;
#endif // ESP_ARDUINO_VERSION
#endif // ESP_ARDUINO_VERSION_MAJOR

	 void start_I2S_Task();
};

#endif // _AudioModule_h
#endif // AUDIO_SD_I2S