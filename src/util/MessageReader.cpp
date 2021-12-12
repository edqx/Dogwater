#include "MessageReader.h"

#define max(a, b) ((a > b) ? (a) : (b))

MessageReader::MessageReader(uint8_t* buffer, unsigned int bufferSize, uint8_t messageTag) {
    Buffer = buffer;
    BufferSize = bufferSize;

    Cursor = 0;
    MessageTag = messageTag;
}

void MessageReader::Release() {
    free(Buffer);
    Buffer = 0;
    BufferSize = 0;
    Cursor = 0;
    MessageTag = 0;
}

void MessageReader::Jump(size_t bytes) {
    assert(BytesRemaining() >= bytes);
    Cursor += bytes;
}

size_t MessageReader::BytesRemaining() {
    return max(0, BufferSize - Cursor);
}

uint8_t MessageReader::ReadUInt8() {
    assert(Buffer != nullptr);
    assert(BytesRemaining() >= 1);

    uint8_t val = Buffer[Cursor];
    Cursor++;
    return val;
}

int8_t MessageReader::ReadInt8() {
    return (int8_t)ReadUInt8();
}

bool MessageReader::ReadBool() {
    return ReadUInt8() == 1;
}

uint16_t MessageReader::ReadUInt16(bool bigEndian) {
    assert(Buffer != nullptr);
    assert(BytesRemaining() >= 2);

    uint16_t val = *(uint16_t*)(&Buffer[Cursor]);
    Cursor += 2;
    if constexpr (std::endian::native == std::endian::big) {
        if (bigEndian) {
            return val;
        } else {
            return _byteswap_ushort(val);
        }
    } else {
        if (bigEndian) {
            return _byteswap_ushort(val);
        } else {
            return val;
        }
    }
}

int16_t MessageReader::ReadInt16(bool bigEndian) {
    return (int16_t) ReadUInt16(bigEndian);
}

uint32_t MessageReader::ReadUInt32(bool bigEndian) {
    assert(Buffer != nullptr);
    assert(BytesRemaining() >= 4);

    uint32_t val = *(uint32_t*)(&Buffer[Cursor]);
    Cursor += 4;
    if constexpr (std::endian::native == std::endian::big) {
        if (bigEndian) {
            return val;
        } else {
            return _byteswap_ulong(val);
        }
    } else {
        if (bigEndian) {
            return _byteswap_ulong(val);
        } else {
            return val;
        }
    }
}

int32_t MessageReader::ReadInt32(bool bigEndian) {
    return (int32_t)ReadUInt32(bigEndian);
}

uint64_t MessageReader::ReadUInt64(bool bigEndian) {
    assert(Buffer != nullptr);
    assert(BytesRemaining() >= 8);

    uint64_t val = *(uint64_t*)(&Buffer[Cursor]);
    this->Cursor += 8;
    if constexpr (std::endian::native == std::endian::big) {
        if (bigEndian) {
            return val;
        } else {
            return _byteswap_uint64(val);
        }
    } else {
        if (bigEndian) {
            return _byteswap_uint64(val);
        } else {
            return val;
        }
    }
}

int64_t MessageReader::ReadInt64(bool bigEndian) {
    return (int64_t)ReadUInt64(bigEndian);
}

float MessageReader::ReadSingle(bool bigEndian) {
    uint32_t val = ReadUInt32(bigEndian);
    return *(float*)&val;
}

double MessageReader::ReadDouble(bool bigEndian) {
    uint64_t val = ReadUInt64(bigEndian);
    return *(double*)&val;
}

uint32_t MessageReader::ReadPackedUInt32() {
    bool readMore = true;
    int shift = 0;
    uint32_t output = 0;

    while (readMore)
    {
        uint8_t b = ReadUInt8();
        if (b >= 0x80)
        {
            readMore = true;
            b ^= 0x80;
        }
        else
        {
            readMore = false;
        }

        output |= (uint32_t)(b << shift);
        shift += 7;
    }

    return output;
}

int32_t MessageReader::ReadPackedInt32() {
    return (int32_t)MessageReader::ReadPackedUInt32();
}

const char* MessageReader::ReadString() {
    uint32_t stringLength = ReadPackedUInt32();
    assert(BytesRemaining() >= stringLength);

    const char* str = (const char*)malloc(stringLength + 1);
    memcpy((void *) str, &Buffer[Cursor], stringLength);
    *(char*)&str[stringLength] = '\0';
    Cursor += stringLength;
    return str;
}

MessageReader MessageReader::ReadSlice(size_t bytes) {
    assert(BytesRemaining() >= bytes);
    return MessageReader(&Buffer[Cursor], bytes);
}

MessageReader MessageReader::ReadMessage() {
    uint16_t messageLength = ReadUInt16();
    uint8_t messageTag = ReadUInt8();

    assert(BytesRemaining() >= messageLength);

    MessageReader reader(&Buffer[Cursor], messageLength, messageTag);

    Cursor += messageLength;

    return reader;
}