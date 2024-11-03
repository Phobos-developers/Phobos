#include "SWSidebarClass.h"
#include <CommandClass.h>
#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <sstream>

std::unique_ptr<SWSidebarClass> SWSidebarClass::Instance = nullptr;

void SWSidebarClass::Allocate()
{
	Instance = std::make_unique<SWSidebarClass>();
}

void SWSidebarClass::Remove()
{
	Instance = nullptr;
}

static PhobosMap<int, const wchar_t*> CreateKeyboardCodeTextMap()
{
	PhobosMap<int, const wchar_t*> Code2Text;

	Code2Text[0x00] = L"  ";
	Code2Text[0x01] = L"MouseLeft";
	Code2Text[0x02] = L"MouseRight";
	Code2Text[0x03] = L"Cancel";
	Code2Text[0x04] = L"MouseCenter";

	Code2Text[0x08] = L"Back";
	Code2Text[0x09] = L"Tab";

	Code2Text[0x0C] = L"Clear";
	Code2Text[0x0D] = L"Enter";

	Code2Text[0x10] = L"Shift";
	Code2Text[0x11] = L"Ctrl";
	Code2Text[0x12] = L"Alt";
	Code2Text[0x13] = L"Pause";
	Code2Text[0x14] = L"CapsLock";

	Code2Text[0x1B] = L"Esc";

	Code2Text[0x20] = L"Space";
	Code2Text[0x21] = L"PageUp";
	Code2Text[0x22] = L"PageDown";
	Code2Text[0x23] = L"End";
	Code2Text[0x24] = L"Home";
	Code2Text[0x25] = L"Left";
	Code2Text[0x26] = L"Up";
	Code2Text[0x27] = L"Right";
	Code2Text[0x28] = L"Down";
	Code2Text[0x29] = L"Select";
	Code2Text[0x2A] = L"Print";
	Code2Text[0x2B] = L"Execute";
	Code2Text[0x2C] = L"PrintScreen";
	Code2Text[0x2D] = L"Insert";
	Code2Text[0x2E] = L"Delete";
	Code2Text[0x2F] = L"Help";
	Code2Text[0x30] = L"0";
	Code2Text[0x31] = L"1";
	Code2Text[0x32] = L"2";
	Code2Text[0x33] = L"3";
	Code2Text[0x34] = L"4";
	Code2Text[0x35] = L"5";
	Code2Text[0x36] = L"6";
	Code2Text[0x37] = L"7";
	Code2Text[0x38] = L"8";
	Code2Text[0x39] = L"9";

	Code2Text[0x41] = L"A";
	Code2Text[0x42] = L"B";
	Code2Text[0x43] = L"C";
	Code2Text[0x44] = L"D";
	Code2Text[0x45] = L"E";
	Code2Text[0x46] = L"F";
	Code2Text[0x47] = L"G";
	Code2Text[0x48] = L"H";
	Code2Text[0x49] = L"I";
	Code2Text[0x4A] = L"J";
	Code2Text[0x4B] = L"K";
	Code2Text[0x4C] = L"L";
	Code2Text[0x4D] = L"M";
	Code2Text[0x4E] = L"N";
	Code2Text[0x4F] = L"O";
	Code2Text[0x50] = L"P";
	Code2Text[0x51] = L"Q";
	Code2Text[0x52] = L"R";
	Code2Text[0x53] = L"S";
	Code2Text[0x54] = L"T";
	Code2Text[0x55] = L"U";
	Code2Text[0x56] = L"V";
	Code2Text[0x57] = L"W";
	Code2Text[0x58] = L"X";
	Code2Text[0x59] = L"Y";
	Code2Text[0x5A] = L"Z";
	Code2Text[0x5B] = L"LWin";
	Code2Text[0x5C] = L"RWin";
	Code2Text[0x5D] = L"Menu";

	Code2Text[0x60] = L"Num0";
	Code2Text[0x61] = L"Num1";
	Code2Text[0x62] = L"Num2";
	Code2Text[0x63] = L"Num3";
	Code2Text[0x64] = L"Num4";
	Code2Text[0x65] = L"Num5";
	Code2Text[0x66] = L"Num6";
	Code2Text[0x67] = L"Num7";
	Code2Text[0x68] = L"Num8";
	Code2Text[0x69] = L"Num9";
	Code2Text[0x6A] = L"Num*";
	Code2Text[0x6B] = L"Num+";
	Code2Text[0x6C] = L"Separator";
	Code2Text[0x6D] = L"Num-";
	Code2Text[0x6E] = L"Num.";
	Code2Text[0x6F] = L"Num/";
	Code2Text[0x70] = L"F1";
	Code2Text[0x71] = L"F2";
	Code2Text[0x72] = L"F3";
	Code2Text[0x73] = L"F4";
	Code2Text[0x74] = L"F5";
	Code2Text[0x75] = L"F6";
	Code2Text[0x76] = L"F7";
	Code2Text[0x77] = L"F8";
	Code2Text[0x78] = L"F9";
	Code2Text[0x79] = L"F10";
	Code2Text[0x7A] = L"F11";
	Code2Text[0x7B] = L"F12";

	Code2Text[0x90] = L"NumLock";
	Code2Text[0x91] = L"ScrollLock";

	Code2Text[0xBA] = L";";
	Code2Text[0xBB] = L"=";
	Code2Text[0xBC] = L",";
	Code2Text[0xBD] = L"-";
	Code2Text[0xBE] = L".";
	Code2Text[0xBF] = L"/";
	Code2Text[0xC0] = L"`";

	Code2Text[0xDB] = L"[";
	Code2Text[0xDC] = L"\\";
	Code2Text[0xDD] = L"]";
	Code2Text[0xDE] = L"'";

	Code2Text[static_cast<int>(WWKey::Shift)] = L"Shift";
	Code2Text[static_cast<int>(WWKey::Ctrl)] = L"Ctrl";
	Code2Text[static_cast<int>(WWKey::Alt)] = L"Alt";

	return Code2Text;
}

