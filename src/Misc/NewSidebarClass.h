#pragma once

#include <SidebarClass.h>
#include <PowerClass.h>
#include <StageClass.h>

class NewSidebarClass : public PowerClass
{
public:
	//New
	int GetTab(AbstractType type, int id);

	class NewStripClass
	{
	public:
		int AddCameo(AbstractType type, int id);
		bool RTTICheck(AbstractType type1, int id1, AbstractType type2, int id2);
		bool IsOnSidebar(AbstractType type, int id);

		StageClass        Progress;
		bool              AllowedToDraw; // prevents redrawing when layouting the list
		PROTECTED_PROPERTY(BYTE, align_1D[3]);
		Point2D           Location;
		RectangleStruct   Bounds;
		int               Index; // the index of this tab
		bool              NeedsRedraw;
		BYTE              unknown_3D;
		BYTE              unknown_3E;
		BYTE              unknown_3F;
		DWORD             unknown_40;
		int               TopRowIndex; // scroll position, which row is topmost visible
		DWORD             unknown_48;
		DWORD             unknown_4C;
		DWORD             unknown_50;
		int               CameoCount; // filled cameos
		BuildType         Cameos[75];
	};

	enum
	{
		TabBuildings = 0,
		TabDefences = 1,
		TabSuperWeapons = 1, // vanilla behaviour for now
		TabInfantry = 2,
		TabVehicles = 3,
	};

	//Static
	static NewSidebarClass Instance;
	static constexpr reference<wchar_t, 0xB07BC4u, 0x42u> const TooltipBuffer {};

	void SidebarNeedsRepaint(int mode = 0);
	void StripNeedsRepaint(int tab = 0);
	bool AddCameo(AbstractType type, int id);

	virtual void Draw(DWORD dwUnk) override
	{ JMP_THIS(0x6A6C30); }

	//Destructor
	virtual ~NewSidebarClass() RX;

	//SidebarClass
	virtual bool vt_entry_D8(int nUnknown) R0;

	//Non-virtual

	// which tab does the 'th object of that type belong in?
	static int __fastcall GetObjectTabIdx(AbstractType abs, int idxType, int unused)
	{ JMP_STD(0x6ABC60); }

	// which tab does the 'th object of that type belong in?
	static int __fastcall GetObjectTabIdx(AbstractType abs, BuildCat buildCat, bool isNaval)
	{ JMP_STD(0x6ABCD0); }

protected:
	//Constructor
	NewSidebarClass() { }	//don't need this

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	// New
	NewStripClass Tabs[0x5];

	// Vanilla
	DWORD unknown_5394;
	DWORD unknown_5398;
	int ActiveTabIndex;
	DWORD unknown_53A0;
	bool HideObjectNameInTooltip; // see 0x6A9343
	bool IsSidebarActive;
	bool SidebarNeedsRedraw;
	bool SidebarBackgroundNeedsRedraw;
	bool unknown_bool_53A8;

	//Information for the Diplomacy menu, I believe
	HouseClass* DiplomacyHouses[0x8];
	int DiplomacyKills[0x8];		//total amount of kills per house
	int DiplomacyOwned[0x8];		//total amount of currently owned unit/buildings per house
	int DiplomacyPowerDrain[0x8];	//current power drain per house
	ColorScheme* DiplomacyColors[0x8];		//color scheme per house
	DWORD unknown_544C[0x8];			//??? per house - unused
	DWORD unknown_546C[0x8];			//??? per house - unused
	DWORD unknown_548C[0x8];			//??? per house - unused
	DWORD unknown_54AC[0x8];			//??? per house - unused
	DWORD unknown_54CC[0x8];			//??? per house - unused
	DWORD unknown_54EC[0x8];			//??? per house - unused
	BYTE unknown_550C;
	int DiplomacyNumHouses;			//possibly?

	bool unknown_bool_5514;
	bool unknown_bool_5515;
	PROTECTED_PROPERTY(BYTE, padding_5516[2]);
};
