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
		vectormap() = default;
		vectormap(const vectormap<K, V>& src)
		{
			*this = src;
		}

		const vectormap<K, V>& operator=(const vectormap<K, V>& src)
		{
			if (this != &src)
			{
				m_vec = src.m_vec;
				m_map = src.m_map;
			}
			return *this;
		}

		vectormap<K, V>& operator=(vectormap<K, V>&& src) noexcept
		{
			if (this != &src)
			{
				m_vec = std::move(src.m_vec);
				m_map = std::move(src.m_map);
			}
			return *this;
		}

		/// <summary>
		/// Access the vector of pairs directly for index access or iteration
		/// </summary>
		const std::vector<std::pair<K, V>>& vec() const noexcept
		{
			return m_vec;
		}

		/// <summary>
		/// clear vectormap
		/// </summary>
		void clear() noexcept
		{
			m_map.clear();
			m_vec.clear();
		}

		bool empty() const noexcept
		{
			return m_vec.empty();
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

		V& get(const K& key)
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
			return operator[](index);
		}

		const V& operator[](size_t index) const
		{
			if (index < m_vec.size())
				return m_vec[index].second;

			throw std::runtime_error("index out of range");
		}

		V& operator[](size_t index)
		{
			if (index < m_vec.size())
				return m_vec[index].second;

			throw std::runtime_error("index out of range");
		}

		const V& front() const
		{
			if (!m_vec.empty())
				return m_vec.front().second;

			throw std::runtime_error("front called on empty vectormap");
		}

		V& front()
		{
			if (!m_vec.empty())
				return m_vec.front().second;

			throw std::runtime_error("front called on empty vectormap");
		}

		const V& back() const
		{
			if (!m_vec.empty())
				return m_vec.back().second;

			throw std::runtime_error("front called on empty vectormap");
		}

		V& back()
		{
			if (!m_vec.empty())
				return m_vec.back().second;

			throw std::runtime_error("front called on empty vectormap");
		}

		/// <summary>
		/// How many key-value pairs are in this?
		/// </summary>
		size_t size() const noexcept
		{
			return m_vec.size();
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
		void set_back(const K& key, const V& val)
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

		void append(const vectormap& source)
		{
			for (const auto& [key, value] : source.vec())
			{
				set_back(key, value);
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
			for (const auto& k : m_vec)
				retVal.emplace_back(k.first);
			return retVal;
		}

		/// <summary>
		/// Get a list of the values of this map
		/// </summary>
		std::vector<V> values() const
		{
			std::vector<V> retVal;
			retVal.reserve(m_vec.size());
			for (const auto& v : m_vec)
				retVal.emplace_back(v.second);

			return retVal;
		}

	private:
		std::unordered_map<K, size_t> m_map;
		std::vector<std::pair<K, V>> m_vec;
	};
}
