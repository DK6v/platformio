#include <memory>
#include <algorithm>

#include "Byte.h"

#include "BitBuffer.h"

namespace app {

// Class BitBuffer

BitBuffer::BitBuffer(std::function<char(void)> read,
                     std::function<void(char)> write)
:   mReadCallback(read),
    mWriteCallback(write),
    mBuffer() {}

const char BitBuffer::read(unsigned short index) {
    return mBuffer[index];
}

void BitBuffer::write(unsigned short index, const char value) {
    mBuffer[index] = value;
}

unsigned short BitBuffer::size() {
    return mBuffer.size();
}

BitBuffer::iterator BitBuffer::begin() {

    return BitBuffer::iterator(this, 0);
}

BitBuffer::iterator BitBuffer::end() {

    return BitBuffer::iterator(this, size());
}

BitBuffer::iterator BitBuffer::read(
    const iterator& it,
    char* value_p,
    unsigned short length
) {

    iterator retIt = it;

    for (unsigned short ix = 0; ix < length; ++ix) {

        value_p[ix] = BYTE_SET(ix, 0x00, *retIt++);
    }

    return retIt;
}

BitBuffer::iterator BitBuffer::write(
    const iterator& it,
    const char* value_p,
    unsigned short length
) {

    iterator retIt = it;

    for (unsigned short ix = 0; ix < length; ++ix) {

        *retIt++ = value_p[ix];
    }

    return retIt;
}

// Class BitBuffer::reference

BitBuffer::reference::reference(iterator& it): mIter(it) {}

BitBuffer::iterator& BitBuffer::reference::operator=(const char value) {

    if (mIter != mIter.mBuffer_p->end()) {
        mIter.mBuffer_p->write(mIter.mCursor, value);
    }

    return mIter;
}

BitBuffer::reference::operator const char () {

    if (mIter != mIter.mBuffer_p->end()) {
        return mIter.mBuffer_p->read(mIter.mCursor);
    }

    return (-1);
}

// Class BitBuffer::iterator

BitBuffer::iterator::iterator(BitBuffer* buffer_p)
    : mBuffer_p(buffer_p),
      mCursor(0) {}

BitBuffer::iterator::iterator(BitBuffer* buffer_p, unsigned short cursor)
    : mBuffer_p(buffer_p),
      mCursor(cursor) {}

BitBuffer::iterator BitBuffer::iterator::operator++(int) {

    return BitBuffer::iterator(mBuffer_p, mCursor++);
}

BitBuffer::iterator& BitBuffer::iterator::operator++() {

    mCursor += (*this != mBuffer_p->end()) ? 1 : 0;
    return *this;
}

BitBuffer::iterator& BitBuffer::iterator::operator+=(const short distance) {

    auto end = mBuffer_p->end();
    mCursor = std::min<short>(mCursor + distance, end.mCursor);

    return *this;
}

bool BitBuffer::iterator::operator==(BitBuffer::iterator other) {

    return this->mCursor == other.mCursor;
}

bool BitBuffer::iterator::operator!=(BitBuffer::iterator other) {

    return !(*this == other);
}

BitBuffer::reference BitBuffer::iterator::operator*() {

    return BitBuffer::reference(*this);
}

bool BitBuffer::iterator::isValid() {

    return (*this != mBuffer_p->end());
}

} // namespace