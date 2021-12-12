#include "MessageWriter.h"

#define max(a, b) ((a > b) ? (a) : (b))

MessageWriter::MessageWriter(uint8_t *buffer, unsigned int bufferSize) {
    Buffer = buffer;
    BufferSize = bufferSize;

    Cursor = 0;
    MessageStack = std::vector<size_t>(4);
}

MessageWriter::MessageWriter(unsigned int bufferSize) {
    Buffer = (uint8_t*)calloc(bufferSize, 1);
    BufferSize = bufferSize;
    Cursor = 0;
    MessageStack = std::vector<size_t>(4);
}

void MessageWriter::Release() {
    free(Buffer);
    Buffer = 0;
    Cursor = 0;
}

void MessageWriter::Realloc(size_t bufferSize) {
    Buffer = (uint8_t*)realloc(Buffer, bufferSize);
    BufferSize = bufferSize;
}

void MessageWriter::Expand(size_t extraSize) {
    if (BytesRemaining() < extraSize) {
        Realloc(BufferSize + extraSize);
    }
}

void MessageWriter::ShrinkToSize() {
    Realloc(Cursor);
}

void MessageWriter::Jump(size_t bytes) {
    Expand(bytes);
    Cursor += bytes;
}

size_t MessageWriter::BytesRemaining() {
    return max(0, BufferSize - Cursor);
}

void MessageWriter::WriteUInt8(uint8_t val) {
    Expand(1);
    Buffer[Cursor] = val;
    Cursor++;
}

void MessageWriter::WriteInt8(int8_t val) {
    WriteUInt8((int8_t)val);
}

void MessageWriter::WriteBool(bool val) {
    WriteUInt8(val == true ? 1 : 0);
}

void MessageWriter::WriteUInt16(uint16_t val, bool bigEndian) {
    Expand(2);
    if constexpr (std::endian::native == std::endian::big) {
        if (bigEndian) {
            *(uint16_t*)&Buffer[Cursor] = val;
        } else {
            *(uint16_t*)&Buffer[Cursor] = ((val & 0xff) << 8) | ((val & 0xff00) >> 8);
        }
    } else {
        if (bigEndian) {
            *(uint16_t*)&Buffer[Cursor] = ((val & 0xff) << 8) | ((val & 0xff00) >> 8);
        } else {
            *(uint16_t*)&Buffer[Cursor] = val;
        }
    }
    Cursor += 2;
}

void MessageWriter::WriteInt16(int16_t val, bool bigEndian) {
    WriteUInt16((uint16_t)val, bigEndian);
}

void MessageWriter::WriteUInt32(uint32_t val, bool bigEndian) {
    Expand(4);
    if constexpr (std::endian::native == std::endian::big) {
        if (bigEndian) {
            *(uint32_t*)&Buffer[Cursor] = val;
        } else {
            *(uint32_t*)&Buffer[Cursor] = ((val & 0xff) << 24)
                                          | ((val & 0xff00) << 8)
                                          | ((val & 0xff0000) >> 8)
                                          | ((val & 0xff000000) >> 24);
        }
    } else {
        if (bigEndian) {
            *(uint32_t*)&Buffer[Cursor] = ((val & 0xff) << 24)
                                          | ((val & 0xff00) << 8)
                                          | ((val & 0xff0000) >> 8)
                                          | ((val & 0xff000000) >> 24);
        } else {
            *(uint32_t*)&Buffer[Cursor] = val;
        }
    }
    Cursor += 4;
}

void MessageWriter::WriteInt32(int32_t val, bool bigEndian) {
    WriteUInt32((uint32_t)val, bigEndian);
}

void MessageWriter::WriteUInt64(uint64_t val, bool bigEndian) {
    Expand(8);
    if constexpr (std::endian::native == std::endian::big) {
        if (bigEndian) {
            *(uint64_t*)&Buffer[Cursor] = val;
        } else {
            *(uint64_t*)&Buffer[Cursor] = ((val & 0xff) << 56)
                                          | ((val & 0xff00) << 40)
                                          | ((val & 0xff0000) << 24)
                                          | ((val & 0xff000000) << 8)
                                          | ((val & 0xff00000000ULL) >> 8)
                                          | ((val & 0xff0000000000ULL) >> 24)
                                          | ((val & 0xff000000000000ULL) >> 40)
                                          | ((val & 0xff00000000000000ULL) >> 56);
        }
    } else {
        if (bigEndian) {
            *(uint64_t*)&Buffer[Cursor] = ((val & 0xff) << 56)
                                          | ((val & 0xff00) << 40)
                                          | ((val & 0xff0000) << 24)
                                          | ((val & 0xff000000) << 8)
                                          | ((val & 0xff00000000ULL) >> 8)
                                          | ((val & 0xff0000000000ULL) >> 24)
                                          | ((val & 0xff000000000000ULL) >> 40)
                                          | ((val & 0xff00000000000000ULL) >> 56);
        } else {
            *(uint64_t*)&Buffer[Cursor] = val;
        }
    }
    Cursor += 8;
}

void MessageWriter::WriteInt64(int64_t val, bool bigEndian) {
    WriteUInt64((uint64_t)val, bigEndian);
}

void MessageWriter::WriteSingle(float val, bool bigEndian) {
    WriteUInt32(*(uint32_t*)&val, bigEndian);
}

void MessageWriter::WriteDouble(double val, bool bigEndian) {
    WriteUInt64(*(uint64_t*)&val, bigEndian);
}

void MessageWriter::WritePackedUInt32(uint32_t val) {
    do
    {
        uint8_t b = (uint8_t)(val & 0xFF);
        if (val >= 0x80)
        {
            b |= 0x80;
        }

        WriteUInt8(b);
        val >>= 7;
    } while (val > 0);
}

void MessageWriter::WritePackedInt32(int32_t val) {
    WritePackedUInt32((uint32_t)val);
}

void MessageWriter::WriteString(const char *val) {
    size_t len = strlen(val);
    WritePackedUInt32(len);
    for (size_t i = 0; i < len; i++) {
        WriteUInt8(val[i]);
    }
}

void MessageWriter::WriteBytes(uint8_t *bytes, size_t byteLength) {
    for (size_t i = 0; i < byteLength; i++) {
        WriteUInt8(bytes[i]);
    }
}

void MessageWriter::BeginMessage(uint8_t messageTag) {
    MessageStack.push_back(Cursor);
    Jump(2);
    WriteUInt8(messageTag);
}

void MessageWriter::EndMessage() {
    size_t lastCursor = Cursor;
    Cursor = MessageStack.back();
    WriteUInt16(lastCursor - Cursor - 3);
    Cursor = lastCursor;
    MessageStack.pop_back();
}