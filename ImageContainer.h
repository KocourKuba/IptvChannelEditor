#pragma once

class ImageContainer
{
public:
	ImageContainer() = default;
	ImageContainer(const ImageContainer& src)
	{
		*this = src;
	}

	ImageContainer(const ImageContainer* src)
	{
		*this = src;
	}

	ImageContainer(ImageContainer&& src)
	{
		*this = std::move(src);
	}

public:
	ImageContainer* operator=(const ImageContainer& src)
	{
		if (this != &src)
		{
			set_image(src.get_image());
		}

		return this;
	}

	ImageContainer* operator=(const ImageContainer* src)
	{
		if (this != src)
		{
			set_image(src->get_image());
		}

		return this;
	}

	ImageContainer* operator=(ImageContainer&& src)
	{
		if (this != &src)
		{
			set_image(src.get_image());
		}
		return this;
	}

	const CImage& get_image() const { return icon; }
	CImage& get_image() { return icon; }

	// move source image
	void set_image(CImage& val);

	// copy source image
	void set_image(const CImage& src);

private:
	CImage icon;
};
