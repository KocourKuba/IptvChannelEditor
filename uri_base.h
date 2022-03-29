#pragma once

enum class StreamSubType
{
	enHLS = 0,
	enHLS2,
	enMPEGTS,
};


/// <summary>
/// Base uri class
/// </summary>
class uri_base
{
public:
	static constexpr auto PLUGIN_SCHEME = L"plugin_file://";

public:
	uri_base() = default;
	virtual ~uri_base() = default;

public:

	/// <summary>
	/// clear uri
	/// </summary>
	virtual void clear() { schema.clear(); path.clear(); set_template(false); }

	/// <summary>
	/// get combined uri
	/// </summary>
	/// <returns>string</returns>
	virtual std::wstring get_uri() const { return schema + path; }

	/// <summary>
	/// set combined uri
	/// </summary>
	/// <returns></returns>
	virtual void set_uri(const std::wstring& url);

	/// <summary>
	/// get path
	/// </summary>
	/// <returns>string</returns>
	virtual std::wstring get_path() const { return path; }

	/// <summary>
	/// set path
	/// </summary>
	/// <returns></returns>
	virtual void set_path(const std::wstring& val) { path = val; };

	/// <summary>
	/// get schema (http://, https://, plugin_file://)
	/// </summary>
	/// <returns>string</returns>
	virtual std::wstring get_schema() const { return schema; }
	/// <summary>
	/// set schema
	/// </summary>
	/// <returns></returns>
	virtual void set_schema(const std::wstring& val) { schema = val; };

	/// <summary>
	/// is uri local (plugin_file), internal schema for plugin
	/// </summary>
	/// <returns>string</returns>
	bool is_local() const { return schema == PLUGIN_SCHEME; }

	/// <summary>
	/// set uri templated
	/// </summary>
	/// <returns></returns>
	void set_template(bool val) { templated = val; }

	/// <summary>
	/// check is uri templated
	/// </summary>
	/// <returns>bool</returns>
	bool is_template() const { return templated; }

	/// <summary>
	/// is uri valid
	/// </summary>
	/// <returns>bool</returns>
	bool is_valid() const { return is_template() ? true : (!schema.empty() && !path.empty());  }

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
		return src.get_schema() == schema && src.get_path() == path;
	}

	/// <summary>
	/// compare uri
	/// </summary>
	/// <param name="src"></param>
	/// <returns></returns>
	bool operator!=(const uri_base& src) const
	{
		return src.get_schema() != schema || src.get_path() != path;
	}

protected:
	std::wstring schema;
	std::wstring path;
	bool templated = false;
};