PhobosMap<int, const wchar_t*> SWSidebarClass::KeyboardCodeTextMap = CreateKeyboardCodeTextMap();

// =============================
// functions

bool SWSidebarClass::AddColumn()
{
	auto& columns = this->Columns;

	if (static_cast<int>(columns.size()) >= Phobos::UI::ExclusiveSWSidebar_MaxColumn)
		return false;

	const auto column = DLLCreate<SWColumnClass>(TacticalButtonClass::StartID + SuperWeaponTypeClass::Array->Count + 1 + static_cast<int>(columns.size()), 0, 0, 60 + Phobos::UI::ExclusiveSWSidebar_Interval, 48);

	if (!column)
		return false;

	column->Zap();
	GScreenClass::Instance->AddButton(column);
	return true;
}

bool SWSidebarClass::RemoveColumn()
{
	auto& columns = this->Columns;

	if (columns.empty())
		return false;

	if (const auto backColumn = columns.back())
	{
		AnnounceInvalidPointer(SWSidebarClass::Global()->CurrentColumn, backColumn);
		GScreenClass::Instance->RemoveButton(backColumn);

		DLLDelete(backColumn);
		columns.erase(columns.end() - 1);
		return true;
	}

	return false;
}

void SWSidebarClass::InitClear()
{
	this->CurrentColumn = nullptr;
	this->CurrentButton = nullptr;

	if (const auto toggleButton = this->ToggleButton)
	{
		this->ToggleButton = nullptr;
		GScreenClass::Instance->RemoveButton(toggleButton);
		DLLDelete(toggleButton);
	}

	auto& columns = this->Columns;

	for (const auto column : columns)
	{
		column->ClearButtons();
		GScreenClass::Instance->RemoveButton(column);
		DLLDelete(column);
	}

	columns.clear();
}

bool SWSidebarClass::AddButton(int superIdx)
{
	auto& columns = this->Columns;

	if (columns.empty() && !this->AddColumn())
		return false;

	if (std::any_of(columns.begin(), columns.end(), [superIdx](SWColumnClass* column) { return std::any_of(column->Buttons.begin(), column->Buttons.end(), [superIdx](TacticalButtonClass* button) { return button->SuperIndex == superIdx; }); }))
		return true;

	const bool success = columns.back()->AddButton(superIdx);

	if (success)
		SidebarExt::Global()->SWSidebar_Indices.AddUnique(superIdx);

	return success;
}

