#include "pimg/PixelCoveredLUT.h"

#include <algorithm>

#include <assert.h>

namespace pimg
{

PixelCoveredLUT::PixelCoveredLUT(int width, int height, const CU_VEC<CRect>& rects)
	: m_width(width)
	, m_height(height)
{
	LoadCRects(rects);
}

PixelCoveredLUT::~PixelCoveredLUT()
{
	delete[] m_covered_count;
	delete[] m_area;
}

int PixelCoveredLUT::GetCoveredArea(int x, int y, int w, int h) const
{
	assert(w >= 1 && h >= 1
		&& x >= 0 && x + w <= m_width
		&& y >= 0 && y + h <= m_height);
	int left_down_area = (x > 0 && y > 0) ? m_area[(y-1)*m_width+x-1] : 0;
	int left_area = x > 0 ? m_area[(y-1+h)*m_width+x-1] : 0;
	int right_area = y > 0 ? m_area[(y-1)*m_width+x-1+w] : 0;
	int right_up_area = m_area[(y-1+h)*m_width+x-1+w];

	int area = right_up_area - left_area - right_area + left_down_area;
	assert(area >= 0 && area <= m_area[m_width*m_height-1]);
	return area;
}

void PixelCoveredLUT::Remove(const CRect& r)
{
	for (int x = r.x; x < r.x + r.w; ++x) {
		for (int y = r.y; y < r.y + r.h; ++y) {
			int idx = y * m_width + x;
			assert(m_covered_count[idx] > 0);
			--m_covered_count[idx];
		}
	}

	for (int x = r.x; x < m_width; ++x) {
		for (int y = r.y; y < m_height; ++y) {
			int w = std::min(x - r.x + 1, r.w),
				h = std::min(y - r.y + 1, r.h);
			int idx = y * m_width + x;
			m_area[idx] -= w * h;
			assert(m_area[idx] >= 0);
		}
	}
}

void PixelCoveredLUT::Insert(const CRect& r)
{
	for (int x = r.x; x < r.x + r.w; ++x) {
		for (int y = r.y; y < r.y + r.h; ++y) {
			int idx = y * m_width + x;
			++m_covered_count[idx];
		}
	}

	for (int x = r.x; x < m_width; ++x) {
		for (int y = r.y; y < m_height; ++y) {
			int w = std::min(x - r.x + 1, r.w),
				h = std::min(y - r.y + 1, r.h);
			m_area[y * m_width + x] += w * h;
		}
	}
}

void PixelCoveredLUT::LoadCRects(const CU_VEC<CRect>& rects)
{
	int sz = m_width * m_height;

	m_covered_count = new int[sz];
	memset(m_covered_count, 0, sizeof(int) * sz);
	for (int i = 0, n = rects.size(); i < n; ++i) {
		const CRect& r = rects[i];
		for (int x = r.x; x < r.x + r.w; ++x) {
			assert(x >= 0 && x < m_width);
			for (int y = r.y; y < r.y + r.h; ++y) {
				assert(y >= 0 && y < m_height);
				++m_covered_count[y * m_width + x];
			}
		}
	}

	m_area = new int[sz];
	memset(m_area, 0, sizeof(int) * sz);
	int curr_idx = 0;
	for (int y = 0; y < m_height; ++y) {
		for (int x = 0; x < m_width; ++x) {
			int left = x > 0 ? m_area[curr_idx-1] : 0;
			int down = y > 0 ? m_area[curr_idx-m_width] : 0;
			int left_down = (x > 0 && y > 0) ? m_area[curr_idx-m_width-1] : 0;
			int curr = left + down - left_down + m_covered_count[curr_idx];
			m_area[curr_idx] = curr;

			++curr_idx;
		}
	}
}

}