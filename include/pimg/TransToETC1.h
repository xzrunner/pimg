#ifndef _PIMG_TRANS_TO_ETC1_H_
#define _PIMG_TRANS_TO_ETC1_H_

#include <string>

#include <stdint.h>

namespace pimg
{

class TransToETC1
{
public:
	TransToETC1(const uint8_t* pixels, int width, int height, int channels,
		bool align_bottom = false, bool fastest = false);
	~TransToETC1();

	void OutputFile(const std::string& filepath) const;

	uint8_t* GetPixelsData(int& width, int& height) const;

private:
	void InitSrcImage(const uint8_t* pixels, int width, int height, int channels,
		bool align_bottom);
	void InitSrcImageAlpha();

	void InitETC1Header();

	void EncodeETC1();
	void EncodeETC1ByRgEtc1();
	void EncodeETC1ByEtcPack();

private:
	struct PKMHeader {
		unsigned char identifier[8];
		unsigned char paddedWidthMSB;
		unsigned char paddedWidthLSB;
		unsigned char paddedHeightMSB;
		unsigned char paddedHeightLSB;
		unsigned char widthMSB;
		unsigned char widthLSB;
		unsigned char heightMSB;
		unsigned char heightLSB;
	};

private:
	uint8_t *m_pixels, *m_alpha_pixels;
	int m_width, m_height;

	size_t m_etc1_size;
	uint8_t *m_etc1_pixels, *m_etc1_alpha_pixels;

	PKMHeader m_header;

	bool m_fastest;

}; // TransToETC1

}

#endif // _PIMG_TRANS_TO_ETC1_H_