#include <Utilities/Macro.h>
#include <Ext/SWType/Body.h>
#include <ControlClass.h>

class TacticalButtonsClass
{
public:
	static TacticalButtonsClass Instance;
	static PhobosMap<int, const wchar_t*> KeyboardCodeTextMap;

private:
	int CheckMouseOverButtons(const Point2D* pMousePosition);
	bool CheckMouseOverBackground(const Point2D* pMousePosition);

public:
	inline bool MouseIsOverButtons();
	inline bool MouseIsOverTactical();

	int GetButtonIndex();
	void RecheckButtonIndex();
	void SetMouseButtonIndex(const Point2D* pMousePosition);
	void PressDesignatedButton(int triggerIndex);

	// Button index 1-10 : Super weapons buttons
	inline bool IndexInSWButtons();

	void SWSidebarDraw();
	void SWSidebarRecheck();
	bool SWSidebarAdd(int& superIndex);
	bool SWSidebarSort(SuperWeaponTypeClass* pDataType, SuperWeaponTypeClass* pAddType, SWTypeExt::ExtData* pAddTypeExt, unsigned int ownerBits);
	void SWSidebarTrigger(int buttonIndex);
	void SWSidebarRecord(int buttonIndex, int key);

	struct DummySelectClass
	{
		char _[0x2C] {}; // : ControlClass
		StripClass *LinkTo { nullptr };
		int unknown_int_30 { 0 };
		bool MouseEntered { false };
		int SWIndex { -1 }; // New
	};

	// Button index 11 : SW sidebar switch
	inline bool IndexIsSWSwitch();

	void SWSidebarSwitch();

	// Extra functions
	bool SWQuickLaunch(int superIndex);

	// TODO New buttons (Start from index = 12)

public:
	bool PressedInButtonsLayer { false }; // Check press

	// Button index 1-10 : Super weapons buttons
	bool DummyAction { false };
	bool KeyboardCall { false };
	std::wstring keyCodeText[10] {};
	int keyCodeData[10] {};

	// TODO New buttons (Start from index = 12)

private:
	int ButtonIndex { -1 }; // -1 -> above no buttons, 0 -> above buttons background, POSITIVE -> above button who have this index
	Point2D LastPosition { Point2D::Empty }; // Check moving

	// Button index 1-10 : Super weapons buttons
	std::vector<int> SWButtonData;
	SuperClass* RecordSuper { nullptr }; // Cannot be used, only for comparison purposes

	// Button index 11 : SW sidebar switch
	bool SuperVisible { true };

	// TODO New buttons (Start from index = 12)
};
