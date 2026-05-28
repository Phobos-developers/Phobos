#include "OwnerDraw.Internal.h"

static COLORREF ListBoxTextColor()
{
	return Phobos::UI::ColorTextList;
}

static COLORREF ListBoxSelectionFillColor()
{
	return Phobos::UI::ColorSelectionList;
}

static COLORREF ListBoxDisabledTextColor()
{
	return Phobos::UI::ColorDisabledList;
}

constexpr int ListBoxScrollBarExtraWidth = 18;
constexpr int ListBoxTextEntryInlineBytes = 2;

static int SignedLowWord(LPARAM value)
{
	return static_cast<short>(LOWORD(value));
}

static int SignedHighWord(LPARAM value)
{
	return static_cast<short>(HIWORD(value));
}

static WPARAM ListBoxCommand(HWND hWnd, int notificationCode)
{
	return static_cast<WPARAM>(
		(::GetWindowLongA(hWnd, GWL_ID) & 0xFFFF)
		| ((notificationCode & 0xFFFF) << 16));
}

static void NotifyListBoxSelectionChanged(HWND hWnd)
{
	if (const HWND parentHwnd = ::GetParent(hWnd))
		::SendMessageA(parentHwnd, WM_COMMAND, ListBoxCommand(hWnd, LBN_SELCHANGE), reinterpret_cast<LPARAM>(hWnd));
}

static void PostListBoxDoubleClick(HWND hWnd)
{
	if (const HWND parentHwnd = ::GetParent(hWnd))
		::PostMessageA(parentHwnd, WM_COMMAND, ListBoxCommand(hWnd, LBN_DBLCLK), reinterpret_cast<LPARAM>(hWnd));
}

static WWUIIntArray* CreateIntArray()
{
	auto pArray = static_cast<WWUIIntArray*>(YRMemory::Allocate(sizeof(WWUIIntArray)));
	if (pArray)
		*pArray = {};

	return pArray;
}

static void DeleteIntArray(WWUIIntArray*& pArray)
{
	if (!pArray)
		return;

	YRMemory::Deallocate(pArray->Items);
	YRMemory::Deallocate(pArray);
	pArray = nullptr;
}

static void ResizeIntArrayStorage(WWUIIntArray& array, int capacity)
{
	if (capacity < 10)
		capacity = 10;

	auto pItems = static_cast<int*>(YRMemory::Allocate(sizeof(int) * capacity));
	std::memset(pItems, 0, sizeof(int) * capacity);

	if (array.Items && array.Count > 0)
		std::memcpy(pItems, array.Items, sizeof(int) * array.Count);

	YRMemory::Deallocate(array.Items);
	array.Items = pItems;
	array.Capacity = capacity;
}

static void EnsureIntArraySize(WWUIIntArray& array, int count, int fillValue)
{
	if (count <= array.Count)
		return;

	if (count > array.Capacity)
	{
		int capacity = array.Capacity;
		do
		{
			capacity = std::max(capacity * 2, 10);
		}
		while (capacity < count);

		ResizeIntArrayStorage(array, capacity);
	}

	for (int i = array.Count; i < count; ++i)
		array.Items[i] = fillValue;

	array.Count = count;
}

static void MaybeShrinkIntArray(WWUIIntArray& array)
{
	if (array.Capacity <= 10 || array.Count * 3 > array.Capacity)
		return;

	ResizeIntArrayStorage(array, std::max(array.Capacity / 2, 10));
}

static void RemoveIntArrayItem(WWUIIntArray* pArray, int index)
{
	if (!pArray || index < 0 || index >= pArray->Count)
		return;

	if (index < pArray->Count - 1)
	{
		std::memmove(
			&pArray->Items[index],
			&pArray->Items[index + 1],
			sizeof(int) * (pArray->Count - index - 1));
	}

	--pArray->Count;
	MaybeShrinkIntArray(*pArray);
}

static void InsertIntArrayItem(WWUIIntArray* pArray, int index, int value)
{
	if (!pArray)
		return;

	index = std::clamp(index, 0, pArray->Count);
	EnsureIntArraySize(*pArray, pArray->Count + 1, value);

	if (index < pArray->Count - 1)
	{
		std::memmove(
			&pArray->Items[index + 1],
			&pArray->Items[index],
			sizeof(int) * (pArray->Count - index - 1));
	}

	pArray->Items[index] = value;
}

static void SetIntArrayValue(WWUIIntArray*& pArray, int index, int value, int fillValue)
{
	if (index < 0)
		return;

	if (!pArray)
		pArray = CreateIntArray();

	if (!pArray)
		return;

	EnsureIntArraySize(*pArray, index + 1, fillValue);
	pArray->Items[index] = value;
}

static int GetIntArrayValue(const WWUIIntArray* pArray, int index, int defaultValue)
{
	if (!pArray || index < 0 || index >= pArray->Count)
		return defaultValue;

	return pArray->Items[index];
}

static void ConstructListBoxCell(WWUIListBoxCell& cell)
{
	new (&cell) WWUIListBoxCell();
	cell.Format = WWUIListBoxCellFormat::Empty;
	cell.TextColor = static_cast<COLORREF>(-1);
	cell.Image = nullptr;
	cell.Value = -1;
}

static void ResetListBoxCell(WWUIListBoxCell& cell)
{
	cell.~WWUIListBoxCell();
	ConstructListBoxCell(cell);
}

static WWUIListBoxCell* AllocateListBoxCells(int capacity)
{
	auto pItems = static_cast<WWUIListBoxCell*>(YRMemory::Allocate(sizeof(WWUIListBoxCell) * capacity));
	for (int i = 0; i < capacity; ++i)
		ConstructListBoxCell(pItems[i]);

	return pItems;
}

static void DeleteListBoxCells(WWUIListBoxCell*& pItems, int capacity)
{
	if (!pItems)
		return;

	for (int i = 0; i < capacity; ++i)
		pItems[i].~WWUIListBoxCell();

	YRMemory::Deallocate(pItems);
	pItems = nullptr;
}

static void ResizeListBoxCellStorage(WWUIListBoxColumn& column, int capacity)
{
	if (capacity < 10)
		capacity = 10;

	auto pItems = AllocateListBoxCells(capacity);
	for (int i = 0; i < column.CellCount; ++i)
		pItems[i] = column.Cells[i];

	DeleteListBoxCells(column.Cells, column.CellCapacity);
	column.Cells = pItems;
	column.CellCapacity = capacity;
}

