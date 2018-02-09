#pragma once

#include <boost/noncopyable.hpp>

#include <string>

namespace pimg
{

class ImageData : boost::noncopyable
{
public:
	ImageData(bool pre_mul_alpha);
	virtual ~ImageData();

	bool LoadFromFile(const std::string& filepath);

	const uint8_t* GetPixelData() const { return m_pixels; }
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	int GetFormat() const { return m_format; }

private:
	bool m_pre_mul_alpha;

	uint8_t* m_pixels;
	int m_width, m_height;
	int m_format;

}; // ImageData

}