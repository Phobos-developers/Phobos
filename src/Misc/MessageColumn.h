// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.


#pragma once

#include <GadgetClass.h>

#include <Ext/Scenario/Body.h>

#include <string>

// --------------------------------------------------

class MessageToggleClass : public GadgetClass
{
public:
	static constexpr int ButtonSide = 18;
	static constexpr int ButtonIconWidth = 4;
	static constexpr int ButtonHeight = ButtonIconWidth + 2;

	MessageToggleClass() = default;
	MessageToggleClass(int id, int x, int y, int width, int height);

	~MessageToggleClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override;

	void DrawShape() const;

	int ID { 0 };
	bool Hovering { false };
	bool Clicking { false };
};

// --------------------------------------------------

class MessageButtonClass : public MessageToggleClass
{
public:
	static constexpr int HoldInitialDelay = 30;
	static constexpr int HoldTriggerDelay = 5;

	MessageButtonClass() = default;
	MessageButtonClass(int id, int x, int y, int width, int height);

	~MessageButtonClass() = default;

	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override;

	void DrawShape() const;

	int CheckTime { 0 };
};

// --------------------------------------------------

class MessageScrollClass : public GadgetClass
{
public:
	MessageScrollClass() = default;
	MessageScrollClass(int id, int x, int y, int width, int height);

	~MessageScrollClass() = default;

	virtual bool Draw(bool forced) override;
	virtual void OnMouseEnter() override;
	virtual void OnMouseLeave() override;
	virtual bool Clicked(DWORD* pKey, GadgetFlag flags, int x, int y, KeyModifier modifier) override;

	void DrawShape() const;

	bool Hovering { false };
	int ID { 0 };
	int LastY { 0 };
	int LastScroll { 0 };
};

// --------------------------------------------------

class MessageLabelClass : public GadgetClass
{
public:
	MessageLabelClass() = default;
	MessageLabelClass(int x, int y, size_t id, int deleteTime, bool animate, int drawDelay);

	~MessageLabelClass() = default;

	virtual bool Draw(bool bForced) override;

	inline const wchar_t* GetText() const { return ScenarioExt::Global()->RecordMessages[this->ID].c_str(); }

	size_t ID { 0 };
	int DeleteTime { 0 };
	bool Animate { false };
	size_t AnimPos { 0 };
	size_t AnimTiming { 0 };
	size_t DrawPos { 0 };
	int DrawDelay { 0 };
};

// --------------------------------------------------

class MessageColumnClass
{
public:
	static MessageColumnClass Instance;
	static constexpr int TextReservedSpace = 8;
	static constexpr int HighOpacity = 90;
	static constexpr int MediumOpacity = 60;
	static constexpr int LowOpacity = 30;

public:
	MessageColumnClass() = default;
	~MessageColumnClass();

	void InitClear();
	void InitIO();

private:
	void Initialize(int x = 0, int y = 0, int maxCount = 0, int maxRecord = 0, int maxChars = 0, int width = 640);

public:
	MessageLabelClass* AddMessage(const wchar_t* name, const wchar_t* message, int timeout, bool silent, int delay = 0);

	void MouseEnter(bool block = false);
	void MouseLeave(bool block = false);
	bool CanScrollUp();
	bool CanScrollDown();
	void ScrollUp();
	void ScrollDown();
	void SetScroll(int index = 0);
	void Expand();
	void PackUp(bool clear = false);

private:
	void CleanUp();
	void Refresh();
	void Update();

public:
	void Toggle();
	void Manage();
	void DrawAll();

	inline int GetWidth() const { return this->Width; }
	inline size_t GetTextColor() const { return (this->Color.B << 16) | (this->Color.G << 8) | this->Color.R; }
	inline ColorStruct GetColor() const { return this->Color; }
	inline int GetScrollIndex() const { return this->ScrollIndex; }
	inline bool IsHovering() const { return this->Hovering; }
	inline bool IsExpanded() const { return this->Expanded; }
	inline bool IsDrawing() const { return this->Drawing; }
	inline bool IsBlocked() const { return (this->Expanded || this->Blocked) && this->Hovering; }

	static inline int GetSystemTime();
	static inline bool IsStickyButton(const GadgetClass* pButton);
	static inline void IncreaseBrightness(ColorStruct& color, int level = 1);
	static inline void DecreaseBrightness(ColorStruct& color, int level = 1);

	inline bool GetThumbDimension(int* pMax, int* pHeight, int* pPosY = nullptr) const;

private:
	static inline bool AddRecordString(const std::wstring& message, size_t copySize = std::wstring::npos);

	inline void RemoveTextLabel(MessageLabelClass* pLabel);
	inline int GetLabelCount() const;
	inline MessageLabelClass* GetLastLabel() const;
	template <bool check = false>
	inline int GetMaxScroll() const;

	MessageLabelClass* LabelList { nullptr };
	Point2D LabelsPos { Point2D::Empty };

	int MaxCount { 0 };
	int MaxRecord { 0 };
	int MaxChars { 0 };
	int Height { 0 };
	int Width { 0 };
	ColorStruct Color { ColorStruct { 0, 0, 0 } };

	MessageToggleClass* Button_Main { nullptr };
	MessageToggleClass* Button_Toggle { nullptr };
	MessageButtonClass* Button_Up { nullptr };
	MessageButtonClass* Button_Down { nullptr };
	MessageScrollClass* Scroll_Bar { nullptr };
	MessageScrollClass* Scroll_Board { nullptr };

	int ScrollIndex { 0 };
	bool Hovering { false };
	bool Expanded { false };
	bool Drawing { false };
	bool Blocked { false };
};

// --------------------------------------------------
