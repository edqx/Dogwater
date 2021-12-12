#include <cstdint>
#include <bit>
#include <cassert>
#include <string.h>
#include <vector>

#ifndef DOGWATER_BINARYWRITER_H
#define DOGWATER_BINARYWRITER_H

class MessageWriter {
public:
    uint8_t* Buffer;
    size_t BufferSize;

    size_t Cursor;
    std::vector<size_t> MessageStack;

    MessageWriter(uint8_t* buffer, unsigned int bufferSize);
    MessageWriter(unsigned int bufferSize);

    void Release();
    void Realloc(size_t bufferSize);
    void Expand(size_t extraSize);
    void ShrinkToSize();

    void Jump(size_t bytes);
    size_t BytesRemaining();

    void WriteUInt8(uint8_t val);
    void WriteInt8(int8_t val);

    void WriteBool(bool val);

    void WriteUInt16(uint16_t val, bool bigEndian = false);
    void WriteInt16(int16_t val, bool bigEndian = false);

    void WriteUInt32(uint32_t val, bool bigEndian = false);
    void WriteInt32(int32_t val, bool bigEndian = false);

    void WriteUInt64(uint64_t val, bool bigEndian = false);
    void WriteInt64(int64_t val, bool bigEndian = false);

    void WriteSingle(float val, bool bigEndian = false);
    void WriteDouble(double val, bool bigEndian = false);

    void WritePackedUInt32(uint32_t val);
    void WritePackedInt32(int32_t val);

    void WriteString(const char* val);
    void WriteBytes(uint8_t* bytes, size_t stringLength);

    void BeginMessage(uint8_t messageTag);
    void EndMessage();
};

#endif //DOGWATER_BINARYWRITER_H
