// AudioInterace.h

#ifndef _AudioInterace_h
#define _AudioInterace_h

#include <Arduino.h>

class IAudioPlayback
{
public:

	virtual void checkAudioSource(const char* effectorIdentifier, const char* hash) = 0;

	virtual bool isAudioSourceOK() const = 0;

	/**
	 * @brief Pure virtual function that starts playback of audio identified by effectorIdentifier at the specified offset in milliseconds.
	 * @param identifier Pointer to a null-terminated string that identifies the effector.
	 * @param offsetMS Playback start offset in milliseconds.
	 */
	virtual void play(const char* effectorIdentifier, uint32_t offsetMS) = 0;

	/**
	 * @brief Pure virtual function that stops audio playback.
	 */
	virtual void stop() = 0;

	/**
	 * @brief Pure virtual function that checks if audio is currently playing.
	 * @return true if audio is playing, false otherwise.
	 */
	virtual bool isPlaying() const = 0;
};

#endif