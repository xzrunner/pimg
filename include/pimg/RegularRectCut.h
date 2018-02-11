#ifndef _EASYIMAGE_REGULAR_RECT_CUT_H_
#define _EASYIMAGE_REGULAR_RECT_CUT_H_

#include "pimg/CRect.h"

#include <cu/cu_stl.h>

#include <stdint.h>

namespace pimg
{

class PixelAreaLUT;

class RegularRectCut
{
public:
	RegularRectCut(const uint8_t* pixels, int w, int h);
	RegularRectCut(const uint8_t* pixels, int w, int h, const CU_VEC<CRect>& pre_rects);
	~RegularRectCut();

	void AutoCut();
	
	const CU_VEC<CRect>& GetResult() { return m_result; }

	// statics
	int GetLeftArea() const { return m_left_area; }
	int GetUseArea() const;

private:
	void LoadPixels(const uint8_t* pixels, int width, int height);

	void AutoCutWithLimit(float limit);
	int CalBestCRectPos(int w, int h, int& ret_x, int& ret_y);

private:
	bool* m_pixels;
	int m_width, m_height;
	int m_left_area;

	float m_density;

	PixelAreaLUT* m_area_array;	// real data

	CU_VEC<CRect> m_result;

}; // RegularRectCut

}

#endif // _EASYIMAGE_REGULAR_RECT_CUT_H_