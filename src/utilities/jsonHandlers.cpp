#include "utilities/jsonHandlers.h"

void JsonHandlers::printJsonContents(const JsonDocument &doc)
{
    Serial.print(F("[commsManager] "));
    Serial.println(F("Received JSON from server:"));
    printJsonObjectContents(doc.as<JsonObjectConst>(), 0);
}

void JsonHandlers::printJsonObjectContents(JsonObjectConst obj, int indentLevel)
{
    Serial.print(F("[commsManager] "));
    // Print opening brace with proper indentation
    printIndent(indentLevel);
    Serial.println(F("{"));    

    size_t count = 0;
    size_t total = obj.size();
    
    for (JsonPairConst keyvalue : obj)
    {
        const char *key = keyvalue.key().c_str();
        JsonVariantConst value = keyvalue.value();
        
        Serial.print(F("[commsManager] "));
        // Print key with proper indentation
        printIndent(indentLevel + 1);
        Serial.print('"');
        Serial.print(key);
        Serial.print(F("\": "));
        
        // Handle different value types
        handleJsonValue(value, indentLevel + 1);
        
        // Add comma if not the last element
        if (++count < total) {
            Serial.println(',');
        } else {
            Serial.println();
        }
    }
    
    Serial.print(F("[commsManager] "));
    // Print closing brace with proper indentation
    printIndent(indentLevel);
    Serial.print('}');
    
    if (indentLevel == 0) {
        Serial.println(); // Final newline for top-level object
    }
}

void JsonHandlers::printJsonArrayContents(const JsonArrayConst &arr, int indentLevel = 0)
{
    Serial.print(F("[commsManager] "));
    printIndent(indentLevel);
    Serial.println('[');
    
    size_t count = 0;
    size_t total = arr.size();
    
    for (JsonVariantConst value : arr)
    {
        Serial.print(F("[commsManager] "));
        printIndent(indentLevel + 1);
        handleJsonValue(value, indentLevel + 1);
        
        // Add comma if not the last element
        if (++count < total) {
            Serial.println(',');
        } else {
            Serial.println();
        }
    }
    
    Serial.print(F("[commsManager] "));
    printIndent(indentLevel);
    Serial.print(']');
}

void JsonHandlers::handleJsonValue(JsonVariantConst value, int indentLevel)
{
    if (value.is<const char*>()) {
        Serial.print('"');
        Serial.print(value.as<const char*>());
        Serial.print('"');
    }
    else if (value.is<int>()) {
        Serial.print(value.as<int>());
    }
    else if (value.is<unsigned int>()) {
        Serial.print(value.as<unsigned int>());
    }
    else if (value.is<long>()) {
        Serial.print(value.as<long>());
    }
    else if (value.is<unsigned long>()) {
        Serial.print(value.as<unsigned long>());
    }
    else if (value.is<bool>()) {
        Serial.print(value.as<bool>() ? F("true") : F("false"));
    }
    else if (value.is<float>()) {
        Serial.print(value.as<float>(), 6);
    }
    else if (value.is<double>()) {
        Serial.print(value.as<double>(), 6);
    }
    else if (value.is<JsonArrayConst>()) {
        JsonArrayConst nestedArr = value.as<JsonArrayConst>();
        printJsonArrayContents(nestedArr, indentLevel);
    }
    else if (value.is<JsonObjectConst>()) {
        JsonObjectConst nestedObj = value.as<JsonObjectConst>();
        printJsonObjectContents(nestedObj, indentLevel);
    }
    else if (value.isNull()) {
        Serial.print(F("null"));
    }
    else {
        Serial.print(F("(unhandled type)"));
    }
}

void JsonHandlers::printIndent(int level)
{
    // Print spaces for indentation (after [taskName] prefix)
    for (int i = 0; i < level * 2; i++) {
        Serial.print(' ');
    }
}