static void EnsureListBoxCellCount(WWUIListBoxColumn& column, int count, WWUIListBoxCellFormat defaultFormat)
{
	if (count <= column.CellCount)
		return;

	if (count > column.CellCapacity)
	{
		int capacity = column.CellCapacity;
		do
		{
			capacity = std::max(capacity * 2, 10);
		}
		while (capacity < count);

		ResizeListBoxCellStorage(column, capacity);
	}

	for (int i = column.CellCount; i < count; ++i)
	{
		auto& cell = column.Cells[i];
		ResetListBoxCell(cell);
		cell.Format = defaultFormat;
	}

	column.CellCount = count;
}

static void ClearListBoxColumnCells(WWUIListBoxColumn& column, bool releaseStorage)
{
	if (!column.Cells)
	{
		column.CellCount = 0;
		column.CellCapacity = 0;
		return;
	}

	for (int i = 0; i < column.CellCapacity; ++i)
		ResetListBoxCell(column.Cells[i]);

	column.CellCount = 0;

	if (releaseStorage)
	{
		DeleteListBoxCells(column.Cells, column.CellCapacity);
		column.CellCapacity = 0;
	}
}

static void DeleteListBoxColumns(WWUIListBoxColumnArray*& pColumns)
{
	if (!pColumns)
		return;

	for (int i = 0; i < pColumns->Count; ++i)
		ClearListBoxColumnCells(pColumns->Items[i], true);

	YRMemory::Deallocate(pColumns->Items);
	YRMemory::Deallocate(pColumns);
	pColumns = nullptr;
}

static WWUIListBoxColumnArray* CreateListBoxColumnArray()
{
	auto pArray = static_cast<WWUIListBoxColumnArray*>(YRMemory::Allocate(sizeof(WWUIListBoxColumnArray)));
	if (pArray)
		*pArray = {};

	return pArray;
}

static void ResizeListBoxColumnStorage(WWUIListBoxColumnArray& columns, int capacity)
{
	if (capacity < 10)
		capacity = 10;

	auto pItems = static_cast<WWUIListBoxColumn*>(YRMemory::Allocate(sizeof(WWUIListBoxColumn) * capacity));
	std::memset(pItems, 0, sizeof(WWUIListBoxColumn) * capacity);

	if (columns.Items && columns.Count > 0)
		std::memcpy(pItems, columns.Items, sizeof(WWUIListBoxColumn) * columns.Count);

	YRMemory::Deallocate(columns.Items);
	columns.Items = pItems;
	columns.Capacity = capacity;
}

static WWUIListBoxColumn* FindListBoxColumn(WWUIListBoxColumnArray* pColumns, int x)
{
	if (!pColumns)
		return nullptr;

	for (int i = 0; i < pColumns->Count; ++i)
	{
		if (pColumns->Items[i].X == x)
			return &pColumns->Items[i];
	}

	return nullptr;
}

static WWUIListBoxColumn* FindListBoxColumnAtX(WWUIListBoxColumnArray* pColumns, int x)
{
	if (!pColumns)
		return nullptr;

	WWUIListBoxColumn* pBest = nullptr;
	for (int i = 0; i < pColumns->Count; ++i)
	{
		auto& column = pColumns->Items[i];
		if (column.X <= x && (!pBest || column.X > pBest->X))
			pBest = &column;
	}

	return pBest;
}

static WWUIListBoxTextEntry* AllocateListBoxTextEntry(OwnerDrawDialogElement& data, const wchar_t* pText, bool isWide)
{
	if (!pText)
		pText = L"";

	const size_t length = std::wcslen(pText);
	const size_t bytes = sizeof(WWUIListBoxTextEntry) + (length + 1) * sizeof(wchar_t) + ListBoxTextEntryInlineBytes;
	auto pEntry = static_cast<WWUIListBoxTextEntry*>(YRMemory::Allocate(bytes));
	if (!pEntry)
		return nullptr;

	pEntry->Next = data.AsListBox().TextEntries();
	pEntry->ItemData = 0;
	pEntry->Text = reinterpret_cast<wchar_t*>(reinterpret_cast<char*>(pEntry) + sizeof(WWUIListBoxTextEntry));
	pEntry->IsWide = isWide ? 1 : 0;
	std::wcscpy(pEntry->Text, pText);
	data.AsListBox().TextEntries() = pEntry;
	return pEntry;
}

static void RemoveListBoxTextEntry(OwnerDrawDialogElement& data, WWUIListBoxTextEntry* pEntry)
{
	if (!pEntry)
		return;

	WWUIListBoxTextEntry* pPrevious = nullptr;
	for (auto pCurrent = data.AsListBox().TextEntries(); pCurrent; pCurrent = pCurrent->Next)
	{
		if (pCurrent != pEntry)
		{
			pPrevious = pCurrent;
			continue;
		}

		if (pPrevious)
			pPrevious->Next = pCurrent->Next;
		else
			data.AsListBox().TextEntries() = pCurrent->Next;

		YRMemory::Deallocate(pCurrent);
		return;
	}
}

static void ClearListBoxTextEntries(OwnerDrawDialogElement& data)
{
	auto pEntry = data.AsListBox().TextEntries();
	while (pEntry)
	{
		auto pNext = pEntry->Next;
		YRMemory::Deallocate(pEntry);
		pEntry = pNext;
	}

	data.AsListBox().TextEntries() = nullptr;
}

static WWUIListBoxTextEntry* GetListBoxTextEntry(WNDPROC pOriginalWndProc, HWND hWnd, int index)
{
	const auto result = CallSelectedHandler(pOriginalWndProc, hWnd, LB_GETITEMDATA, index, 0);
	if (result == LB_ERR || !result)
		return nullptr;

	return reinterpret_cast<WWUIListBoxTextEntry*>(result);
}

static void RemoveListBoxRow(OwnerDrawDialogElement& data, int index)
{
	RemoveIntArrayItem(data.AsListBox().ItemData(), index);
	RemoveIntArrayItem(data.AsListBox().SelectionStates(), index);

	if (auto pColumns = data.AsListBox().Columns())
	{
		for (int i = 0; i < pColumns->Count; ++i)
		{
			auto& column = pColumns->Items[i];
			if (index < 0 || index >= column.CellCount)
				continue;

			for (int cellIndex = index; cellIndex < column.CellCount - 1; ++cellIndex)
				column.Cells[cellIndex] = column.Cells[cellIndex + 1];

			ResetListBoxCell(column.Cells[column.CellCount - 1]);
			--column.CellCount;

			if (column.CellCapacity > 10 && column.CellCount * 3 <= column.CellCapacity)
				ResizeListBoxCellStorage(column, std::max(column.CellCapacity / 2, 10));
		}
	}
}