void SWSidebarClass::SortButtons()
{
	auto& columns = this->Columns;

	if (columns.empty())
		return;

	columns.erase(
		std::remove_if(columns.begin(), columns.end(),
			[](SWColumnClass* const column)
			{ return column->Buttons.empty(); }
		),
		columns.end()
	);

	if (columns.empty())
	{
		if (const auto toggleButton = this->ToggleButton)
			toggleButton->UpdatePosition();

		return;
	}

	std::vector<TacticalButtonClass*> vec_Buttons;
	vec_Buttons.reserve(this->GetMaximumButtonCount());

	for (const auto column : columns)
	{
		for (const auto button : column->Buttons)
			vec_Buttons.emplace_back(button);

		column->ClearButtons(false);
	}

	std::stable_sort(vec_Buttons.begin(), vec_Buttons.end(), [](TacticalButtonClass* const a, TacticalButtonClass* const b)
		{
			return BuildType::SortsBefore(AbstractType::Special, a->SuperIndex, AbstractType::Special, b->SuperIndex);
		 });

	const int buttonCount = static_cast<int>(vec_Buttons.size());
	const int cameoWidth = 60, cameoHeight = 48;
	const int maximum = Phobos::UI::ExclusiveSWSidebar_Max;
	Point2D location = { 0, (DSurface::ViewBounds().Height - std::min(buttonCount, maximum) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int rowIdx = 0, columnIdx = 0;

	for (const auto button : vec_Buttons)
	{
		const auto column = columns[columnIdx];

		if (rowIdx == 0)
			column->SetPosition(location.X, location.Y);

		column->Buttons.emplace_back(button);
		button->SetColumn(columnIdx);
		button->SetPosition(location.X, location.Y);
		rowIdx++;

		if (rowIdx >= maximum - columnIdx)
		{
			rowIdx = 0;
			columnIdx++;
			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth + Phobos::UI::ExclusiveSWSidebar_Interval, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	for (const auto column : columns)
		column->SetHeight(column->Buttons.size() * 48);

	if (const auto toggleButton = this->ToggleButton)
		toggleButton->UpdatePosition();
}

void SWSidebarClass::RecordHotkey(int buttonIndex, int key)
{
	const int index = buttonIndex - 1;

	if (this->KeyCodeData[index] == key)
		return;

	this->KeyCodeData[index] = key;
	std::wostringstream oss;

	if (key & static_cast<int>(WWKey::Shift))
		oss << KeyboardCodeTextMap[static_cast<int>(WWKey::Shift)] << L"+";

	if (key & static_cast<int>(WWKey::Ctrl))
		oss << KeyboardCodeTextMap[static_cast<int>(WWKey::Ctrl)] << L"+";

	if (key & static_cast<int>(WWKey::Alt))
		oss << KeyboardCodeTextMap[static_cast<int>(WWKey::Alt)] << L"+";

	const int pureKey = key & 0xFF;

	if (KeyboardCodeTextMap.contains(pureKey))
		oss << KeyboardCodeTextMap[pureKey];
	else
		oss << L"Unknown";

	this->KeyCodeText[index] = oss.str();
}

int SWSidebarClass::GetMaximumButtonCount()
{
	const int firstColumn = Phobos::UI::ExclusiveSWSidebar_Max;
	const int columns = std::min(firstColumn, Phobos::UI::ExclusiveSWSidebar_MaxColumn);
	return (firstColumn + (firstColumn - (columns - 1))) * columns / 2;
}

bool SWSidebarClass::IsEnabled()
{
	return SidebarExt::Global()->SWSidebar_Enable;
}

// Hooks

DEFINE_HOOK(0x692419, DisplayClass_ProcessClickCoords_SWSidebar, 0x7)
{
	enum { Nothing = 0x6925FC };

	if (SWSidebarClass::IsEnabled() && SWSidebarClass::Global()->CurrentColumn)
		return Nothing;

	const auto toggleButton = SWSidebarClass::Global()->ToggleButton;

	return toggleButton && toggleButton->IsHovering ? Nothing : 0;
}

// I cannot add it into YRppp :(
// It always failed, help me
static void __fastcall HouseClass_UpdateSuperWeaponsUnavailable(HouseClass* pHouse)
{
	JMP_STD(0x50B1D0);
}

DEFINE_HOOK(0x4F92FB, HouseClass_UpdateTechTree_SWSidebar, 0x7)
{
	enum { SkipGameCode = 0x4F9302 };

	GET(HouseClass*, pHouse, ESI);

	HouseClass_UpdateSuperWeaponsUnavailable(pHouse);

	if (pHouse->IsCurrentPlayer())
	{
		for (const auto column : SWSidebarClass::Global()->Columns)
		{
			for (const auto button : column->Buttons)
			{
				if (HouseClass::CurrentPlayer->Supers[button->SuperIndex]->IsPresent)
					continue;

				if (column->RemoveButton(button->SuperIndex))
					SidebarExt::Global()->SWSidebar_Indices.Remove(button->SuperIndex);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6A6316, SidebarClass_AddCameo_SuperWeapon_SWSidebar, 0x6)
{
	enum { ReturnFalse = 0x6A65FF };

	GET_STACK(AbstractType, whatAmI, STACK_OFFSET(0x14, 0x4));
	GET_STACK(int, index, STACK_OFFSET(0x14, 0x8));

	switch (whatAmI)
	{
	case AbstractType::Super:
	case AbstractType::SuperWeaponType:
	case AbstractType::Special:
		if (SWSidebarClass::Global()->AddButton(index))
			return ReturnFalse;

		break;

	default:
		break;
	}

	return 0;
}

DEFINE_HOOK(0x6A5082, SidebarClass_InitClear_InitializeSWSidebar, 0x5)
{
	SWSidebarClass::Global()->InitClear();
	return 0;
}

DEFINE_HOOK(0x6A5839, SidebarClass_InitIO_InitializeSWSidebar, 0x5)
{
	if (!Phobos::UI::ExclusiveSWSidebar || Unsorted::ArmageddonMode)
		return 0;

	if (const auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->Items[ScenarioClass::Instance->PlayerSideIndex]))
	{
		if (const auto toggleShape = pSideExt->ExclusiveSWSidebar_ToggleShape.Get())
		{
			if (const auto toggleButton = DLLCreate<ToggleSWButtonClass>(TacticalButtonClass::StartID + SuperWeaponTypeClass::Array->Count, 0, 0, toggleShape->Width, toggleShape->Height))
			{
				toggleButton->Zap();
				GScreenClass::Instance->AddButton(toggleButton);
				SWSidebarClass::Global()->ToggleButton = toggleButton;
				toggleButton->UpdatePosition();
			}
		}
	}

	for (const auto superIdx : SidebarExt::Global()->SWSidebar_Indices)
		SWSidebarClass::Global()->AddButton(superIdx);

	return 0;
}

// Shortcuts keys hooks
DEFINE_HOOK(0x533E69, UnknownClass_sub_533D20_LoadKeyboardCodeFromINI, 0x6)
{
	GET(CommandClass*, pCommand, ESI);
	GET(int, key, EDI);

	const char* name = pCommand->GetName();
	char buffer[29];

	for (int idx = 1; idx <= 10; idx++)
	{
		sprintf_s(buffer, "SW Sidebar Shortcuts Num %02d", idx);

		if (!_strcmpi(name, buffer))
			SWSidebarClass::Global()->RecordHotkey(idx, key);
	}

	return 0;
}

DEFINE_HOOK(0x5FB992, UnknownClass_sub_5FB320_SaveKeyboardCodeToINI, 0x6)
{
	GET(CommandClass*, pCommand, ECX);
	GET(int, key, EAX);

	const char* name = pCommand->GetName();
	char buffer[30];

	for (int idx = 1; idx <= 10; idx++)
	{
		sprintf_s(buffer, "SW Sidebar Shortcuts Num %02d", idx);

		if (!_strcmpi(name, buffer))
			SWSidebarClass::Global()->RecordHotkey(idx, key);
	}

	return 0;
}
