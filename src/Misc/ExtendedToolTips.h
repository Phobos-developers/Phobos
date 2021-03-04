#pragma once
#include <BuildingTypeClass.h>
#include <HouseClass.h>
#include <SidebarClass.h>
#include <CCToolTip.h>
#include <StringTable.h>
#include <wchar.h>
#include "../Ext/TechnoType/Body.h"
#include "../Ext/SWType/Body.h"

class ExtToolTip
{
public:
	static inline void ClearBuffer() {
		_uesExtBuffer = false;
		_ExtBuffer[0] = NULL;
	}

	static inline void UseExtBuffer() {
		_uesExtBuffer = true;
		_ExtBuffer[TOOLTIP_BUFFER_LENGTH - 1] = NULL;
	}

	static inline void SetBuffer(REGISTERS* R) {
		if (_uesExtBuffer)
			R->EDI(_ExtBuffer);
	}

	static inline void Append(const wchar_t* str) {
		wcscat_s(_ExtBuffer, str);
	}

	static inline void Append_NewLineLater() {
		_addNewLine = true;
	}

	static inline void Append_SpaceLater() {
		_addSpace = true;
	}

	static void Apply_SeparatorAsNewLine() {
		if (_addNewLine || _addSpace) {
			wcscat_s(_ExtBuffer, L"\n");
		}
		Clear_Separator();
	}

	static void Apply_Separator() {
		if (_addNewLine) {
			wcscat_s(_ExtBuffer, L"\n");
		}
		else if (_addSpace) {
			wcscat_s(_ExtBuffer, L" ");
		}
		Clear_Separator();
	}

	static inline void Clear_Separator() {
		_addNewLine = false;
		_addSpace = false;
	}

	static void CreateHelpText(AbstractType itemType, int itemIndex);
private:
	static const int inline TOOLTIP_BUFFER_LENGTH = 1024;
	static bool inline _uesExtBuffer = false;
	static wchar_t inline _ExtBuffer[TOOLTIP_BUFFER_LENGTH] = L"";

	static bool inline _addSpace = false;
	static bool inline _addNewLine = false;

public:
	static const inline wchar_t* pseudoBuff = L"ToolTip";

	static bool inline isCameo = false;
	static bool inline slaveDraw = false;
};
