#ifndef _EASYIMAGE_REGULAR_RECT_CONDENSE_H_
#define _EASYIMAGE_REGULAR_RECT_CONDENSE_H_

#include "pimg/CRect.h"

#include <cu/cu_stl.h>

namespace pimg
{

class RegularRectCondense
{
public:
	RegularRectCondense(const CU_VEC<CRect>& rects, int width, int height, bool* ori_pixels);
	~RegularRectCondense();

	void Condense(const CRect& r);

	void Remove(const CRect& r);
	void Insert(const CRect& r);

	void GetSortedCRects(CU_VEC<CRect>& rects) const;

private:
	void LoadPixels(const CU_VEC<CRect>& rects, bool* ori_pixels);

private:
	class Pixel;

	struct Grid
	{
		Grid(const CRect& r) : r(r) {}
		CRect r;
	}; // Grid

	class AreaCmp
	{
	public:
		bool operator () (const Grid* g0, const Grid* g1) const;
	}; // AreaCmp

	class Pixel
	{
	public:
		Pixel(bool has_data) : m_has_data(has_data) {}

		void Insert(Grid* g);
		void Remove(Grid* g);

		bool IsSingleCoverd() const {
			return m_grids.size() == 1;
		}
		bool HasData() const {
			return m_has_data;
		}

		Grid* QueryGrid(const CRect& r) const;

	private:
		CU_VEC<Grid*> m_grids;

		const bool m_has_data;

	}; // Pixel

private:
	enum Direction
	{
		e_left,
		e_right,
		e_down,
		e_up
	};

	void CondenseEmpty(Grid* g);
	bool IsPixelImmoveable(int x, int y) const;

	void CondenseCovered(const CRect& r);
	bool CondenseCovered(Grid* s, Grid* l);

	void RemoveSize(Grid* g, Direction dir, int len);

	void RemovePixelGrid(const CRect& r, Grid* g);
	void InsertPixelGrid(const CRect& r, Grid* g);

	bool IsCRectIntersect(const CRect& r0, const CRect& r1) const;

private:
	int m_width, m_height;
	Pixel** m_pixels;

	CU_VEC<Grid*> m_grids;

}; // RegularRectCondense

}

#endif // _EASYIMAGE_REGULAR_RECT_CONDENSE_H_