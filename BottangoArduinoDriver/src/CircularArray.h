
#ifndef BOTTANGO_CIRCULARARRAY_H
#define BOTTANGO_CIRCULARARRAY_H

#include "Errors.h"
#include "Log.h"

template <class T>
class CircularArray
{
    T **array;
    byte capacity;
    byte idx;

public:
    explicit CircularArray(int capacity);

    void pushBack(T *data);

    void remove(T *toRemove);

    T *get(int index);

    byte size();

    void clear();
};

template <class T>
CircularArray<T>::CircularArray(int _capacity)
{
    capacity = _capacity;
    array = new T *[capacity];
    idx = 0;

    for (int i = 0; i < capacity; ++i)
    {
        array[i] = NULL;
    }
}

template <class T>
void CircularArray<T>::pushBack(T *data)
{
    if (idx >= capacity)
    {
        Error::reportError_NoSpaceAvailable();
        return;
    }

    array[idx++] = data;
}

template <class T>
void CircularArray<T>::remove(T *toRemove)
{
    // find index of element to remove
    int index = -1;
    for (int i = 0; i < capacity; i++)
    {
        if (array[i] == toRemove)
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
    for (int i = index; i < capacity - 1; i++)
    {
        array[i] = array[i + 1];
    }

    // if next add would add past max, move next add back to end
    if (idx >= capacity)
    {
        idx = capacity - 1;
        array[idx] = NULL;
    }
    // otherwise just clear at wherever idx is and move idx down
    else
    {
        array[idx] = NULL;
        idx--;
    }
}

template <class T>
T *CircularArray<T>::get(int index)
{
    return array[index];
}

template <class T>
byte CircularArray<T>::size()
{
    return idx;
}

template <class T>
void CircularArray<T>::clear()
{
    for (int i = 0; i < capacity; ++i)
    {
        array[i] = NULL;
    }
}

#endif
