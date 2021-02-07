#pragma once
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <SidebarClass.h>
#include <CCToolTip.h>
#include <StringTable.h>
#include <wchar.h>
#include "../Ext/TechnoType/Body.h"
#include "../Ext/SWType/Body.h"

#define TOOLTIP_BUFFER_LENGTH 1024
class ExtToolTip
{
public:
	static inline wchar_t* GetBuffer() {
		return _UseExtBuffer ? _ExtBuffer : CCToolTip::Instance->manager.ToolTipDraw.HelpText;
	}

	static inline void ClearBuffer() {
		_UseExtBuffer = false;
		_ExtBuffer[0] = NULL;
	}

	static inline void UseExtBuffer() {
		_UseExtBuffer = true;
		_ExtBuffer[TOOLTIP_BUFFER_LENGTH - 1] = NULL;
	}

	static inline void SetBuffer(REGISTERS* R) {
		if (ExtToolTip::_UseExtBuffer)
			R->EDI(ExtToolTip::_ExtBuffer);
	}

	static inline void Append(const wchar_t* str) {
		wcscat_s(ExtToolTip::_ExtBuffer, str);
	}

	static inline void Append_NewLineLater() {
		addNewLine = true;
	}

	static inline void Append_SpaceLater() {
		addSpace = true;
	}

	static void Apply_SeparatorAsNewLine() {
		if (addNewLine || addSpace) {
			wcscat_s(ExtToolTip::_ExtBuffer, L"\n");
		}
		Clear_Separator();
	}

	static void Apply_Separator() {
		if (addNewLine) {
			wcscat_s(ExtToolTip::_ExtBuffer, L"\n");
		}
		else if (addSpace) {
			wcscat_s(ExtToolTip::_ExtBuffer, L" ");
		}
		Clear_Separator();
	}

	static inline void Clear_Separator() {
		addNewLine = false;
		addSpace = false;
	}

	static bool isCameo;
	static bool slaveDraw;
private:
	static bool _UseExtBuffer;
	static wchar_t _ExtBuffer[TOOLTIP_BUFFER_LENGTH];

	static bool addSpace;
	static bool addNewLine;
};