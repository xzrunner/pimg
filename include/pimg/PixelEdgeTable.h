#ifndef _EASYIMAGE_PIXEL_EDGE_TABLE_H_
#define _EASYIMAGE_PIXEL_EDGE_TABLE_H_

#include <map>

namespace pimg
{

class PixelEdgeTable
{
public:
	PixelEdgeTable(const bool* pixels, int width, int height);

	int GetCRectArea(int x, int y, int w, int h, int limit) const;
	void CutByCRect(int x, int y, int w, int h, int& left_area);

private:
	void Load(const bool* pixels, int width, int height);

private:
	struct Line
	{
		Line() : area(0) {}

		int area;
		std::map<int, int> worlds;
	};

private:
	std::map<int, Line> m_lines;

}; // PixelEdgeTable

}

#endif // _EASYIMAGE_PIXEL_EDGE_TABLE_H_