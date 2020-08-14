/**
 * Config file parser.
 */
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <Stream.h>


// internal linked-list structure used to associate keys and values
struct _ConfigPair {
    String key;
    String value;
    _ConfigPair *next;
};

/**
 */
class ConfigParser {
    public:
        /**
         * Create an empty parser.
         */
        ConfigParser();
        ~ConfigParser();

        /**
         * Add all key/value entries contained in the input stream.
         */
        void parse(Stream &inputStream);

        /**
         * Retrieve the value for a key or default value if key not found.
         */
        const char *get(const char *key, const char *defaultValue);

    private:
        _ConfigPair *firstPair;
        _ConfigPair *lastPair;
        void put(String key, String value);
};


#endif
