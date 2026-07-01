#pragma once

#include "../../BottangoArduinoModules.h"
#if defined(RELAY_SUPPORTED) || defined(USE_SD_CARD_COMMAND_STREAM)

#include <cstring>
#include "../../BottangoArduinoConfig.h"
#include "../Communication/Outgoing.h"

#ifdef ESP32
#include <freertos/semphr.h>
#endif

/**
 * @brief This class implements a circular buffer for storing text data. It is designed to handle strings and characters, providing methods to add, retrieve, and manage the buffer's contents.
 * The buffer size is defined at compile time through the template parameter BufferSize.
 * @tparam BufferSize The size of the buffer.
 */
template <unsigned int BufferSize>
class TxtBuffer
{
public:
	TxtBuffer()
	{
		m_head = 0;
		m_tail = 0;
		m_buffer[0] = '\0';

#ifdef ESP32
		m_mutex = xSemaphoreCreateMutex();
		configASSERT(m_mutex);
#endif
	}

	/**
	 * @brief Adds a string to the buffer. If the string is too long, it will be truncated to MAX_COMMAND_LENGTH. If the buffer is full, the characters will be discarded.
	 * @param txt The string to add to the buffer., must be null-terminated.
	 */
	void addTxt(const char* txt)
	{
		int len = strlen(txt);
		if (len > MAX_COMMAND_LENGTH)
			len = MAX_COMMAND_LENGTH;

#ifdef ESP32
		xSemaphoreTake(m_mutex, portMAX_DELAY);
#endif

		for (int i = 0; i < len; ++i)
		{
			if (isFullInternal())
			{
				break;
			}
			m_buffer[m_head] = txt[i];
			m_head = (m_head + 1) % BufferSize;

			// Handle overflow by advancing tail if head catches up
			if (m_head == m_tail)
			{
				m_tail = (m_tail + 1) % BufferSize;
			}
		}
#ifdef ESP32
		xSemaphoreGive(m_mutex);
#endif
	}

	/**
	 * @brief Adds a single character to the buffer. If the buffer is full, the character will be discarded.
	 * @param c Character to add to the buffer.
	 */
	void addChar(const char c)
	{
#ifdef ESP32
		xSemaphoreTake(m_mutex, portMAX_DELAY);
#endif
		if (!isFullInternal())
		{
			m_buffer[m_head] = c;
			m_head = (m_head + 1) % BufferSize;
			if (m_head == m_tail)
			{
				m_tail = (m_tail + 1) % BufferSize;
			}
		}
#ifdef ESP32
		xSemaphoreGive(m_mutex);
#endif
	}

	/**
	 * @brief Indicates whether there is space available in the buffer for more data.
	 * @return True if there is space available, false if the buffer is full.
	 */
	bool available() const
	{
#ifdef ESP32
		xSemaphoreTake(m_mutex, portMAX_DELAY);
#endif
		bool has = (m_head != m_tail);
#ifdef ESP32
		xSemaphoreGive(m_mutex);
#endif
		return has;
	}

	/**
	 * @brief Returns the number of bytes available in the buffer for reading.
	 * @return Number of bytes available to read.
	 */
	int getSpaceAvailable() const
	{
#ifdef ESP32
		xSemaphoreTake(m_mutex, portMAX_DELAY);
#endif
		int space;
		if (m_head >= m_tail)
		{
			space = BufferSize - (m_head - m_tail) - 1;
		}
		else
		{
			space = (m_tail - m_head) - 1;
		}
#ifdef ESP32
		xSemaphoreGive(m_mutex);
#endif
		return space;
	}

	/**
	 * @brief Returns the number of bytes currently used in the buffer.
	 * @return Number of bytes currently used in the buffer.
	 */
	int getSpaceUsed() const
	{
#ifdef ESP32
		xSemaphoreTake(m_mutex, portMAX_DELAY);
#endif
		int used;
		if (m_head >= m_tail)
		{
			used = m_head - m_tail;
		}
		else
		{
			used = BufferSize - (m_tail - m_head);
		}
#ifdef ESP32
		xSemaphoreGive(m_mutex);
#endif
		return used;
	}

