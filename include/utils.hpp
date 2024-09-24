#pragma once

#include "errors.hpp"

template<typename T>
class Vector {
private:
    T *data;
    size_t size;
    size_t capacity;

public:
    inline size_t size(void) const { return size; }
    inline size_t capacity(void) const { return capacity; }

    inline T& operator[](size_t index)
    {
        if (index <= size)
            return data[index];
        else
            error<FAIL, ULONG>("Vector access our of bounds:", index);
    }
};