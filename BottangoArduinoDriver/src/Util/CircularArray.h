#pragma once

#include <Arduino.h>
#include "../System/Errors.h"

/**
 * @brief A circular array implementation that allows for efficient storage and retrieval of elements in a fixed-size buffer.
 * @tparam T The type of elements stored in the circular array.
 */
template <class T>
class CircularArray
{
public:
	/**
	 * @brief Constructor for the CircularArray class. Initializes the array with a specified capacity and sets the index to 0.
	 * @tparam T The type of elements stored in the circular array.
	 * @param capacity The maximum number of elements the circular array can hold.
	 */
	explicit CircularArray(int capacity);

	/**
	 * @brief Inserts a new element at the end of the circular array. If the array is full, an error is reported and the element is not added.
	 * @tparam T The type of elements stored in the circular array.
	 * @param data The element to be added to the circular array.
	 */
	void pushBack(T* data);

	/**
	 * @brief Removes a specific element from the circular array. If the element is found, it is removed and the subsequent elements are shifted to fill the gap.
	 * @note Does not delete the element itself, only removes it from the array.
	 * @tparam T The type of elements stored in the circular array.
	 * @param toRemove  The element to be removed from the circular array.
	 */
	void remove(T* toRemove);

	/**
	 * @brief Helper function to retrieve an element at a specific index in the circular array. Returns a pointer to the element if it exists, or NULL if the index is out of bounds.
	 * @tparam T The type of elements stored in the circular array.
	 * @param index The index of the element to retrieve. Must be less than the current size of the array.
	 * @return The element at the specified index, or NULL if the index is invalid.
	 */
	T* get(int index) const;

	/**
	 * @brief Helper function to get the current number of elements in the circular array. Returns the number of elements currently stored in the array.
	 * @tparam T The type of elements stored in the circular array.
	 * @return The number of elements currently stored in the circular array.
	 */
	byte size() const;

	/**
	 * @brief Clears the circular array by setting all elements to NULL and resetting the index to 0. This effectively removes all elements from the array without deallocating the memory.
	 * @note Does not delete the elements themselves, only clears the references in the array.
	 * @tparam T The type of elements stored in the circular array.
	 */
	void clear();

private:
	T** m_array;
	byte m_capacity;
	byte m_idx;
}; 

template <class T>
CircularArray<T>::CircularArray(int capacity)
{
	m_capacity = capacity;
	m_array = new T * [m_capacity];
	m_idx = 0;

	for (int i = 0; i < m_capacity; ++i)
	{
		m_array[i] = NULL;
	}
}

template <class T>
void CircularArray<T>::pushBack(T* data)
{
	if (m_idx >= m_capacity)
	{
		Error::reportError_NoSpaceAvailable();
		return;
	}

	m_array[m_idx++] = data;
}

template <class T>
void CircularArray<T>::remove(T* toRemove)
{
	// find index of element to remove
	int index = -1;
	for (int i = 0; i < m_capacity; i++)
	{
		if (m_array[i] == toRemove)
		{
			index = i;
			break;
		}
	}

	// abort if not found
	if (index == -1)
	{
		return;
	}

	// Move everything ahead of the found index back one index
	for (int i = index; i < m_capacity - 1; i++)
	{
		m_array[i] = m_array[i + 1];
	}

	// if next add would add past max, move next add back to end
	if (m_idx >= m_capacity)
	{
		m_idx = m_capacity - 1;
		m_array[m_idx] = NULL;
	}
	// otherwise just clear at wherever idx is and move idx down
	else
	{
		m_array[m_idx] = NULL;
		m_idx--;
	}
}

template <class T>
T* CircularArray<T>::get(int index) const
{
	return m_array[index];
}

template <class T>
byte CircularArray<T>::size() const
{
	return m_idx;
}

template <class T>
void CircularArray<T>::clear()
{
	for (int i = 0; i < m_capacity; ++i)
	{
		m_array[i] = NULL;
	}
	m_idx = 0;
}