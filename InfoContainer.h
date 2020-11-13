#pragma once

class InfoContainer
{
public:
	InfoContainer() = default;
	~InfoContainer() = default;

	const std::string& get_audio() const { return audio; }
	void set_audio(const std::string& val) { audio = val; }

	const std::string& get_video() const { return video; }
	void set_video(const std::string& val) { video = val; }

private:
	std::string audio;
	std::string video;
};