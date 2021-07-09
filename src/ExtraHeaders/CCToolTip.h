#pragma once
#include <ASMMacros.h>
#include <GeneralStructures.h>
#include <Helpers/CompileTime.h>

struct ToolTipManager_Draw
{
	Point2D __LastCursorPos;
	Point2D __uPos;
	wchar_t HelpText[256];
};

struct ToolTip
{
	Point2D* CursorPositions;
	RectangleStruct Rect;
	int String;
	char char_18;
};

struct ToolTipManager
{
	void* vtable;
	ToolTip* ActiveTooltip;
	DWORD HWND;
	BYTE TooltipState;
	char field_D;
	char field_E;
	char field_F;
	Point2D __CursorPos;
	ToolTipManager_Draw ToolTipDraw;
	DWORD Timeout1;
	int LastTimeout;
	DWORD Timeout2;
	DynamicVectorClass<ToolTip*> ToolTips;
	BYTE ToolTipData[0x14];
};

class CCToolTip
{
public:
	static constexpr reference<bool, 0x884B8C> HideName{};

	const inline void Draw2() const
	{
		auto t = &this->manager.ToolTipDraw;
		PUSH_VAR32(t);
		THISCALL(0x478E30)
	}

	const inline bool Adjust() const
	{
		THISCALL(0x724AD0)
	}

	ToolTipManager manager;
	bool drawOnSidebar;
	char field_261;
	char field_262;
	char field_263;
	int delay_264;
};
