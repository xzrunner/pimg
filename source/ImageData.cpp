#include "pimg/ImageData.h"

#include <gimg_import.h>
#include <gimg_typedef.h>

namespace pimg
{

ImageData::ImageData(bool pre_mul_alpha)
	: m_pre_mul_alpha(pre_mul_alpha)
	, m_pixels(nullptr)
	, m_width(0)
	, m_height(0)
	, m_format(0)
{
}

ImageData::~ImageData()
{
	if (m_pixels) {
		free(m_pixels);
	}
}

bool ImageData::LoadFromFile(const std::string& filepath)
{
	uint8_t* pixels = gimg_import(filepath.c_str(), &m_width, &m_height, &m_format);
	if (m_format == GPF_RGBA8 && m_pre_mul_alpha) {
		gimg_pre_mul_alpha(pixels, m_width, m_height);
	}
	return true;
}

}