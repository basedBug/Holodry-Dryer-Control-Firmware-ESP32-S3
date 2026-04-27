#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

namespace JsonHandlers
{
    void printJsonContents(const JsonDocument &doc);
    void printJsonObjectContents(JsonObjectConst obj, int indentLevel);
    void printJsonArrayContents(const JsonArrayConst &arr, int indentLevel);
    void handleJsonValue(JsonVariantConst value, int indentLevel);
    void printIndent(int level);
};