#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Preferences.h>
#include <Arduino.h>

class ConfigManager {
private:
    Preferences preferences;

public:
    ConfigManager();
    ~ConfigManager();

    void begin(const char* namespaceName);
    void end();

    void setInt(const char* key, int value);
    int getInt(const char* key, int defaultValue = 0);

    void setBool(const char* key, bool value);
    bool getBool(const char* key, bool defaultValue = false);

    void setString(const char* key, const String& value);
    String getString(const char* key, const String& defaultValue = "");

    void setBytes(const char* key, const void* data, size_t len);
    size_t getBytes(const char* key, void* data, size_t maxLen);

    void removeKey(const char* key);
    void clear();
};

#endif