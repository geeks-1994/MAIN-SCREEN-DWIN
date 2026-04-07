#include "ConfigManager.h"

ConfigManager::ConfigManager() {}

ConfigManager::~ConfigManager() {
    end();
}

void ConfigManager::begin(const char* namespaceName) {
    preferences.begin(namespaceName, false);
}

void ConfigManager::end() {
    preferences.end();
}

void ConfigManager::setInt(const char* key, int value) {
    preferences.putInt(key, value);
}

int ConfigManager::getInt(const char* key, int defaultValue) {
    return preferences.getInt(key, defaultValue);
}

void ConfigManager::setBool(const char* key, bool value) {
    preferences.putBool(key, value);
}

bool ConfigManager::getBool(const char* key, bool defaultValue) {
    return preferences.getBool(key, defaultValue);
}

void ConfigManager::setString(const char* key, const String& value) {
    preferences.putString(key, value);
}

String ConfigManager::getString(const char* key, const String& defaultValue) {
    return preferences.getString(key, defaultValue);
}

void ConfigManager::setBytes(const char* key, const void* data, size_t len) {
    preferences.putBytes(key, data, len);
}

size_t ConfigManager::getBytes(const char* key, void* data, size_t maxLen) {
    return preferences.getBytes(key, data, maxLen);
}

void ConfigManager::removeKey(const char* key) {
    preferences.remove(key);
}

void ConfigManager::clear() {
    preferences.clear();
}