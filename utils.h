#pragma once
#include <string>
#include "rapidxml.hpp"

namespace utils
{

/// <summary>
/// Converts char string to int.
/// </summary>
/// <param name="str">null-terminated char string</param>
/// <returns>int value if success, 0 otherwise</returns>
int char_to_int(const char* str);

/// <summary>
/// Converts int to char string.
/// </summary>
/// <param name="value">int value</param>
/// <returns>string representaion of value, empty string otherwise</returns>
std::string int_to_char(int value);

/// <summary>
/// Allocate node using memory_pool allocator
/// </summary>
/// <param name="alloc">Allocator</param>
/// <param name="name">Node name</param>
/// <param name="value">Node value</param>
/// <returns>Allocated node</returns>
rapidxml::xml_node<>* alloc_node(rapidxml::memory_pool<>& alloc, const char* name, const char* value = nullptr);

/// <summary>
/// return int value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>int value</returns>
int get_value_int(rapidxml::xml_node<>* node);

/// <summary>
/// return string value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>string</returns>
std::string get_value_string(rapidxml::xml_node<>* node);

/// <summary>
/// return wstring value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>wstring</returns>
std::wstring get_value_wstring(rapidxml::xml_node<>* node);

/// <summary>
/// Converts a UTF-16 string to a UTF-8 string.
/// </summary>
/// <param name="w">A two byte character UTF-16 string.</param>
/// <returns>A single byte character UTF-8 string.</returns>
std::string utf16_to_utf8(const std::wstring& w);

/// <summary>
/// Converts a UTF-8 string to a UTF-16
/// </summary>
/// <param name="s">A single byte character UTF-8 string.</param>
/// <returns>A two byte character UTF-16 string.</returns>
std::wstring utf8_to_utf16(const std::string& s);
}
