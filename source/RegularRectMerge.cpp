#include "pimg/RegularRectMerge.h"
#include "pimg/PixelUncoveredLUT.h"
#include "pimg/RegularRectCondense.h"

#include <algorithm>

#include <assert.h>

namespace pimg
{

RegularRectMerge::RegularRectMerge(const CU_VEC<CRect>& rects, int width, 
								   int height, bool* ori_pixels)
	: m_width(width)
	, m_height(height)
{
	m_uncovered_lut = new PixelUncoveredLUT(width, height);
	m_condense = new RegularRectCondense(rects, width, height, ori_pixels);
}

RegularRectMerge::~RegularRectMerge()
{
	delete m_condense;
	delete m_uncovered_lut;
}

void RegularRectMerge::Merge()
{
	static const int COUNT_MIN = 6;
	static const int AREA_MIN = 16 * 16;
	const int ENLARGE_MAX = (int)std::max(16 * 16 * 2.0f, m_width * 0.1f * m_height * 0.1f);

	int debug_count = 0;

	m_condense->Condense(CRect(0, 0, m_width, m_height));

	while (true) 
	{
		CU_VEC<CRect> rects;
		m_condense->GetSortedCRects(rects);
		if (rects.size() <= 1) {
			break;
		}

		assert(rects[0].w > 0 && rects[0].h > 0);
		//if (rects[0].w * rects[0].h > AREA_MIN && rects.size() < COUNT_MIN) {
		//	break;
		//}

		m_uncovered_lut->LoadCRects(rects);

		int cost_min = INT_MAX;
		int r0_min = -1, r1_min = -1;

		//int r_min = -1;
// 		for (int i = 1, n = rects.size(); i < n; ++i) {
// 			const CRect& r1 = rects[i];
// 
// 			int left = std::min(r0.x, r1.x),
// 				right = std::max(r0.x + r0.w, r1.x + r1.w);
// 			int down = std::min(r0.y, r1.y),
// 				up = std::max(r0.y + r0.h, r1.y + r1.h);
// 			CRect mr(left, down, right - left, up - down);
// 
// 			int cost = ComputeCost(mr, rects);
// 			if (cost == 0) {
// 				cost_min = 0;
// 				r_min = i;
// 				break;
// 			} else if (cost < cost_min) {
// 				cost_min = cost;
// 				r_min = i;
// 			}
// 		}

		for (int i = 0, n = rects.size(); i < n - 1 && cost_min > 0; ++i) {
			const CRect& r0 = rects[i];
			if (r0.is_const) {
				continue;
			}
			for (int j = i + 1; j < n && cost_min > 0; ++j) {
				const CRect& r1 = rects[j];
				if (r1.is_const) {
					continue;
				}
	 			int left = std::min(r0.x, r1.x),
	 				right = std::max(r0.x + r0.w, r1.x + r1.w);
	 			int down = std::min(r0.y, r1.y),
	 				up = std::max(r0.y + r0.h, r1.y + r1.h);
	 			CRect mr(left, down, right - left, up - down);	
	 			int cost = ComputeCost(mr, rects);
	 			if (cost == 0) {
	 				cost_min = 0;
	 				r0_min = i;
					r1_min = j;
	 				break;
	 			} else if (cost < cost_min) {
	 				cost_min = cost;
					r0_min = i;
					r1_min = j;

	 			}
			}
		}

		if (cost_min > ENLARGE_MAX) {
			break;
		}

		assert(r0_min != -1 && r1_min != -1);

		const CRect& r0 = rects[r0_min];
		const CRect& r1 = rects[r1_min];

		int left = std::min(r0.x, r1.x),
			right = std::max(r0.x + r0.w, r1.x + r1.w);
		int down = std::min(r0.y, r1.y),
			up = std::max(r0.y + r0.h, r1.y + r1.h);
		CRect mr(left, down, right - left, up - down);

// 		if (debug_count == 1) {
// 			break;
// 		}

		m_condense->Remove(r0);
		m_condense->Remove(r1);
		m_condense->Insert(mr);
		m_condense->Condense(mr);

		++debug_count;
// 		if (debug_count == 3) {
// 			break;
// 		}
	}
}

void RegularRectMerge::GetResult(CU_VEC<CRect>& result) const
{
	m_condense->GetSortedCRects(result);
}

int RegularRectMerge::ComputeCost(const CRect& r, const CU_VEC<CRect>& rects) const
{
	int cost = 0;
	for (int i = 0, n = rects.size(); i < n; ++i) 
	{
		const CRect& b = rects[i];

		if (r.x > b.x && (r.x + r.w < b.x + b.w) &&
			r.y < b.y && (r.y + r.h > b.y + b.h)) {
			return INT_MAX;
		} 
		if (b.x > r.x && (b.x + b.w < r.x + r.w) &&
			b.y < r.y && (b.y + b.h > r.y + r.h)) {
			return INT_MAX;
		}

		if (r.x > b.x && (r.x < b.x + b.w) && r.y > b.y && (r.y < b.y + b.h) &&
			(r.x + r.w > b.x + b.w) && (r.y + r.h > b.y + b.h)) {
			cost += (b.x + b.w - r.x) * (b.y + b.h - r.y);
		} 
		if (r.x > b.x && (r.x < b.x + b.w) && (r.y + r.h > b.y) && (r.y + r.h < b.y + b.h) &&
			(r.x + r.w > b.x + b.w) && (r.y < b.y)) {
			cost += (b.x + b.w - r.x) * (r.y + r.h - 1 - b.y);
		}
		if ((r.x + r.w > b.x) && (r.x + r.w < b.x + b.w) && (r.y + r.h > b.y) && (r.y + r.h < b.y + b.h) &&
			r.x < b.x && r.y < b.y) {
			cost += (r.x + r.w - 1 - b.x) * (r.y + r.h - 1 - b.y);
		}
		if ((r.x + r.w > b.x) && (r.x + r.w < b.x + b.w) && (r.y > b.y) && (r.y < b.y + b.h) &&
			r.x < b.x && (r.y + r.h > b.y + b.h)) {
			cost += (r.x + r.w - 1 - b.x) * (b.y + b.h - r.y);
		}
	}

	if (m_uncovered_lut->IntersectConstArea(r.x, r.y, r.w, r.h)) {
		return INT_MAX;
	}

	cost += m_uncovered_lut->GetUncoveredArea(r.x, r.y, r.w, r.h);
	return cost;
}

}