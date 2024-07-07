#include <memory>
#include <algorithm>

#include "Console.h"
#include "common/Byte.h"

#include "ByteBuffer.h"

namespace app {

// Class ByteBuffer

ByteBuffer::iterator ByteBuffer::begin() {

    return ByteBuffer::iterator(this, 0);
}

ByteBuffer::iterator ByteBuffer::end() {

    return ByteBuffer::iterator(this, size());
}

ByteBuffer::iterator ByteBuffer::read(
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

ByteBuffer::iterator ByteBuffer::write(
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

// Class ByteBuffer::reference

ByteBuffer::reference::reference(iterator& it): mIter(it) {}

ByteBuffer::iterator& ByteBuffer::reference::operator=(const char value) {

    if (mIter != mIter.mBuffer_p->end()) {
        mIter.mBuffer_p->write(mIter.mCursor, value);
    }

    return mIter;
}

ByteBuffer::reference::operator const char () {

    if (mIter != mIter.mBuffer_p->end()) {
        return mIter.mBuffer_p->read(mIter.mCursor);
    }

    return (-1);
}

// Class ByteBuffer::iterator

ByteBuffer::iterator::iterator(ByteBuffer* buffer_p)
    : mBuffer_p(buffer_p),
      mCursor(0) {}

ByteBuffer::iterator::iterator(ByteBuffer* buffer_p, unsigned short cursor)
    : mBuffer_p(buffer_p),
      mCursor(cursor) {}

ByteBuffer::iterator ByteBuffer::iterator::operator++(int) {

    return ByteBuffer::iterator(mBuffer_p, mCursor++);
}

ByteBuffer::iterator& ByteBuffer::iterator::operator++() {

    mCursor += (*this != mBuffer_p->end()) ? 1 : 0;
    return *this;
}

ByteBuffer::iterator& ByteBuffer::iterator::operator+=(const short distance) {

    auto end = mBuffer_p->end();
    mCursor = std::min<short>(mCursor + distance, end.mCursor);

    return *this;
}

bool ByteBuffer::iterator::operator==(ByteBuffer::iterator other) {

    return this->mCursor == other.mCursor;
}

bool ByteBuffer::iterator::operator!=(ByteBuffer::iterator other) {

    return !(*this == other);
}

ByteBuffer::reference ByteBuffer::iterator::operator*() {

    return ByteBuffer::reference(*this);
}

bool ByteBuffer::iterator::isValid() {

    return (*this != mBuffer_p->end());
}

} // namespace