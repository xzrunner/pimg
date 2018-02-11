#ifndef _EASYIMAGE_PIXEL_COVERED_LUT_H_
#define _EASYIMAGE_PIXEL_COVERED_LUT_H_

#include "pimg/CRect.h"

#include <cu/cu_stl.h>

namespace pimg
{

class PixelCoveredLUT
{
public:
	PixelCoveredLUT(int width, int height, const CU_VEC<CRect>& rects);
	~PixelCoveredLUT();

	int GetCoveredArea(int x, int y, int w, int h) const;

	void Remove(const CRect& r);
	void Insert(const CRect& r);

private:
	void LoadCRects(const CU_VEC<CRect>& rects);

private:
	int* m_covered_count;
	int* m_area;

	int m_width, m_height;

}; // PixelCoveredLUT

}

#endif // _EASYIMAGE_PIXEL_COVERED_LUT_H_