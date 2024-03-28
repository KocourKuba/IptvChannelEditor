#pragma once
#include <rapidxml/rapidxml.hpp>
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
	return alloc.allocate_node(node_type::node_element, name, value ? alloc.allocate_string(value) : nullptr);
}

/// <summary>
/// return string value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>string</returns>
inline std::string get_value_string(const xml_node<>* node)
{
	if (node && node->value())
		return {node->value(), node->value_size()};
	return {};
}

/// <summary>
/// return wstring value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>wstring</returns>
inline std::wstring get_value_wstring(const xml_node<>* node)
{
	if (node && node->value())
		return utils::utf8_to_utf16(node->value(), node->value_size());
	return {};
}

/// <summary>
/// return wstring value from xml node attributes
/// </summary>
/// <param name="attr"></param>
/// <returns>string</returns>
inline std::string get_value_string(const xml_attribute<>* attr)
{
	if (attr && attr->value())
		return {attr->value(), attr->value_size()};
	return {};
}

/// <summary>
/// return wstring value from xml node attributes
/// </summary>
/// <param name="attr"></param>
/// <returns>wstring</returns>
inline std::wstring get_value_wstring(const xml_attribute<>* attr)
{
	if (attr && attr->value())
		return utils::utf8_to_utf16(attr->value(), attr->value_size());
	return {};
}

/// <summary>
/// return int value from xml node
/// </summary>
/// <param name="node"></param>
/// <returns>int value</returns>
inline int get_value_int(const xml_node<>* node)
{
	if (node && node->value())
		return utils::char_to_int<char>(get_value_string(node));
	return 0;
}

/// <summary>
/// return int value from xml node attributes
/// </summary>
/// <param name="attr"></param>
/// <returns>int value</returns>
inline int get_value_int(const xml_attribute<>* attr)
{
	if (attr && attr->value())
		return utils::char_to_int<char>(get_value_string(attr));
	return 0;
}

}
