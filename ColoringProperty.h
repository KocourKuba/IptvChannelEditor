#pragma once

class ColoringProperty
{
public:
	ColoringProperty() = default;
	~ColoringProperty() = default;

	bool is_colored() const { return colored; }
	void set_colored(bool val) { colored = val; }

private:
	bool colored = false;
};
