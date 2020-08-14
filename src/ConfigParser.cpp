#include "ConfigParser.h"
#include "LineReader.h"


ConfigParser::ConfigParser() : firstPair(NULL), lastPair(NULL) {}

ConfigParser::~ConfigParser() {
    // clean up linked list
    _ConfigPair *currentPair = this->firstPair;
    while (currentPair != NULL) {
        _ConfigPair *nextPair = currentPair->next;
        delete currentPair;
        currentPair = nextPair;
    }
}

void ConfigParser::parse(Stream &inputStream) {
    // read until there are no more characters available
    LineReader lineReader(inputStream);
    while (lineReader.hasMore()) {
        String line = lineReader.readLine();
        line.trim();

        // skip empty lines and comment lines
        if (line.length() == 0 || line.startsWith("#")) {
            continue;
        }

        // skip lines that don't have the delimiter
        int splitPos = line.indexOf('=');
        if (splitPos == -1) {
            continue;
        }

        // extract key and value
        _ConfigPair *linePair = new _ConfigPair;
        linePair->key = line.substring(0, splitPos);
        linePair->value = line.substring(splitPos + 1);
        linePair->key.trim();
        linePair->value.trim();
        linePair->next = NULL;

        // insert as first pair
        if (this->firstPair == NULL) {
            this->firstPair = linePair;
            this->lastPair = linePair;
    
        // insert after last pair
        } else {
            this->lastPair->next = linePair;
            this->lastPair = linePair;
        }
    }
}

const char *ConfigParser::get(const char *key, const char *defaultValue) {
    _ConfigPair *currentPair = this->firstPair;
    while (currentPair != NULL) {
        // return value for matching key
        if (currentPair->key.equals(key)) {
            return currentPair->value.c_str();
        }

        // advance to next pair in the linked list
        currentPair = currentPair->next;
    }

    // use default if key isn't found
    return defaultValue;
}
