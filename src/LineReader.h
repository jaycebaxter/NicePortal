/**
 * Utility to read a stream line-by-line.
 */
#ifndef LINE_READER_H
#define LINE_READER_H

#include <Stream.h>

#include <cstdint>


/**
 */
class LineReader {
    public:
        LineReader(Stream &inputStream);
        String readLine();
        bool hasMore();

    private:
        Stream &inputStream;
};

/**
 * 
 */
class StringStream : public Stream {
    public:
        StringStream(String &string);
        virtual int available() override;
        virtual int read() override;
        virtual int peek() override;
        virtual void flush() override;
        virtual size_t write(uint8_t c) override;

    private:
        String &string;
        size_t position;
};

#endif
