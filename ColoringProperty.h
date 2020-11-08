#pragma once

class ColoringProperty
{
public:
	ColoringProperty() = default;
	~ColoringProperty() = default;

	bool is_colored() const { return colored; }
	void set_colored(bool val) { colored = val; }

	bool is_disabled() const { return disabled; }
	void set_disabled(bool val) { disabled = val; }

private:
	bool colored = false;
	bool disabled = false;
};