static void InsertListBoxRow(OwnerDrawDialogElement& data, int index)
{
	InsertIntArrayItem(data.AsListBox().ItemData(), index, -1);
	InsertIntArrayItem(data.AsListBox().SelectionStates(), index, 0);

	if (auto pColumns = data.AsListBox().Columns())
	{
		for (int i = 0; i < pColumns->Count; ++i)
		{
			auto& column = pColumns->Items[i];
			const int insertIndex = std::clamp(index, 0, column.CellCount);
			EnsureListBoxCellCount(
				column,
				column.CellCount + 1,
				i == 0 ? WWUIListBoxCellFormat::ItemText : WWUIListBoxCellFormat::Empty);

			for (int cellIndex = column.CellCount - 1; cellIndex > insertIndex; --cellIndex)
				column.Cells[cellIndex] = column.Cells[cellIndex - 1];

			ResetListBoxCell(column.Cells[insertIndex]);
			column.Cells[insertIndex].Format = i == 0 ? WWUIListBoxCellFormat::ItemText : WWUIListBoxCellFormat::Empty;
		}
	}
}

static void ClearListBoxRows(OwnerDrawDialogElement& data, bool destroyColumns)
{
	DeleteIntArray(data.AsListBox().ItemData());
	DeleteIntArray(data.AsListBox().SelectionStates());
	data.AsListBox().TopIndex() = 0;
	data.AsListBox().CurrentSelection() = -1;

	if (destroyColumns)
	{
		DeleteListBoxColumns(data.AsListBox().Columns());
	}
	else if (auto pColumns = data.AsListBox().Columns())
	{
		for (int i = 0; i < pColumns->Count; ++i)
			ClearListBoxColumnCells(pColumns->Items[i], false);
	}
}

static void PaintListBoxCellText(
	HWND hWnd,
	BitFont* pFont,
	RectangleStruct rect,
	const wchar_t* pText,
	COLORREF textColor,
	int maxWidth)
{
	if (!pText)
		pText = L"";

	wchar_t buffer[512] {};
	std::wcsncpy(buffer, pText, std::size(buffer) - 1);

	int textWidth = 0;
	int textHeight = 0;
	auto pMeasureFont = pFont ? pFont : BitFont::Instance;

	if (pMeasureFont)
	{
		size_t length = std::wcslen(buffer);
		while (length > 0)
		{
			pMeasureFont->GetTextDimension(buffer, &textWidth, &textHeight, maxWidth);
			if (textWidth <= maxWidth)
				break;

			--length;
			buffer[length] = L'\0';
			if (length > 3)
				std::wcscat(buffer, L"...");
		}
	}

	OwnerDraw::PrintTextFixedLength(
		textColor,
		pFont,
		&rect,
		buffer,
		static_cast<int>(std::wcslen(buffer)),
		0,
		0,
		nullptr,
		0);
}

static void DrawListBoxProgressCell(RectangleStruct rect, int value)
{
	if (!DSurface::Alternate)
		return;

	WORD color = static_cast<WORD>(ConvertRGBToSurfaceColor(RGB(0, 0, 192)));
	if (value < 0)
	{
		color = static_cast<WORD>(ConvertRGBToSurfaceColor(RGB(0, 0, 192)));
		value = 1000;
	}
	else if (value < 300)
	{
		color = static_cast<WORD>(ConvertRGBToSurfaceColor(RGB(0, 192, 0)));
	}
	else if (value < 500)
	{
		color = static_cast<WORD>(ConvertRGBToSurfaceColor(RGB(192, 192, 0)));
	}
	else
	{
		color = static_cast<WORD>(ConvertRGBToSurfaceColor(RGB(192, 0, 0)));
	}

	BlendGradientRect(rect, DSurface::Alternate, color, (value << 16) / 1000);
}

static void PaintListBox(HWND hWnd, OwnerDrawDialogElement& data, const RECT& clientRect, const RECT& ownerRect, WNDPROC pOriginalWndProc)
{
	if (data.SkipDraw)
	{
		::ValidateRect(hWnd, nullptr);
		return;
	}

	if (!DSurface::Alternate)
		return;

	RectangleStruct drawRect
	{
		ownerRect.left,
		ownerRect.top,
		clientRect.right - clientRect.left,
		clientRect.bottom - clientRect.top
	};

	OwnerDraw::CopyDimmedBackground(&drawRect, hWnd, static_cast<unsigned char>(data.Alpha));
	DrawBeveledBorder(DSurface::Alternate, drawRect, 2, -1);

	RECT updateRect {};
	if (!::GetUpdateRect(hWnd, &updateRect, FALSE))
		return;

	const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
	int itemIndex = static_cast<int>(::SendMessageA(hWnd, LB_GETTOPINDEX, 0, 0));
	const LONG style = ::GetWindowLongA(hWnd, GWL_STYLE);
	const int selectionColor = ConvertRGBToSurfaceColor(ListBoxSelectionFillColor());
	auto pFont = data.AsListBox().Font();

	while (itemIndex >= 0 && itemIndex < itemCount)
	{
		RECT itemRect {};
		if (::SendMessageA(hWnd, LB_GETITEMRECT, itemIndex, reinterpret_cast<LPARAM>(&itemRect)) == LB_ERR)
			break;

		if (clientRect.top + itemRect.bottom > clientRect.bottom)
			break;

		wchar_t itemText[512] {};
		if (auto pEntry = GetListBoxTextEntry(pOriginalWndProc, hWnd, itemIndex))
			std::wcsncpy(itemText, pEntry->Text ? pEntry->Text : L"", std::size(itemText) - 1);

		const bool selected = ::SendMessageA(hWnd, LB_GETSEL, itemIndex, 0) > 0;
		RectangleStruct rowRect
		{
			ownerRect.left + itemRect.left,
			ownerRect.top + itemRect.top,
			itemRect.right - itemRect.left,
			itemRect.bottom - itemRect.top
		};

		if (selected)
			DSurface::Alternate->FillRect(&rowRect, selectionColor);

		if (auto pColumns = data.AsListBox().Columns())
		{
			for (int columnIndex = 0; columnIndex < pColumns->Count; ++columnIndex)
			{
				auto& column = pColumns->Items[columnIndex];
				if (itemIndex < 0 || itemIndex >= column.CellCount || !column.Cells)
					continue;

				auto& cell = column.Cells[itemIndex];
				if (cell.Format == WWUIListBoxCellFormat::Empty)
					continue;

				RectangleStruct cellRect
				{
					ownerRect.left + itemRect.left + column.X,
					ownerRect.top + itemRect.top,
					column.Width ? column.Width : rowRect.Width,
					rowRect.Height
				};

				if (cellRect.Width <= 0)
					cellRect.Width = 0xFFFF;

				const int availableWidth = std::min(
					cellRect.Width,
					static_cast<int>(ownerRect.right - cellRect.X));

				switch (cell.Format)
				{
				case WWUIListBoxCellFormat::Text:
				case WWUIListBoxCellFormat::ItemText:
				{
					COLORREF textColor = cell.TextColor == static_cast<COLORREF>(-1)
						? ListBoxTextColor()
						: cell.TextColor;

					if (style & WS_DISABLED)
						textColor = ListBoxDisabledTextColor();

					const wchar_t* pText = cell.Format == WWUIListBoxCellFormat::Text
						? GetWideTextBuffer(cell.PrimaryText)
						: itemText;

					PaintListBoxCellText(hWnd, pFont, cellRect, pText, textColor, availableWidth);
					break;
				}

				case WWUIListBoxCellFormat::Image:
					if (cell.Image)
					{
						RectangleStruct imageRect
						{
							cellRect.X,
							cellRect.Y + (cellRect.Height - cell.Image->GetHeight()) / 2,
							cell.Image->GetWidth(),
							cell.Image->GetHeight()
						};
						PCX::Instance.BlitToSurface(&imageRect, DSurface::Alternate, static_cast<BSurface*>(cell.Image));
					}
					break;

				case WWUIListBoxCellFormat::Progress:
				{
					RectangleStruct progressRect { cellRect.X, cellRect.Y, 32, 12 };
					DrawListBoxProgressCell(progressRect, cell.Value);
					break;
				}

				default:
					break;
				}
			}
		}
		else
		{
			COLORREF textColor = GetIntArrayValue(data.AsListBox().ItemData(), itemIndex, ListBoxTextColor());
			if (textColor == static_cast<COLORREF>(-1))
				textColor = ListBoxTextColor();

			if (style & WS_DISABLED)
				textColor = ListBoxDisabledTextColor();

			RectangleStruct textRect
			{
				rowRect.X + 2,
				rowRect.Y,
				rowRect.Width,
				rowRect.Height
			};

			OwnerDraw::PrintTextFixedLength(
				textColor,
				pFont,
				&textRect,
				itemText,
				static_cast<int>(std::wcslen(itemText)),
				0,
				0,
				nullptr,
				0);
		}

		++itemIndex;
	}

	::ValidateRect(hWnd, &updateRect);
	if (data.AsListBox().ScrollBarHwnd())
		::InvalidateRect(data.AsListBox().ScrollBarHwnd(), nullptr, FALSE);
}

