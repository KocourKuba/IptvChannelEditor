#pragma once
#include "rapidxml.hpp"
#include <stdexcept>
#include <string>
#include "utils.h"

namespace rapidxml
{

/// <summary>
/// Allocate node using memory_pool allocator
/// </summary>
/// <param name="alloc">Allocator</param>
/// <param name="name">Node name</param>
/// <param name="value">Node value</param>
/// <returns>Allocated node</returns>
inline xml_node<>* alloc_node(memory_pool<>& alloc, const char* name, const char* value = nullptr)
{
	return alloc.allocate_node(node_element, name, value ? alloc.allocate_string(value) : nullptr);
}

/// <summary>
/// return int value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>int value</returns>
inline int get_value_int(xml_node<>* node)
{
	if (node && node->value())
		return utils::char_to_int(node->value());
	return 0;
}

/// <summary>
/// return string value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>string</returns>
inline std::string get_value_string(xml_node<>* node)
{
	if (node && node->value())
		return node->value();
	return std::string();
}

/// <summary>
/// return wstring value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>wstring</returns>
inline std::wstring get_value_wstring(xml_node<>* node)
{
	if (node && node->value())
		return utils::utf8_to_utf16(node->value());
	return std::wstring();
}

}
