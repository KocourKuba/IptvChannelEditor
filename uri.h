#pragma once

class uri
{
public:
	static constexpr auto PLUGIN_SCHEME = L"plugin_file://";

public:
	uri() = default;

public:
	virtual void clear() { schema.clear(); path.clear(); }

	virtual std::wstring get_uri() const { return schema + path; }
	virtual void set_uri(const std::wstring& url);

	virtual std::wstring get_path() const { return path; }
	virtual void set_path(const std::wstring& val) { path = val; };

	virtual std::wstring get_schema() const { return schema; }
	virtual void set_schema(const std::wstring& val) { schema = val; };

	bool is_local() const { return schema == PLUGIN_SCHEME; }

	std::wstring get_icon_relative_path(LPCWSTR szRoot = nullptr) const;

	bool operator==(const uri& src) const { return src.get_schema() == schema && src.get_path() == path; }
	bool operator!=(const uri& src) const { return src.get_schema() != schema && src.get_path() != path; }

protected:
	std::wstring schema;
	std::wstring path;
};

class uri_stream : public uri
{
public:
	uri_stream() = default;

public:
	void clear() override;

	/// <summary>
	/// getter for playable url
	/// return url without ts:// substring
	/// and substituted channel id
	/// </summary>
	/// <returns></returns>
	std::wstring get_ts_translated_url() const;

	/// <summary>
	/// getter for id translated uri
	/// return url with substituted channel id
	/// </summary>
	/// <returns></returns>
	std::wstring get_id_translated_url() const;

	/// <summary>
	/// setter not translated uri
	/// </summary>
	/// <param name="url"></param>
	void set_uri(const std::wstring& url) override;

	/// <summary>
	/// getter channel id
	/// </summary>
	/// <returns></returns>
	int get_Id() const { return templated ? id : hash; }
	/// <summary>
	/// setter channel id
	/// </summary>
	/// <param name="val"></param>
	void set_Id(int val) { id = val; }

	/// <summary>
	/// check if uri templated
	/// </summary>
	/// <returns></returns>
	bool is_template() const { return templated; }

protected:
	int id = 0;
	bool templated = false;
	int hash = 0;
};

