#pragma once

#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace utils
{
	/// <summary>
	/// A vectormap combines a map with its fast key->value lookups
	/// with a vector to preserve the order that key-value pairs were added
	/// </summary>
	/// <typeparam name="K">Key type of the map</typeparam>
	/// <typeparam name="V">Value type of the map</typeparam>
	template <typename K, typename V>
	class vectormap
	{
	public:
		/// <summary>
		/// Access the vector of pairs directly for index access or iteration
		/// </summary>
		const std::vector<std::pair<K, V>>& vec() const
		{
			return m_vec;
		}

		/// <summary>
		/// What is the value for a given key?
		/// NOTE: By returning the value by, well, value
		///       this function can stay const
		///       and users of the class can use pointers
		///       if they want side effects down the line.
		/// </summary>
		const V& get(const K& key) const
		{
			if (const auto& it = m_map.find(key); it != m_map.end())
				return m_vec[it->second].second;

			throw std::runtime_error("key not found");
		}

		/// <summary>
		/// What is the value at a given index?
		/// NOTE: By returning the value by, well, value
		///       this function can stay const
		///       and users of the class can use pointers
		///       if they want side effects down the line.
		/// </summary>
		const V& getAt(size_t index) const
		{
			if (index < m_vec.size())
				return m_vec[index].second;

			throw std::runtime_error("index out of range");
		}

		/// <summary>
		/// How many key-value pairs are in this?
		/// </summary>
		size_t size() const
		{
			return m_vec.size();
		}

		/// <summary>
		/// is empty
		/// </summary>
		bool empty() const
		{
			return m_vec.empty();
		}

		/// <summary>
		/// Does this map contain this key?
		/// </summary>
		bool contains(const K& key) const
		{
			return m_map.find(key) != m_map.end();
		}

		/// <summary>
		/// Associate a value with a key
		/// </summary>
		void set(const K& key, const V& val)
		{
			auto it = m_map.find(key);
			if (it == m_map.end())
			{
				m_map.insert({ key, m_vec.size() });
				m_vec.push_back({ key, val });
			}
			else
			{
				m_vec[it->second].second = val;
			}
		}

		/// <summary>
		/// Get a value, checking if it exists first
		/// </summary>
		/// <param name="key">Key value to look up</param>
		/// <param name="val">Value to populate</param>
		/// <returns>true if a value exists for the key, false otherwise</returns>
		bool tryGet(const K& key, V& val) const
		{
			if (const auto& it = m_map.find(key); it != m_map.end())
			{
				val = m_vec[it->second].second;
				return true;
			}

			return false;
		}

		/// <summary>
		/// Get a list of the keys of this map
		/// </summary>
		std::vector<K> keys() const
		{
			std::vector<K> retVal;
			retVal.reserve(m_vec.size());
			for (size_t k = 0; k < m_vec.size(); ++k)
				retVal.push_back(m_vec[k].first);
			return retVal;
		}

		/// <summary>
		/// Get a list of the values of this map
		/// </summary>
		std::vector<V> values() const
		{
			std::vector<V> retVal;
			retVal.reserve(m_vec.size());
			for (size_t v = 0; v < m_vec.size(); ++v)
				retVal.push_back(m_vec[v].second);

			return retVal;
		}

	private:
		std::unordered_map<K, size_t> m_map;
		std::vector<std::pair<K, V>> m_vec;
	};
}
