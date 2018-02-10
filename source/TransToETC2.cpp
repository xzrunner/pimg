#include "pimg/TransToETC2.h"
#include "pimg/ImagePack.h"

#include <guard/check.h>
#include <SM_Math.h>

#include <etcpack_lib.h>

#include <fstream>

namespace pimg
{

static bool INITED = false;

TransToETC2::TransToETC2(const uint8_t* pixels, int width, int height, int channels,
						 Type type, bool align_bottom, bool fastest)
	: m_type(type)
	, m_fastest(fastest)
	, m_perceptual(false)
{
	if (!INITED) {
		readCompressParams();
		setupAlphaTableAndValtab();
		INITED = true;
	}

	Prepare(pixels, width, height, channels, align_bottom);
	Encode();
}

TransToETC2::~TransToETC2()
{
	delete m_rgb_data;
	delete m_alpha_data;
	delete m_compressed;
}

void TransToETC2::OutputFile(const std::string& filepath) const
{
	std::locale::global(std::locale(""));
	std::ofstream fout(filepath.c_str(), std::ios::binary);
	std::locale::global(std::locale("C"));
	OutputHeader(fout);
	fout.write(reinterpret_cast<const char*>(m_compressed), m_size);
	fout.close();
}

uint8_t* TransToETC2::GetPixelsData(int& width, int& height) const
{
	width = m_width;
	height = m_height;

	int sz = m_width * m_height;
	uint8_t* pixels = new uint8_t[m_size];
	memcpy(pixels, m_compressed, sz);
	return pixels;
}

void TransToETC2::Prepare(const uint8_t* pixels)
{
	int rgb_sz = m_width * m_height * 3,
		alpha_sz = m_width * m_height;
	m_rgb_data = new uint8_t[rgb_sz];
	m_alpha_data = new uint8_t[alpha_sz];

	const uint8_t* in = pixels;
	uint8_t *out_rgb = m_rgb_data,
		    *out_alpha = m_alpha_data;
	const uint8_t* end = in + m_width * m_height * 4;
	while (in < end)
	{
		memcpy(out_rgb, in, 3);
		out_rgb += 3;
		in += 3;

		if (m_type == RGBA1) {
			*out_alpha++ = (*in++) > 180 ? 255 : 0;
		} else {
			*out_alpha++ = *in++;
		}
	}
}

void TransToETC2::Prepare(const uint8_t* pixels, int width, int height, int channels, bool align_bottom)
{
	GD_ASSERT(channels == 4, "err channels.");
	if (sm::is_power_of_two(width) &&
		sm::is_power_of_two(height) &&
		width == height) 
	{
		m_width = width;
		m_height = height;
		Prepare(pixels);
	}
	else
	{
		int nw = sm::next_p2(width),
			nh = sm::next_p2(height);
		pimg::ImagePack pack(nw, nh);
		int h = align_bottom ? nh - height : 0;
		pack.AddImage(pixels, width, height, 0, h, pimg::ImagePack::PT_NORMAL);

		m_width = nw;
		m_height = nh;
		Prepare(pack.GetPixels());
	}

	if (m_type == RGBA) {
		m_size = m_width * m_height;
	} else {
		m_size = m_width * m_height / 2;		
	}

	m_compressed = new uint8_t[m_size];
}

static void WriteRGB(uint8_t*& output, unsigned int block1, unsigned int block2)
{
	output[0] = (block1 >> 24) & 0xFF;
	output[1] = (block1 >> 16) & 0xFF;
	output[2] = (block1 >> 8) & 0xFF;
	output[3] =  block1 & 0xFF;
	output[4] = (block2 >> 24) & 0xFF;
	output[5] = (block2 >> 16) & 0xFF;
	output[6] = (block2 >> 8) & 0xFF;
	output[7] =  block2 & 0xFF;
	output += 8;
}

static void WriteAlpha(uint8_t*& output, uint8_t* alphadata)
{
	memcpy(output, alphadata, 8);
	output += 8;
}

void TransToETC2::Encode()
{
	uint8_t* decoded = new uint8_t[m_width * m_height * 3];

	uint8_t* output = m_compressed;

	int bw = m_width / 4;
	int bh = m_height / 4;
	for (int y = 0; y < bh; ++y) {
		for (int x = 0; x < bw; ++x) {
			//pimg::ConsoleProgressBar::Print(y * bw + x, bh * bw);
			switch (m_type) 
			{
			case RGB:
				{
					unsigned int block1, block2;
					ComputeRGB(decoded, x, y, block1, block2);
					WriteRGB(output, block1, block2);
				}
				break;
			case RGBA:
				{
					uint8_t alphadata[8];
					ComputeAlpha(x, y, alphadata);
					WriteAlpha(output, alphadata);

					unsigned int block1, block2;
					ComputeRGB(decoded, x, y, block1, block2);
					WriteRGB(output, block1, block2);					
				}
				break;
			case RGBA1:
				{
					unsigned int block1, block2;
					compressBlockETC2RGBA1(m_rgb_data, m_alpha_data, decoded, m_width, m_height, x * 4, y * 4, block1, block2);
					WriteRGB(output, block1, block2);					
				}
				break;
			default:
				GD_REPORT_ASSERT("Unknown type");
			}
		}
	}
}

void TransToETC2::OutputHeader(std::ofstream& fout) const
{
	unsigned char magic[4] = {'P', 'K', 'M', ' '};
	fout.write(reinterpret_cast<const char*>(&magic[0]), 1);
	fout.write(reinterpret_cast<const char*>(&magic[1]), 1);
	fout.write(reinterpret_cast<const char*>(&magic[2]), 1);
	fout.write(reinterpret_cast<const char*>(&magic[3]), 1);

	unsigned char version[2] = { '2', '0' };
	fout.write(reinterpret_cast<const char*>(&version[0]), 1);
	fout.write(reinterpret_cast<const char*>(&version[1]), 1);

	// In a PKM-file, the codecs are stored using the following identifiers
	// 
	// identifier                         value               codec
	// --------------------------------------------------------------------
	// ETC1_RGB_NO_MIPMAPS                  0                 GL_ETC1_RGB8_OES
	// ETC2PACKAGE_RGB_NO_MIPMAPS           1                 GL_COMPRESSED_RGB8_ETC2
	// ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD      2, not used       -
	// ETC2PACKAGE_RGBA_NO_MIPMAPS          3                 GL_COMPRESSED_RGBA8_ETC2_EAC
	// ETC2PACKAGE_RGBA1_NO_MIPMAPS         4                 GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
	// ETC2PACKAGE_R_NO_MIPMAPS             5                 GL_COMPRESSED_R11_EAC
	// ETC2PACKAGE_RG_NO_MIPMAPS            6                 GL_COMPRESSED_RG11_EAC
	// ETC2PACKAGE_R_SIGNED_NO_MIPMAPS      7                 GL_COMPRESSED_SIGNED_R11_EAC
	// ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS     8                 GL_COMPRESSED_SIGNED_RG11_EAC

	enum ETC_TYPE
	{
		ETC1_RGB_NO_MIPMAPS = 0,
		ETC2PACKAGE_RGB_NO_MIPMAPS,
		ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD,
		ETC2PACKAGE_RGBA_NO_MIPMAPS,
		ETC2PACKAGE_RGBA1_NO_MIPMAPS,
		ETC2PACKAGE_R_NO_MIPMAPS,
		ETC2PACKAGE_RG_NO_MIPMAPS,
		ETC2PACKAGE_R_SIGNED_NO_MIPMAPS,
		ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS
	};

	unsigned short type;
	switch (m_type)
	{
	case RGB:
		type = ETC2PACKAGE_RGB_NO_MIPMAPS;
		break;
	case RGBA:
		type = ETC2PACKAGE_RGBA_NO_MIPMAPS;
		break;
	case RGBA1:
		type = ETC2PACKAGE_RGBA1_NO_MIPMAPS;
		break;
	default:
		GD_REPORT_ASSERT("Unknown type.");
	}
	WriteBigEndian2byteWord(&type, fout);

	unsigned short w = m_width, h = m_height;
	WriteBigEndian2byteWord(&w, fout);
	WriteBigEndian2byteWord(&h, fout);

	WriteBigEndian2byteWord(&w, fout);
	WriteBigEndian2byteWord(&h, fout);	
}

void TransToETC2::ComputeRGB(uint8_t* decoded, int x, int y, unsigned int& block1, unsigned int& block2)
{
	if (m_perceptual) {
		if (m_fastest) {
			compressBlockETC2FastPerceptual(m_rgb_data, decoded, m_width, m_height, x * 4, y * 4, block1, block2);
		} else {
			compressBlockETC2ExhaustivePerceptual(m_rgb_data, decoded, m_width, m_height, x * 4, y * 4, block1, block2);
		}
	} else {
		if (m_fastest) {
			compressBlockETC2Fast(m_rgb_data, NULL, decoded, m_width, m_height, x * 4, y * 4, block1, block2);
		} else {
			compressBlockETC2Exhaustive(m_rgb_data, decoded, m_width, m_height, x * 4, y * 4, block1, block2);
		}
	}
}

void TransToETC2::ComputeAlpha(int x, int y, uint8_t* alphadata)
{
	if (m_fastest) {
		compressBlockAlphaFast(m_alpha_data, x * 4, y * 4, m_width, m_height, alphadata);
	} else {
		compressBlockAlphaSlow(m_alpha_data, x * 4, y * 4, m_width, m_height, alphadata);
	}
}

void TransToETC2::WriteBigEndian2byteWord(unsigned short* blockadr, std::ofstream& fout)
{
	uint8 bytes[2];
	unsigned short block;

	block = blockadr[0];

	bytes[0] = (block >> 8) & 0xff;
	bytes[1] = (block >> 0) & 0xff;

	fout.write(reinterpret_cast<const char*>(&bytes[0]), 1);
	fout.write(reinterpret_cast<const char*>(&bytes[1]), 1);
}

void TransToETC2::WriteBigEndian4byteWord(unsigned short* blockadr, std::ofstream& fout)
{
	uint8 bytes[4];
	unsigned int block;

	block = blockadr[0];

	bytes[0] = (block >> 24) & 0xff;
	bytes[1] = (block >> 16) & 0xff;
	bytes[2] = (block >> 8) & 0xff;
	bytes[3] = (block >> 0) & 0xff;

	fout.write(reinterpret_cast<const char*>(&bytes[0]), 1);
	fout.write(reinterpret_cast<const char*>(&bytes[1]), 1);
	fout.write(reinterpret_cast<const char*>(&bytes[2]), 1);
	fout.write(reinterpret_cast<const char*>(&bytes[3]), 1);
}

}