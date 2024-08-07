#pragma once

#include <memory>
#include <iterator>
#include <functional>

#include "Byte.h"

namespace app
{

class BitBuffer
{
public:
    BitBuffer(std::function<char(void)> read_cb,
              std::function<void(char)> write_cb);

    class iterator;
    class reference;

    iterator begin();
    iterator end();

    iterator read(const iterator& it, char* value_p, unsigned short length);
    iterator write(const iterator& it, const char* value_p, unsigned short length);

    template <typename T>
    iterator write(const iterator& it, const T value);

    template <typename T>
    iterator read(const iterator& it, T* value_p);

    const char read(unsigned short index);
    void write(unsigned short index, const char value);
    unsigned short size();

private:
    std::function<char(void)> mReadCallback;
    std::function<void(char)> mWriteCallback;

    std::vector<char> mBuffer;
};

class BitBuffer::iterator
    : public std::iterator<std::input_iterator_tag,
                           char,
                           char,
                           const char*,
                           BitBuffer::reference> {
    friend reference;

public:
    iterator(BitBuffer* buffer_p);
    iterator(BitBuffer* buffer_p, unsigned short cursor);

    iterator operator++(int); /* postfix */
    iterator &operator++();   /* prefix */
    iterator& operator+=(const short distance);
    bool operator==(iterator other);
    bool operator!=(iterator other);
    reference operator*();

    bool isValid();

private:
    BitBuffer* mBuffer_p;
    unsigned short mCursor;
};

class BitBuffer::reference {
public:
    reference(iterator& it);

    iterator& operator=(const char value);

    operator const char ();

private:
    iterator& mIter;
};

template <typename T>
BitBuffer::iterator BitBuffer::read(const iterator& it, T* value_p) {

    return read(it, reinterpret_cast<char*>(value_p), sizeof(T));
}

template <typename T>
BitBuffer::iterator BitBuffer::write(const iterator& it, const T value) {

    return write(it, reinterpret_cast<const char*>(&value), sizeof(T));
}


} // namespace