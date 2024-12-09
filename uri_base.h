/*
IPTV Channel Editor

The MIT License (MIT)

Author and copyright (2021-2024): sharky72 (https://github.com/KocourKuba)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#pragma once

/// <summary>
/// Base uri class
/// </summary>
class uri_base
{
public:

	/// <summary>
	/// clear uri
	/// </summary>
	virtual void clear() { scheme.clear(); path.clear(); }

	/// <summary>
	/// is uri valid
	/// </summary>
	/// <returns>bool</returns>
	virtual bool is_valid() const { return !scheme.empty() && !path.empty(); }

	/// <summary>
	/// get combined uri
	/// </summary>
	/// <returns>string</returns>
	std::wstring get_uri() const { return scheme + path; }

	/// <summary>
	/// set combined uri
	/// </summary>
	/// <returns></returns>
	void set_uri(const std::wstring& url);

	/// <summary>
	/// get path
	/// </summary>
	/// <returns>string</returns>
	std::wstring get_path() const { return path; }

	/// <summary>
	/// set path
	/// </summary>
	/// <returns></returns>
	void set_path(const std::wstring& val) { path = val; };

	/// <summary>
	/// get schema (http://, https://, plugin_file://)
	/// </summary>
	/// <returns>string</returns>
	const std::wstring& get_scheme() const { return scheme; }

	/// <summary>
	/// set schema
	/// </summary>
	/// <returns></returns>
	void set_scheme(const std::wstring& val) { scheme = val; };

	/// <summary>
	/// is uri local (plugin_file), internal schema for plugin
	/// </summary>
	/// <returns>string</returns>
	bool is_local() const { return scheme == utils::PLUGIN_SCHEME; }

	/// <summary>
	/// get filesystem path
	/// convert local to filesystem path
	/// </summary>
	/// <param name="root">root path for combine absolute path</param>
	/// <returns></returns>
	std::wstring get_filesystem_path(const std::wstring& root) const;

	/// <summary>
	/// compare uri
	/// </summary>
	/// <param name="src">uri</param>
	/// <param name="compare_scheme">take in account scheme</param>
	/// <returns></returns>
	bool is_equal(const uri_base& src, bool compare_scheme = true) const
	{
		if (compare_scheme)
			return operator==(src);

		return src.get_path() == path;
	}

	/// <summary>
	/// compare uri
	/// </summary>
	/// <param name="src"></param>
	/// <returns></returns>
	bool operator==(const uri_base& src) const
	{
		return src.get_scheme() == scheme && src.get_path() == path;
	}

	/// <summary>
	/// compare uri
	/// </summary>
	/// <param name="src"></param>
	/// <returns></returns>
	bool operator!=(const uri_base& src) const
	{
		return src.get_scheme() != scheme || src.get_path() != path;
	}

	/// <summary>
	/// copy info
	/// </summary>
	/// <returns>uri_stream&</returns>
	const uri_base& operator=(const uri_base& src)
	{
		if (&src != this)
		{
			set_scheme(src.get_scheme());
			set_path(src.get_path());
		}

		return *this;
	}

protected:
	std::wstring scheme;
	std::wstring path;
};
