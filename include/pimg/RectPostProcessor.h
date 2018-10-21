#ifndef _EASYIMAGE_RECT_MANAGER_H_
#define _EASYIMAGE_RECT_MANAGER_H_

#include "pimg/CRect.h"

#include <cu/cu_stl.h>
#include <set>

namespace pimg
{

class PixelCoveredLUT;

class CRectPostProcessor
{
public:
	CRectPostProcessor(const CU_VEC<CRect>& rects,
		int width, int height, bool* ori_pixels);
	~CRectPostProcessor();

	//////////////////////////////////////////////////////////////////////////

	void Condense();
	void Condense(const CRect& r, PixelCoveredLUT* covered = NULL);

	void RemoveItem(const CRect& r);
	void InsertItem(const CRect& r);

	//////////////////////////////////////////////////////////////////////////

	void MoveToNoCover();

	void RemoveUnnecessary();

	bool Merge();

	void Align();

	void Reduce();

	void LoadResult(CU_VEC<CRect>& rects) const;

private:
	struct Item;
	enum Direction
	{
		e_left,
		e_right,
		e_down,
		e_up
	};

	void LoadPixels(const CU_VEC<CRect>& rects, bool* ori_pixels);

	void UpdateFreedom(Item* item);
	bool IsNoCoverdSide(Item* item, Direction dir) const;

	bool IsAlignBetter(Item* item, Direction dir) const;

	bool MoveItem(Item* item, Direction dir);

	void MergeCRect(Item* remove, Item* newone);

//	bool PixelHasData(int x, int y) const;
	bool IsPixelCovered(int x, int y) const;
	bool IsPixelImmoveable(int x, int y) const;

	int GetItemDataSize(Item* item) const;

	void CondenseEmpty();
	void CondenseEmpty(const CRect& r, PixelCoveredLUT* covered = NULL);
	CRect GetRealDataCRect(const CRect& src) const;

	void CondenseCovered(const CRect& r, PixelCoveredLUT* covered = NULL);
	bool CondenseCovered(Item* s, Item* l);

	void RemovePixelItem(Item* item, const CRect& r);
	void InsertPixelItem(Item* item, const CRect& r);

private:
	struct Item
	{
		Item(const CRect& r) : r(r) {
			to_left = to_right = to_down = to_up = true;
		}

		CRect r;

		bool to_left, to_right, to_down, to_up;

	}; // Item

	class ItemCmp
	{
	public:
		bool operator () (const Item* lhs, const Item* rhs) const {
			return lhs->r.h * lhs->r.w < rhs->r.h * rhs->r.w;
		}
	}; // ItemCmp

	class Pixel
	{
	public:
		Pixel(bool has_data) : m_has_data(has_data) {}

		void Add(Item* item) {
			m_items.insert(item);
		}
		void Remove(Item* item) {
			m_items.erase(item);
		}
		bool Find(Item* item) const {
			return m_items.find(item) != m_items.end();
		}
		bool IsMultiCoverd() const {
			return m_items.size() > 1;
		}
		bool IsSingleCoverd() const {
			return m_items.size() == 1;
		}
		bool IsEmpty() const {
			return m_items.empty();
		}

		bool HasData() const {
			return m_has_data;
		}

		Item* FindSameEdgeCRect(int x, int y, int w, int h) const;
		Item* FindCRect(const CRect& r) const;

	public:
		std::set<Item*> m_items;

		bool m_has_data;

	}; // Pixel

private:
	int m_width, m_height;

	std::multiset<Item*, ItemCmp> m_items;
	Pixel** m_pixels;

}; // CRectManager

}

#endif // _EASYIMAGE_RECT_MANAGER_H_