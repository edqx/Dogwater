#include <cstdint>
#include <bit>
#include <cassert>
#include <memory.h>

#ifndef DOGWATER_BINARYREADER_H
#define DOGWATER_BINARYREADER_H

class MessageReader {
public:
    uint8_t* Buffer;
    size_t BufferSize;

    size_t Cursor;
    uint8_t MessageTag;

    MessageReader(uint8_t* buffer, unsigned int bufferSize, uint8_t messageTag = 0);

    void Release();

    void Jump(size_t bytes);
    size_t BytesRemaining();

    uint8_t ReadUInt8();
    int8_t ReadInt8();

    bool ReadBool();

    uint16_t ReadUInt16(bool bigEndian = false);
    int16_t ReadInt16(bool bigEndian = false);

    uint32_t ReadUInt32(bool bigEndian = false);
    int32_t ReadInt32(bool bigEndian = false);

    uint64_t ReadUInt64(bool bigEndian = false);
    int64_t ReadInt64(bool bigEndian = false);

    float ReadSingle(bool bigEndian = false);
    double ReadDouble(bool bigEndian = false);

    uint32_t ReadPackedUInt32();
    int32_t ReadPackedInt32();

    const char* ReadString();
    MessageReader ReadSlice(size_t bytes);

    MessageReader ReadMessage();
};

#endif //DOGWATER_BINARYREADER_H