static void SyncListBoxScrollBar(HWND hWnd, OwnerDrawDialogElement& data, const RECT& clientRect, int itemCount, int itemHeight)
{
	if (itemHeight <= 0)
		itemHeight = 1;

	const int visibleItems = itemHeight ? (clientRect.bottom - clientRect.top) / itemHeight : 0;
	int maxTopIndex = itemCount - visibleItems;
	if (maxTopIndex < 0)
		maxTopIndex = 0;

	if (data.AsListBox().TopIndex() > maxTopIndex)
		data.AsListBox().TopIndex() = maxTopIndex;

	if (data.AsListBox().ScrollBarHwnd() && reinterpret_cast<intptr_t>(data.AsListBox().ScrollBarHwnd()) > 1)
	{
		SCROLLINFO scrollInfo {};
		scrollInfo.cbSize = sizeof(scrollInfo);
		scrollInfo.fMask = SIF_RANGE | SIF_POS;
		scrollInfo.nMin = 0;
		scrollInfo.nMax = maxTopIndex;
		scrollInfo.nPos = data.AsListBox().TopIndex();
		::SendMessageA(data.AsListBox().ScrollBarHwnd(), SBM_SETSCROLLINFO, 0, reinterpret_cast<LPARAM>(&scrollInfo));
	}
}

static bool HasListBoxScrollBar(OwnerDrawDialogElement& data)
{
	const HWND scrollBarHwnd = data.AsListBox().ScrollBarHwnd();
	return scrollBarHwnd && reinterpret_cast<intptr_t>(scrollBarHwnd) > 1 && ::IsWindow(scrollBarHwnd);
}

static void PositionListBoxScrollBar(HWND hWnd, OwnerDrawDialogElement& data, BOOL repaint)
{
	if (!HasListBoxScrollBar(data))
		return;

	const HWND parentHwnd = ::GetParent(hWnd);
	if (!parentHwnd)
		return;

	RECT parentRect {};
	RECT listRect {};
	if (!OwnerDraw::GetRectangle(parentHwnd, &parentRect) || !OwnerDraw::GetRectangle(hWnd, &listRect))
		return;

	const int inset = OwnerDraw::ControlInsetPx;
	const int scrollBarWidth = 2 * inset + ListBoxScrollBarExtraWidth;
	const int x = listRect.right - parentRect.left - 2 * inset + 1;
	const int y = listRect.top - parentRect.top;
	const int height = listRect.bottom - listRect.top;

	if (height <= 0)
		return;

	const HWND scrollBarHwnd = data.AsListBox().ScrollBarHwnd();
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
	{
		::MoveWindow(scrollBarHwnd, x, y, scrollBarWidth, height, repaint);
	}
	else
	{
		RenderDX::MoveWindowInRender(scrollBarHwnd, x, y, scrollBarWidth, height, repaint);
	}

	if (auto pScrollData = FindOwnerDrawData(scrollBarHwnd))
		ResetOwnerDrawCachedSurface(*pScrollData);

	::InvalidateRect(scrollBarHwnd, nullptr, FALSE);
}

void WWUI::SyncListBoxScrollBarPositions(HWND rootHwnd)
{
	if (!rootHwnd)
	{
		for (auto it = OwnerDraw::Dialogs.begin(); it != OwnerDraw::Dialogs.end(); ++it)
		{
			const HWND hWnd = it->Key;
			auto& data = it->Value;
			if (::IsWindow(hWnd) && data.ControlType == WWControlType::ListBox)
				PositionListBoxScrollBar(hWnd, data, FALSE);
		}

		return;
	}

	if (auto pData = FindOwnerDrawData(rootHwnd))
	{
		if (pData->ControlType == WWControlType::ListBox)
			PositionListBoxScrollBar(rootHwnd, *pData, FALSE);
	}

	for (HWND child = ::GetWindow(rootHwnd, GW_CHILD); child; child = ::GetWindow(child, GW_HWNDNEXT))
		SyncListBoxScrollBarPositions(child);
}