	/**
	 * @brief Retrieves the next complete line from the buffer, if available.
		If a complete line is found, it is copied to outTxt and the buffer is advanced unless peek is true.
		If no complete line is found, outTxt will be set to an empty string.
	 * @param outTxt Buffer to store the retrieved line. Must be large enough to hold MAX_COMMAND_LENGTH characters.
	 * @param peek If true, the buffer will not be advanced after retrieving the line. If false, the buffer will be advanced past the retrieved line.
	 * @return True if a complete line was found and copied to outTxt, false if no complete line was found, or if the buffer is empty.
	 */
	bool getNextTxt(char* outTxt, bool peek = false)
	{
#ifdef ESP32
		xSemaphoreTake(m_mutex, portMAX_DELAY);
#endif
		// No data
		if (m_head == m_tail)
		{
			outTxt[0] = '\0';
#ifdef ESP32
			xSemaphoreGive(m_mutex);
#endif
			return false;
		}

		// Scan up to MAX_COMMAND_LENGTH for a newline
		int tempTail = m_tail;
		char temp[MAX_COMMAND_LENGTH];
		int len = 0;
		bool foundNewline = false;
		while (tempTail != m_head && len < MAX_COMMAND_LENGTH - 1)
		{
			char c = m_buffer[tempTail];
			temp[len++] = c;
			tempTail = (tempTail + 1) % BufferSize;
			if (c == '\n')
			{
				foundNewline = true;
				break;
			}
		}

		if (!foundNewline)
		{
			// No complete line, return empty
			outTxt[0] = '\0';
#ifdef ESP32
			xSemaphoreGive(m_mutex);
#endif
			return false;
		}
		else
		{
			// Copy out the full line
			memcpy(outTxt, temp, len);
			outTxt[len] = '\0';
			if (!peek)
			{
				// Advance tail past the line
				m_tail = tempTail;
			}
		}

#ifdef ESP32
		xSemaphoreGive(m_mutex);
#endif
		return true;
	}

	/**
	 * @brief Returns the next character from the buffer and advances it.
	 * @return The next character from the buffer, or '\0' if the buffer is empty.
	 */
	char getNextChar()
	{
#ifdef ESP32
		xSemaphoreTake(m_mutex, portMAX_DELAY);
#endif
		char c = '\0';
		if (m_head != m_tail)
		{
			c = m_buffer[m_tail];
			m_tail = (m_tail + 1) % BufferSize;
		}
#ifdef ESP32
		xSemaphoreGive(m_mutex);
#endif
		return c;
	}

	/**
	 * @brief Indicates, whether the buffer is full and cannot accept more data.
	 * @return True if the buffer is full, false otherwise.
	 */
	bool isFull() const
	{
#ifdef ESP32
		xSemaphoreTake(m_mutex, portMAX_DELAY);
#endif
		bool full = isFullInternal();
#ifdef ESP32
		xSemaphoreGive(m_mutex);
#endif
		return full;
	}

	/**
	 * @brief Clears the buffer, resetting it to an empty state. All data in the buffer will be discarded.
	 */
	void clear()
	{
#ifdef ESP32
		xSemaphoreTake(m_mutex, portMAX_DELAY);
#endif
		m_head = 0;
		m_tail = 0;
		m_buffer[0] = '\0';
#ifdef ESP32
		xSemaphoreGive(m_mutex);
#endif
	}

private:
	/**
	 * @brief Calculates whether the buffer is full without acquiring a mutex.
	 * @return True if the buffer is full, false otherwise.
	 */
	bool isFullInternal() const
	{
		return ((m_head + 1) % BufferSize) == m_tail;
	}

	char m_buffer[BufferSize];
	int m_head;
	int m_tail;
#ifdef ESP32
	SemaphoreHandle_t m_mutex;
#endif
};

#endif // RELAY_SUPPORTED || USE_SD_CARD_COMMAND_STREAM