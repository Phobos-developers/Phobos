#pragma once

#include <SidebarClass.h>
#include <SuperWeaponTypeClass.h>
#include <TechnoTypeClass.h>

#include <Phobos.h>

#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>

#include <string>

struct StripClass;

class PhobosToolTip
{
public:
	static PhobosToolTip Instance;

private:
	inline const wchar_t* GetUIDescription(TechnoTypeExt::ExtData* pData) const;
	inline const wchar_t* GetUIDescription(SWTypeExt::ExtData* pData) const;
	inline int GetBuildTime(TechnoTypeClass* pType) const;
	inline int GetPower(TechnoTypeClass* pType) const;

public:
	inline bool IsEnabled() const;
	inline const wchar_t* GetBuffer() const;

	void HelpText(BuildType& cameo);
	void HelpText_Techno(TechnoTypeClass* pType);
	void HelpText_Super(int swidx);

	// Properties
private:
	std::wstring TextBuffer {};

public:
	bool IsCameo { false };
	bool SlaveDraw { false };
};