static void UpdateListBoxScrollBar(HWND hWnd, OwnerDrawDialogElement& data, const RECT& clientRect)
{
	const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
	const int itemHeight = std::max(static_cast<int>(::SendMessageA(hWnd, LB_GETITEMHEIGHT, 0, 0)), 1);
	const bool needsScrollbar = itemCount * itemHeight > clientRect.bottom - clientRect.top;
	const int scrollBarWidth = 2 * OwnerDraw::ControlInsetPx + ListBoxScrollBarExtraWidth;

	SyncListBoxScrollBar(hWnd, data, clientRect, itemCount, itemHeight);

	if (needsScrollbar)
	{
		if (!data.AsListBox().ScrollBarHwnd())
		{
			data.AsListBox().ScrollBarHwnd() = reinterpret_cast<HWND>(1);

			const HWND parentHwnd = ::GetParent(hWnd);
			RECT parentRect {};
			RECT listRect {};
			OwnerDraw::GetRectangle(parentHwnd, &parentRect);
			OwnerDraw::GetRectangle(hWnd, &listRect);

			const int height = listRect.bottom - listRect.top;
			RECT scrollClientRect {};
			if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
			{
				scrollClientRect.left = listRect.left - parentRect.left - scrollBarWidth + clientRect.right + 1;
				scrollClientRect.top = listRect.top - parentRect.top + clientRect.top;
				scrollClientRect.right = scrollClientRect.left + scrollBarWidth;
				scrollClientRect.bottom = scrollClientRect.top + height;
			}
			else
			{
				const RECT scrollRenderRect
				{
					listRect.left - scrollBarWidth + clientRect.right + 1,
					listRect.top + clientRect.top,
					listRect.left + clientRect.right + 1,
					listRect.top + clientRect.top + height
				};

				if (!RenderDX::RenderRectToClient(parentHwnd, scrollRenderRect, &scrollClientRect))
				{
					data.AsListBox().ScrollBarHwnd() = nullptr;
					return;
				}
			}

			data.AsListBox().ScrollBarHwnd() = ::CreateWindowExA(
				0,
				"Scrollbar",
				nullptr,
				WS_CHILD | WS_VISIBLE | SBS_VERT | WS_TABSTOP,
				scrollClientRect.left,
				scrollClientRect.top,
				scrollClientRect.right - scrollClientRect.left,
				scrollClientRect.bottom - scrollClientRect.top,
				parentHwnd,
				nullptr,
				reinterpret_cast<HINSTANCE>(Phobos::hInstance),
				nullptr);

			data.AsListBox().ScrollBarWidth() = scrollBarWidth;
			OwnerDraw::RegisterChildControlProc(data.AsListBox().ScrollBarHwnd(), 0);

			if (auto pScrollData = FindOwnerDrawData(data.AsListBox().ScrollBarHwnd()))
			{
				pScrollData->AsScrollBar().NotifyHwnd() = hWnd;
				pScrollData->AsScrollBar().Disabled() = false;
			}

			SyncListBoxScrollBar(hWnd, data, clientRect, itemCount, itemHeight);

			if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
			{
				::SetWindowPos(
					hWnd,
					nullptr,
					0,
					0,
					listRect.right - listRect.left - scrollBarWidth,
					listRect.bottom - listRect.top,
					SWP_NOMOVE | SWP_NOZORDER);
			}
			else
			{
				RenderDX::SetWindowPosInRender(
					hWnd,
					nullptr,
					0,
					0,
					listRect.right - listRect.left - scrollBarWidth,
					listRect.bottom - listRect.top,
					SWP_NOMOVE | SWP_NOZORDER);
			}

			::ShowWindow(data.AsListBox().ScrollBarHwnd(), SW_SHOW);
			::BringWindowToTop(data.AsListBox().ScrollBarHwnd());
			PositionListBoxScrollBar(hWnd, data, FALSE);
			::InvalidateRect(data.AsListBox().ScrollBarHwnd(), nullptr, FALSE);
			::UpdateWindow(data.AsListBox().ScrollBarHwnd());
		}

		return;
	}

	if (!data.AsListBox().ScrollBarHwnd() || data.NeedsControlImage)
		return;

	const HWND scrollBarHwnd = data.AsListBox().ScrollBarHwnd();
	::DestroyWindow(scrollBarHwnd);
	CleanupDestroyedWindow(scrollBarHwnd);
	data.AsListBox().ScrollBarHwnd() = nullptr;

	RECT listRect {};
	OwnerDraw::GetRectangle(hWnd, &listRect);
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates())
	{
		::SetWindowPos(
			hWnd,
			nullptr,
			0,
			0,
			listRect.right - listRect.left + scrollBarWidth,
			listRect.bottom - listRect.top,
			SWP_NOMOVE | SWP_NOZORDER);
	}
	else
	{
		RenderDX::SetWindowPosInRender(
			hWnd,
			nullptr,
			0,
			0,
			listRect.right - listRect.left + scrollBarWidth,
			listRect.bottom - listRect.top,
			SWP_NOMOVE | SWP_NOZORDER);
	}

	data.AsListBox().ScrollBarWidth() = 0;
}

