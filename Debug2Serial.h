/*
 * Debug2Serial.h
*/

#ifndef DEBUG2SERIAL_H
#define DEBUG2SERIAL_H

#ifdef DEBUG
    #define DEBUG_SERIAL_INIT(baud) \
        Serial.begin(baud); \
        Serial.println("DEBUG ENABLED")
    #define DEBUG_SERIAL_FLUSH() Serial.flush()    
    #ifdef DEBUG_VERBOSE
        #define DEBUG_SERIAL_PRINT(...)  \
            Serial.print(millis());     \
            Serial.print(": ");    \
            Serial.print(__PRETTY_FUNCTION__); \
            Serial.print(' ');      \
            Serial.print(__LINE__);     \
            Serial.print(' ');      \
            Serial.print(__VA_ARGS__)
        #define DEBUG_SERIAL_PRINTLN(...)  \
            Serial.print(millis());     \
            Serial.print(": ");    \
            Serial.print(__PRETTY_FUNCTION__); \
            Serial.print(' ');      \
            Serial.print(__LINE__);     \
            Serial.print(' ');      \
            Serial.println(__VA_ARGS__)
    #define DEBUG_SERIAL_PRINTF(...)  \
            Serial.print(millis());     \
            Serial.print(": ");    \
            Serial.print(__PRETTY_FUNCTION__); \
            Serial.print(' ');      \
            Serial.print(__LINE__);     \
            Serial.print(' ');      \
            Serial.printf(__VA_ARGS__)
    #else
        #define DEBUG_SERIAL_INIT(baud) Serial.begin(baud)
        #define DEBUG_SERIAL_PRINT(...) Serial.print(__VA_ARGS__)
        #define DEBUG_SERIAL_PRINTLN(...) Serial.println(__VA_ARGS__)
        #define DEBUG_SERIAL_PRINTF(...) Serial.printf(__VA_ARGS__)
    #endif
#else
    #define DEBUG_SERIAL_INIT(baud)
    #define DEBUG_SERIAL_FLUSH()
    #define DEBUG_SERIAL_PRINT(...)
    #define DEBUG_SERIAL_PRINTLN(...)
    #define DEBUG_SERIAL_PRINTF(...)
#endif

#endif
