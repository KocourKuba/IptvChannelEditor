#pragma once

/// <summary>
/// Base uri class
/// </summary>
class uri_base
{
public:
	static constexpr auto PLUGIN_SCHEME = "plugin_file://";

public:
	uri_base() = default;

public:
	virtual void clear() { schema.clear(); path.clear(); set_template(true); }

	/// <summary>
	/// get combined uri
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_uri() const { return schema + path; }

	/// <summary>
	/// set combined uri
	/// </summary>
	/// <returns></returns>
	virtual void set_uri(const std::string& url);

	/// <summary>
	/// get path
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_path() const { return path; }
	/// <summary>
	/// set path
	/// </summary>
	/// <returns></returns>
	virtual void set_path(const std::string& val) { path = val; };

	/// <summary>
	/// get schema (http://, https://, plugin_file://)
	/// </summary>
	/// <returns>string</returns>
	virtual std::string get_schema() const { return schema; }
	/// <summary>
	/// set schema
	/// </summary>
	/// <returns></returns>
	virtual void set_schema(const std::string& val) { schema = val; };

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
	/// get filesystem path
	/// convert local to filesystem path
	/// </summary>
	/// <returns>string</returns>
	std::string get_filesystem_path(const std::string& root) const;
	std::wstring get_filesystem_path(const std::wstring& root) const;

	bool operator==(const uri_base& src) const { return src.get_schema() == schema && src.get_path() == path; }
	bool operator!=(const uri_base& src) const { return src.get_schema() != schema && src.get_path() != path; }

protected:
	std::string schema;
	std::string path;
	bool templated = true;
};
