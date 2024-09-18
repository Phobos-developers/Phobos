#include <Utilities/Macro.h>
#include <Ext/SWType/Body.h>
#include <ControlClass.h>

class TacticalButtonClass
{
public:
	static TacticalButtonClass Instance;

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
	void DrawButtonForSW();
	void RecheckButtonForSW();
	bool InsertButtonForSW(int& superIndex);
	bool SortButtonForSW(SuperWeaponTypeClass* pDataType, SuperWeaponTypeClass* pAddType, SWTypeExt::ExtData* pAddTypeExt, unsigned int ownerBits);
	void TriggerButtonForSW(int buttonIndex);

	struct DummySelectClass
	{
		char _[0x2C] {}; // : ControlClass
		StripClass *LinkTo { nullptr };
		int unknown_int_30 { 0 };
		bool MouseEntered { false };
		int SWIndex { -1 }; // New
	};

	// TODO New buttons (Start from index = 11)

public:
	bool PressedInButtonsLayer { false }; // Check press

	// Button index 1-10 : Super weapons buttons
	bool DummyAction { false };
	bool KeyboardCall { false };
	bool SuperVisible { true };

	// TODO New buttons (Start from index = 11)

private:
	int ButtonIndex { -1 }; // -1 -> above no buttons, 0 -> above buttons background, POSITIVE -> above button who have this index
	Point2D LastPosition { Point2D::Empty }; // Check moving

	// Button index 1-10 : Super weapons buttons
	std::vector<int> SWButtonData;
	SuperClass* RecordSuper { nullptr }; // Cannot be used, only for comparison purposes

	// TODO New buttons (Start from index = 11)
};
