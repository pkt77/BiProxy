template<class lengthType>
inline std::string ByteBuffer::readString(lengthType(ByteBuffer::* lengthMethod)()) {
    int length = (this->*lengthMethod)();
    std::string string(buffer + offset, length);

    offset += length;
    return string;
}

template<class lengthType>
inline void ByteBuffer::writeString(const std::string& value, void(ByteBuffer::* lengthMethod)(lengthType)) {
    lengthType length = value.length();

    ensureWritable(sizeof(lengthType) + length);
    (this->*lengthMethod)(length);
    memcpy(buffer + offset, value.data(), length);

    offset += length;
    size += length;
}

template<class lengthType>
inline void ByteBuffer::writeString(const char* value, void(ByteBuffer::* lengthMethod)(lengthType)) {
    size_t length = strlen(value);

    ensureWritable(sizeof(lengthType) + length);
    (this->*lengthMethod)(length);
    memcpy(buffer + offset, value, length);

    offset += length;
    size += length;
}