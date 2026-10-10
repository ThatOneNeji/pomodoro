/**
 * @file WString.h
 * @brief Host-test stand-in for Arduino's String, backed by std::string.
 */
#pragma once

#include <string>

class String {
   private:
    std::string value;

   public:
    String(const char *text = "") : value(text ? text : "") {}
    String(const std::string &text) : value(text) {}
    String(int number) : value(std::to_string(number)) {}
    String(unsigned int number) : value(std::to_string(number)) {}
    String(long number) : value(std::to_string(number)) {}
    String(unsigned long number) : value(std::to_string(number)) {}

    const char *c_str() const { return value.c_str(); }
    unsigned int length() const { return value.size(); }
    bool isEmpty() const { return value.empty(); }

    String &operator+=(const String &other) {
        value += other.value;
        return *this;
    }
    friend String operator+(String left, const String &right) { return left += right; }
    bool operator==(const String &other) const { return value == other.value; }
    bool operator!=(const String &other) const { return value != other.value; }
};
