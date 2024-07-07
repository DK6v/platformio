#pragma once

#include <memory>
#include <iterator>

#include "common/Byte.h"

namespace app
{

class ByteBuffer
{
public:
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

public:
    virtual const char read(unsigned short index) = 0;
    virtual void write(unsigned short index, const char value) = 0;

    virtual void commit() = 0;
    virtual unsigned short size() = 0;
};

class ByteBuffer::iterator
    : public std::iterator<std::input_iterator_tag,
                           char,
                           char,
                           const char*,
                           ByteBuffer::reference> {
    friend reference;

public:
    iterator(ByteBuffer* buffer_p);
    iterator(ByteBuffer* buffer_p, unsigned short cursor);

    iterator operator++(int); /* postfix */
    iterator &operator++();   /* prefix */
    iterator& operator+=(const short distance);
    bool operator==(iterator other);
    bool operator!=(iterator other);
    reference operator*();

    bool isValid();

private:
    ByteBuffer* mBuffer_p;
    unsigned short mCursor;
};

class ByteBuffer::reference {
public:
    reference(iterator& it);

    iterator& operator=(const char value);

    operator const char ();

private:
    iterator& mIter;
};

template <typename T>
ByteBuffer::iterator ByteBuffer::read(const iterator& it, T* value_p) {

    return read(it, reinterpret_cast<char*>(value_p), sizeof(T));
}

template <typename T>
ByteBuffer::iterator ByteBuffer::write(const iterator& it, const T value) {

    return write(it, reinterpret_cast<const char*>(&value), sizeof(T));
}


} // namespace