LRESULT CALLBACK WWUI::ListBoxCtrl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto pData = FindOwnerDrawData(hWnd);
	if (!pData)
		return 0;

	auto& data = *pData;
	const auto pOriginalWndProc = FindWindowProc(OwnerDraw::DialogProcs, hWnd);

	RECT clientRect {};
	if (RenderDX::IsOwnerDrawUsingRawWindowCoordinates() || !RenderDX::GetClientRectInRender(hWnd, &clientRect))
		::GetClientRect(hWnd, &clientRect);
	const RECT fullClientRect = clientRect;

	RECT ownerRect {};
	OwnerDraw::GetRectangle(hWnd, &ownerRect);

	const int inset = OwnerDraw::ControlInsetPx;
	clientRect.right -= 2 * inset;
	clientRect.bottom -= 2 * inset;
	ownerRect.left += inset;
	ownerRect.top += inset;
	ownerRect.right -= inset;
	ownerRect.bottom -= inset;

	const bool updateScrollBar = message != LB_GETCOUNT
		&& message != LB_GETITEMHEIGHT
		&& message != WM_VSCROLL;

	if (updateScrollBar)
	{
		const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
		const int itemHeight = std::max(static_cast<int>(::SendMessageA(hWnd, LB_GETITEMHEIGHT, 0, 0)), 1);
		SyncListBoxScrollBar(hWnd, data, clientRect, itemCount, itemHeight);
	}

	auto finish = [&](LRESULT result) -> LRESULT
	{
		if (updateScrollBar)
			UpdateListBoxScrollBar(hWnd, data, clientRect);

		return result;
	};

	auto forwardOriginal = [&]() -> LRESULT
	{
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);
	};

	auto setSelection = [&](int index, int selected)
	{
		SetIntArrayValue(data.AsListBox().SelectionStates(), index, selected ? 1 : 0, 0);
	};

	auto playClick = []()
	{
		if (RulesClass::Instance)
			VocClass::PlayGlobal(RulesClass::Instance->GenericClick, 0x2000, 1.0f);
	};

	auto addOrInsertString = [&](bool wideText, bool insert) -> LRESULT
	{
		char narrowText[2048] {};
		wchar_t wideBuffer[2048] {};

		const LPARAM nativeTextParam = [&]() -> LPARAM
		{
			if (wideText)
			{
				const auto pWideText = reinterpret_cast<const wchar_t*>(lParam);
				std::wcsncpy(wideBuffer, pWideText ? pWideText : L"", std::size(wideBuffer) - 1);
				WideToCharString(narrowText, std::size(narrowText), wideBuffer);
				return reinterpret_cast<LPARAM>(narrowText);
			}

			const auto pText = reinterpret_cast<const char*>(lParam);
			std::strncpy(narrowText, pText ? pText : "", std::size(narrowText) - 1);
			CharToWideString(wideBuffer, std::size(wideBuffer), narrowText);
			return reinterpret_cast<LPARAM>(narrowText);
		}();

		const UINT nativeMessage = insert ? LB_INSERTSTRING : LB_ADDSTRING;
		const WPARAM nativeIndex = insert ? wParam : 0;
		const auto nativeResult = CallSelectedHandler(pOriginalWndProc, hWnd, nativeMessage, nativeIndex, nativeTextParam);
		if (nativeResult == LB_ERR || nativeResult == LB_ERRSPACE)
			return nativeResult;

		const int itemIndex = static_cast<int>(nativeResult);
		auto pEntry = AllocateListBoxTextEntry(data, wideBuffer, wideText);
		if (!pEntry)
		{
			CallSelectedHandler(pOriginalWndProc, hWnd, LB_DELETESTRING, itemIndex, 0);
			return LB_ERRSPACE;
		}

		const auto setDataResult = CallSelectedHandler(
			pOriginalWndProc,
			hWnd,
			LB_SETITEMDATA,
			itemIndex,
			reinterpret_cast<LPARAM>(pEntry));

		if (setDataResult == LB_ERR)
		{
			RemoveListBoxTextEntry(data, pEntry);
			CallSelectedHandler(pOriginalWndProc, hWnd, LB_DELETESTRING, itemIndex, 0);
			return LB_ERR;
		}

		InsertListBoxRow(data, itemIndex);
		return itemIndex;
	};

	auto findString = [&](bool wideText, bool exact, bool select) -> LRESULT
	{
		wchar_t needle[2048] {};
		if (wideText)
		{
			const auto pText = reinterpret_cast<const wchar_t*>(lParam);
			std::wcsncpy(needle, pText ? pText : L"", std::size(needle) - 1);
		}
		else
		{
			CharToWideString(needle, std::size(needle), reinterpret_cast<const char*>(lParam));
		}

		const int count = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
		int index = static_cast<int>(wParam);
		if (index < 0)
			index = 0;

		if (index >= count)
			return LB_ERR;

		const size_t needleLength = std::wcslen(needle);
		for (; index < count; ++index)
		{
			const auto pEntry = GetListBoxTextEntry(pOriginalWndProc, hWnd, index);
			const wchar_t* pText = pEntry && pEntry->Text ? pEntry->Text : L"";
			const bool match = exact
				? _wcsicmp(needle, pText) == 0
				: _wcsnicmp(needle, pText, needleLength) == 0;

			if (!match)
				continue;

			if (select)
				::SendMessageA(hWnd, LB_SETCURSEL, index, 0);

			return index;
		}

		return LB_ERR;
	};

	switch (message)
	{
	case WM_SIZE:
		PositionListBoxScrollBar(hWnd, data, TRUE);

		if (data.CacheSurface
			&& (fullClientRect.right != data.CacheSurface->GetWidth() || fullClientRect.bottom != data.CacheSurface->GetHeight()))
		{
			ResetOwnerDrawCachedSurface(data);
		}

		return finish(forwardOriginal());

	case WM_PAINT:
		PaintListBox(hWnd, data, clientRect, ownerRect, pOriginalWndProc);
		return finish(0);

	case WM_ERASEBKGND:
		return 0;

	case WM_DELETEITEM:
		if (lParam)
		{
			const auto pDeleteItem = reinterpret_cast<DELETEITEMSTRUCT*>(lParam);
			RemoveListBoxTextEntry(data, reinterpret_cast<WWUIListBoxTextEntry*>(pDeleteItem->itemData));
		}
		return 1;

	case WM_SETFONT:
	{
		TEXTMETRICA metrics {};
		if (const HDC hdc = ::GetDC(hWnd))
		{
			::GetTextMetricsA(hdc, &metrics);
			::ReleaseDC(hWnd, hdc);
		}

		::SendMessageA(hWnd, LB_SETITEMHEIGHT, static_cast<WPARAM>(-1), LOWORD(metrics.tmHeight + 2));
		data.AsListBox().SavedFont() = static_cast<int>(wParam);
		return 0;
	}

	case WM_VSCROLL:
		if (data.AsListBox().ScrollBarHwnd())
		{
			const auto position = ::SendMessageA(data.AsListBox().ScrollBarHwnd(), SBM_GETPOS, 0, 0);
			if (position != ::SendMessageA(hWnd, LB_GETTOPINDEX, 0, 0))
				::SendMessageA(hWnd, LB_SETTOPINDEX, position, 0);
		}
		return 0;

	case WM_RBUTTONDOWN:
		if (::GetWindowLongA(hWnd, GWL_STYLE) & LBS_MULTIPLESEL)
		{
			if (auto pSelections = data.AsListBox().SelectionStates())
			{
				for (int i = 0; i < pSelections->Count; ++i)
					pSelections->Items[i] = 0;
			}

			data.AsListBox().CurrentSelection() = -1;
			NotifyListBoxSelectionChanged(hWnd);
			::InvalidateRect(hWnd, nullptr, FALSE);
			return 0;
		}
		break;

	case WM_LBUTTONDBLCLK:
		PostListBoxDoubleClick(hWnd);
		return 0;

	case WM_LBUTTONDOWN:
	{
		const int itemHeight = std::max(static_cast<int>(::SendMessageA(hWnd, LB_GETITEMHEIGHT, 0, 0)), 1);
		const POINT point = RenderDX::MouseLParamToRenderLocalPoint(hWnd, lParam);
		if (point.y < 0)
			return 0;

		const int itemIndex = data.AsListBox().TopIndex() + point.y / itemHeight;
		const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
		if (itemIndex < 0 || itemIndex >= itemCount)
			return 0;

		const LONG style = ::GetWindowLongA(hWnd, GWL_STYLE);
		::SetFocus(hWnd);
		const auto previousImageState = ::SendMessageA(hWnd, WW_SETHASIMAGE, 0, 1);

		if (style & LBS_MULTIPLESEL)
		{
			const bool selected = ::SendMessageA(hWnd, LB_GETSEL, itemIndex, 0) == 0;
			playClick();
			::SendMessageA(hWnd, LB_SETSEL, selected, itemIndex);
		}
		else if (!(style & LBS_NOSEL))
		{
			playClick();
			::SendMessageA(hWnd, LB_SETCURSEL, itemIndex, 0);
		}

		::SendMessageA(hWnd, WW_SETHASIMAGE, 0, previousImageState);
		::InvalidateRect(hWnd, nullptr, FALSE);
		NotifyListBoxSelectionChanged(hWnd);
		return 0;
	}

	case LB_SETSEL:
	{
		const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
		int index = static_cast<int>(lParam);
		if (index < -1)
			return LB_ERR;

		if (index >= itemCount)
			index = itemCount - 1;

		if (!data.AsListBox().SelectionStates())
			data.AsListBox().SelectionStates() = CreateIntArray();

		if (index == -1)
		{
			if (data.AsListBox().SelectionStates())
			{
				EnsureIntArraySize(*data.AsListBox().SelectionStates(), itemCount, 0);
				for (int i = 0; i < data.AsListBox().SelectionStates()->Count; ++i)
					data.AsListBox().SelectionStates()->Items[i] = wParam ? 1 : 0;
			}
		}
		else
		{
			setSelection(index, wParam ? 1 : 0);
		}

		NotifyListBoxSelectionChanged(hWnd);
		::InvalidateRect(hWnd, nullptr, FALSE);
		return finish(0);
	}

	case LB_GETSEL:
		return GetIntArrayValue(data.AsListBox().SelectionStates(), static_cast<int>(wParam), 0);

	case LB_GETSELCOUNT:
	{
		int count = 0;
		if (auto pSelections = data.AsListBox().SelectionStates())
		{
			for (int i = 0; i < pSelections->Count; ++i)
			{
				if (pSelections->Items[i])
					++count;
			}
		}
		return count;
	}

	case LB_GETSELITEMS:
	{
		int written = 0;
		auto pOut = reinterpret_cast<int*>(lParam);
		if (pOut)
		{
			if (auto pSelections = data.AsListBox().SelectionStates())
			{
				for (int i = 0; i < pSelections->Count && written < static_cast<int>(wParam); ++i)
				{
					if (pSelections->Items[i])
						pOut[written++] = i;
				}
			}
		}
		return written;
	}

	case LB_SELITEMRANGE:
	{
		int first = SignedLowWord(lParam);
		int last = SignedHighWord(lParam);
		const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
		if (first < 0 || last < first)
			return LB_ERR;

		if (last >= itemCount)
			last = itemCount - 1;

		if (!data.AsListBox().SelectionStates())
			data.AsListBox().SelectionStates() = CreateIntArray();

		if (data.AsListBox().SelectionStates())
		{
			EnsureIntArraySize(*data.AsListBox().SelectionStates(), last + 1, 0);
			for (int i = first; i <= last; ++i)
				data.AsListBox().SelectionStates()->Items[i] = wParam ? 1 : 0;
		}

		NotifyListBoxSelectionChanged(hWnd);
		return finish(0);
	}

	case LB_SETCURSEL:
	{
		int selection = static_cast<int>(wParam);
		const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
		if (selection >= -1 && selection < itemCount)
		{
			if (data.AsListBox().CurrentSelection() != -1)
				setSelection(data.AsListBox().CurrentSelection(), 0);

			data.AsListBox().CurrentSelection() = selection;
			if (selection != -1)
				setSelection(selection, 1);
		}

		NotifyListBoxSelectionChanged(hWnd);
		::InvalidateRect(hWnd, nullptr, FALSE);
		return finish(0);
	}

	case LB_GETCURSEL:
		return data.AsListBox().CurrentSelection();

	case LB_GETTOPINDEX:
		return data.AsListBox().TopIndex();

	case LB_SETTOPINDEX:
	{
		const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
		const int itemHeight = std::max(static_cast<int>(::SendMessageA(hWnd, LB_GETITEMHEIGHT, 0, 0)), 1);
		const int visibleItems = (clientRect.bottom - clientRect.top) / itemHeight;
		int topIndex = static_cast<int>(wParam);
		if (topIndex < 0)
			topIndex = 0;

		if (itemCount - visibleItems <= 0)
			topIndex = 0;
		else if (topIndex > itemCount - visibleItems)
			topIndex = itemCount - visibleItems;

		if (topIndex != data.AsListBox().TopIndex())
		{
			data.AsListBox().TopIndex() = topIndex;
			::InvalidateRect(hWnd, nullptr, FALSE);
		}

		return finish(0);
	}

	case LB_GETITEMRECT:
	{
		const int index = static_cast<int>(wParam);
		const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
		const int itemHeight = std::max(static_cast<int>(::SendMessageA(hWnd, LB_GETITEMHEIGHT, 0, 0)), 1);
		const int visibleIndex = index - data.AsListBox().TopIndex();

		if (index < data.AsListBox().TopIndex() || index >= itemCount || visibleIndex > (clientRect.bottom - clientRect.top) / itemHeight)
			return LB_ERR;

		if (auto pRect = reinterpret_cast<RECT*>(lParam))
		{
			pRect->left = clientRect.left;
			pRect->top = visibleIndex * itemHeight;
			pRect->right = clientRect.right - clientRect.left;
			pRect->bottom = pRect->top + itemHeight;
		}
		return 0;
	}

	case LB_GETITEMDATA:
	case LB_SETITEMDATA:
	case CB_GETITEMDATA:
	case CB_SETITEMDATA:
	{
		const auto pEntry = GetListBoxTextEntry(pOriginalWndProc, hWnd, static_cast<int>(wParam));
		if (!pEntry)
			return LB_ERR;

		if (message == LB_GETITEMDATA || message == CB_GETITEMDATA)
			return pEntry->ItemData;

		pEntry->ItemData = static_cast<int>(lParam);
		return reinterpret_cast<LRESULT>(pEntry);
	}

	case LB_DELETESTRING:
	{
		const int index = static_cast<int>(wParam);
		const auto pEntry = GetListBoxTextEntry(pOriginalWndProc, hWnd, index);
		RemoveListBoxRow(data, index);
		const auto result = CallSelectedHandler(pOriginalWndProc, hWnd, LB_DELETESTRING, wParam, lParam);
		RemoveListBoxTextEntry(data, pEntry);
		return finish(result);
	}

	case LB_RESETCONTENT:
		ClearListBoxRows(data, false);
		ClearListBoxTextEntries(data);
		NotifyListBoxSelectionChanged(hWnd);
		return finish(CallSelectedHandler(pOriginalWndProc, hWnd, LB_RESETCONTENT, wParam, lParam));

	case WM_NCDESTROY:
		ClearListBoxRows(data, true);
		ClearListBoxTextEntries(data);
		return CallSelectedHandler(pOriginalWndProc, hWnd, message, wParam, lParam);

	case LB_GETTEXTLEN:
	case WW_GETTEXTW:
	case WW_GETTEXTA:
	case WW_LB_GETTEXTW:
	case WW_LB_GETTEXTA:
	case WW_LB_GETITEMTEXTFORMAT:
	{
		const auto pEntry = GetListBoxTextEntry(pOriginalWndProc, hWnd, static_cast<int>(wParam));
		if (!pEntry)
			return LB_ERR;

		if (message == WW_LB_GETITEMTEXTFORMAT)
			return pEntry->IsWide;

		const wchar_t* pText = pEntry->Text ? pEntry->Text : L"";
		const auto length = static_cast<LRESULT>(std::wcslen(pText));
		if (message == LB_GETTEXTLEN)
			return length;

		if (lParam)
		{
			if (message == WW_GETTEXTA || message == WW_LB_GETTEXTA)
				WideToCharString(reinterpret_cast<char*>(lParam), static_cast<int>(length + 1), pText);
			else
				std::wcscpy(reinterpret_cast<wchar_t*>(lParam), pText);
		}
		return length;
	}

	case WW_LB_FINDSTRINGA:
		return findString(false, false, false);

	case WW_LB_FINDSTRINGEXACTA:
		return findString(false, true, false);

	case WW_LB_SELECTSTRINGA:
		return findString(false, false, true);

	case WW_LB_FINDSTRINGW:
		return findString(true, false, false);

	case WW_LB_FINDSTRINGEXACTW:
		return findString(true, true, false);

	case WW_LB_SELECTSTRINGW:
		return findString(true, false, true);

	case WW_LB_INSERTSTRINGA:
		return finish(addOrInsertString(false, true));

	case WW_LB_ADDSTRINGA:
		return finish(addOrInsertString(false, false));

	case WW_LB_INSERTSTRINGW:
		return finish(addOrInsertString(true, true));

	case WW_LB_ADDSTRINGW:
		return finish(addOrInsertString(true, false));

	case WW_QUERYTOOLTIPHIT:
	{
		const POINT point = RenderDX::MouseLParamToRenderLocalPoint(hWnd, lParam);
		const int x = point.x;
		const int y = point.y;
		if (x >= 0 && y >= 0 && x < clientRect.right && y < clientRect.bottom)
		{
			const int itemHeight = std::max(static_cast<int>(::SendMessageA(hWnd, LB_GETITEMHEIGHT, 0, 0)), 1);
			const int itemIndex = data.AsListBox().TopIndex() + y / itemHeight;
			const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
			if (itemIndex >= 0 && itemIndex < itemCount)
				return itemIndex;
		}
		return -1;
	}

	case WW_LB_GETSCROLLBARHWND:
		return reinterpret_cast<LRESULT>(data.AsListBox().ScrollBarHwnd());

	case WW_LB_ADDCOLUMN:
	{
		if (!data.AsListBox().Columns())
			data.AsListBox().Columns() = CreateListBoxColumnArray();

		auto pColumns = data.AsListBox().Columns();
		if (!pColumns)
			return -1;

		const int x = static_cast<int>(lParam);
		if (FindListBoxColumn(pColumns, x))
			return x;

		if (pColumns->Count >= pColumns->Capacity)
			ResizeListBoxColumnStorage(*pColumns, std::max(pColumns->Capacity * 2, 10));

		auto& column = pColumns->Items[pColumns->Count++];
		column = {};
		column.X = x;
		column.Width = static_cast<int>(wParam);
		return x;
	}

	case WW_LB_REMOVECOLUMN:
	{
		auto pColumns = data.AsListBox().Columns();
		if (!pColumns)
			return -1;

		const int x = static_cast<int>(lParam);
		for (int i = 0; i < pColumns->Count; ++i)
		{
			if (pColumns->Items[i].X != x)
				continue;

			ClearListBoxColumnCells(pColumns->Items[i], true);
			if (i < pColumns->Count - 1)
			{
				std::memmove(
					&pColumns->Items[i],
					&pColumns->Items[i + 1],
					sizeof(WWUIListBoxColumn) * (pColumns->Count - i - 1));
			}
			--pColumns->Count;
			std::memset(&pColumns->Items[pColumns->Count], 0, sizeof(WWUIListBoxColumn));
			return x;
		}
		return -1;
	}

	case WW_LB_SETCELLTEXT:
	{
		auto pColumns = data.AsListBox().Columns();
		const int columnX = LOWORD(wParam);
		const int rowIndex = HIWORD(wParam);
		auto pColumn = FindListBoxColumn(pColumns, columnX);

		if (!pColumn || rowIndex >= ::SendMessageA(hWnd, LB_GETCOUNT, 0, 0))
			return -1;

		const auto defaultFormat = pColumn == &pColumns->Items[0]
			? WWUIListBoxCellFormat::ItemText
			: WWUIListBoxCellFormat::Empty;

		EnsureListBoxCellCount(*pColumn, rowIndex + 1, defaultFormat);
		auto& target = pColumn->Cells[rowIndex];
		ResetListBoxCell(target);

		if (const auto pSource = reinterpret_cast<const WWUIListBoxCell*>(lParam))
			target = *pSource;

		return columnX;
	}

	case WW_LB_GETCELLTEXT:
	{
		const int itemCount = static_cast<int>(::SendMessageA(hWnd, LB_GETCOUNT, 0, 0));
		const int itemHeight = std::max(static_cast<int>(::SendMessageA(hWnd, LB_GETITEMHEIGHT, 0, 0)), 1);
		const int rowIndex = data.AsListBox().TopIndex() + SignedHighWord(wParam) / itemHeight;
		const int x = SignedLowWord(wParam);
		auto pColumn = FindListBoxColumnAtX(data.AsListBox().Columns(), x);
		if (!pColumn || rowIndex < 0 || rowIndex >= itemCount || rowIndex >= pColumn->CellCount)
			return 0;

		const auto& text = pColumn->Cells[rowIndex].SecondaryText;
		if (lParam)
			std::wcscpy(reinterpret_cast<wchar_t*>(lParam), GetWideTextBuffer(text));

		return IsEmpty(text) ? 1 : 0;
	}

	case WW_INITDIALOG:
	{
		data.AsListBox().CurrentSelection() = -1;

		int fontHeight = 10;
		if (const auto pFont = data.AsListBox().Font() ? data.AsListBox().Font() : BitFont::Instance)
		{
			if (pFont->InternalPTR)
				fontHeight = pFont->InternalPTR->FontHeight;
		}

		::SendMessageA(hWnd, LB_SETITEMHEIGHT, static_cast<WPARAM>(-1), LOWORD(fontHeight + 2));
		return finish(0);
	}

	case WW_SETCOLOR:
		SetIntArrayValue(data.AsListBox().ItemData(), static_cast<int>(wParam), static_cast<int>(lParam), -1);
		::InvalidateRect(hWnd, nullptr, FALSE);
		return finish(0);

	default:
		break;
	}

	return finish(forwardOriginal());
}
