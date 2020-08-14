#include "LineReader.h"

        
LineReader::LineReader(Stream &inputStream) : inputStream(inputStream) {}

String LineReader::readLine() {
    String line;
    while (this->hasMore()) {
        char currentChar = this->inputStream.read();
        char nextChar = this->inputStream.peek();

        // handle EOL characters
        if (currentChar == '\r' && nextChar == '\n') {
            continue; // next iteration is end of line
        } else if (currentChar == '\n') {
            break; // end of line found
        }

        line.concat(currentChar);
    }

    return line;
}

bool LineReader::hasMore() {
    return this->inputStream.available() > 0;
}


StringStream::StringStream(String &string) : string(string), position(0) {}

int StringStream::available() {
    return this->string.length() - this->position;
}

int StringStream::read() {
    if (this->position < this->string.length()) {
        return this->string[this->position++];
    }
    return -1;
}

int StringStream::peek() {
    if (this->position < this->string.length()) {
        return this->string[this->position];
    }
    return -1;
}

void StringStream::flush() {}

size_t StringStream::write(uint8_t c) {
    this->string += c;
    return 1;
}
