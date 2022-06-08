#include "DigitalDisplayTypeClass.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <PhobosHelper/Helper.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>

Enumerable<DigitalDisplayTypeClass>::container_t Enumerable<DigitalDisplayTypeClass>::Array;

const char* Enumerable<DigitalDisplayTypeClass>::GetMainSection()
{
	return "DigitalDisplayTypes";
}

void DigitalDisplayTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->Text_ColorHigh.Read(exINI, section, "Text.ColorHigh");
	this->Text_ColorMid.Read(exINI, section, "Text.ColorMid");
	this->Text_ColorLow.Read(exINI, section, "Text.ColorLow");
	this->Text_Background.Read(exINI, section, "Text.Background");
	this->Offset.Read(exINI, section, "Offset");
	this->Offset_WithoutShield.Read(exINI, section, "Offset.WithoutShield");
	this->UseSHP.Read(exINI, section, "UseSHP");
	this->SHP_SHPFile.Read(pINI, section, "SHP.SHPFile");
	this->SHP_PALFile.Read(pINI, section, "SHP.PALFile");
	this->SHP_Interval.Read(exINI, section, "SHP.Interval");
	this->SHP_Interval_Building.Read(exINI, section, "SHP.Interval.Building");
	this->Align.Read(pINI, section, "Align");
	this->Anchor.Read(pINI, section, "Anchor");
	this->Percentage.Read(exINI, section, "Percentage");
	this->HideStrength.Read(exINI, section, "HideStrength");
	this->InfoType.Read(pINI, section, "InfoType");
	this->SetDisplayInfo();

	if (strcmp(Align.data(), "Left") == 0)
		Alignment = AlignType::Left;
	else if (strcmp(Align.data(), "Right") == 0)
		Alignment = AlignType::Right;
	else if (strcmp(Align.data(), "Center") == 0)
		Alignment = AlignType::Center;
	else
		Alignment = AlignType::Default;

	if (strcmp(Anchor.data(), "Left") == 0)
		Anchoring = AnchorType::Left;
	else if (strcmp(Anchor.data(), "Right") == 0)
		Anchoring = AnchorType::Right;
	else if (strcmp(Anchor.data(), "TopRight") == 0)
		Anchoring = AnchorType::TopRight;
	else
		Anchoring = AnchorType::TopLeft;

	if (UseSHP.Get())
	{
		if (strcmp(SHP_SHPFile.data(), "") != 0)
			SHPFile = FileSystem::LoadSHPFile(SHP_SHPFile.data());

		if (strcmp(SHP_PALFile.data(), "") != 0)
			PALFile = FileSystem::LoadPALFile(SHP_PALFile.data(), DSurface::Composite);
		else
			PALFile = FileSystem::PALETTE_PAL;
	}
}

void DigitalDisplayTypeClass::SetDisplayInfo()
{
	if (strcmp(InfoType.data(), "Health") == 0)
		InfoClass = Info::Health;
	else if (strcmp(InfoType.data(), "Shield") == 0)
		InfoClass = Info::Shield;
	else if (strcmp(InfoType.data(), "Ammo") == 0)
		InfoClass = Info::Ammo;
	else if (strcmp(InfoType.data(), "MindControl") == 0)
		InfoClass = Info::MindControl;
	else if (strcmp(InfoType.data(), "Spawns") == 0)
		InfoClass = Info::Spawns;
	else if (strcmp(InfoType.data(), "Passengers") == 0)
		InfoClass = Info::Passengers;
	else if (strcmp(InfoType.data(), "Tiberium") == 0)
		InfoClass = Info::Tiberium;
	else if (strcmp(InfoType.data(), "Experience") == 0)
		InfoClass = Info::Experience;
	else if (strcmp(InfoType.data(), "Occupants") == 0)
		InfoClass = Info::Occupants;
}

int operator & (DigitalDisplayTypeClass::AnchorType a, DigitalDisplayTypeClass::AnchorType b)
{
	return static_cast<int>(a) & static_cast<int>(b);
}

void DigitalDisplayTypeClass::RunDigitalDisplay(TechnoClass* pThis, Point2D* pLocation)
{
	if (!Phobos::Config::DigitalDisplay_Enable)
		return;

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	ValueableVector<DigitalDisplayTypeClass*>* pDefaultTypes = nullptr;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Building:
		pDefaultTypes = &RulesExt::Global()->Buildings_DefaultDigitalDisplayTypes;
		break;
	case AbstractType::Infantry:
		pDefaultTypes = &RulesExt::Global()->Infantrys_DefaultDigitalDisplayTypes;
		break;
	case AbstractType::Unit:
		pDefaultTypes = &RulesExt::Global()->Units_DefaultDigitalDisplayTypes;
		break;
	case AbstractType::Aircraft:
		pDefaultTypes = &RulesExt::Global()->Aircrafts_DefaultDigitalDisplayTypes;
		break;
	default:
		return;
	}

	if (pTypeExt->DigitalDisplayTypes.empty())
	{
		for (DigitalDisplayTypeClass*& pDisplayType : *pDefaultTypes)
		{
			pDisplayType->DigitalDisplay(pThis, pLocation);
		}
	}
	else
	{
		for (DigitalDisplayTypeClass*& pDisplayType : pTypeExt->DigitalDisplayTypes)
		{
			pDisplayType->DigitalDisplay(pThis, pLocation);
		}
	}
}

void DigitalDisplayTypeClass::DigitalDisplay(TechnoClass* pThis, Point2D* pLocation)
{//pos use for reference ShieldClass::DrawShieldBar_Building
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	Point2D Loc = *pLocation;
	int ValueA = 0;
	int ValueB = 0;
	AbstractType ThisAbstractType = pThis->WhatAmI();
	bool isBuilding = ThisAbstractType == AbstractType::Building;
	bool Shield = InfoClass == Info::Shield;

	switch (InfoClass)
	{
	case Info::Health:
	{
		ValueA = pThis->Health;
		ValueB = pType->Strength;
		break;
	}
	case Info::Shield:
	{
		if (pExt->Shield == nullptr || pExt->Shield->IsBrokenAndNonRespawning())
			return;
		ValueA = pExt->Shield->GetHP();
		ValueB = pExt->Shield->GetType()->Strength.Get();
		break;
	}
	case Info::Ammo:
	{
		if (pType->Ammo <= 0)
			return;
		ValueA = pThis->Ammo;
		ValueB = pType->Ammo;
		break;
	}
	case Info::MindControl:
	{
		if (pThis->CaptureManager == nullptr)
			return;
		ValueA = pThis->CaptureManager->ControlNodes.Count;
		ValueB = pThis->CaptureManager->MaxControlNodes;
		break;
	}
	case Info::Spawns:
	{
		if (pThis->SpawnManager == nullptr || pType->Spawns == nullptr || pType->SpawnsNumber <= 0)
			return;
		ValueA = pThis->SpawnManager->SpawnCount;
		ValueB = pType->SpawnsNumber;
		break;
	}
	case Info::Passengers:
	{
		if (pType->Passengers <= 0)
			return;
		ValueA = pThis->Passengers.NumPassengers;
		ValueB = pType->Passengers;
		break;
	}
	case Info::Tiberium:
	{
		if (pType->Storage <= 0)
			return;
		ValueA = static_cast<int>(pThis->Tiberium.GetTotalAmount());
		ValueB = pType->Storage;
		break;
	}
	case Info::Experience:
	{
		ValueA = static_cast<int>(pThis->Veterancy.Veterancy * RulesClass::Instance->VeteranRatio * pType->GetCost());
		ValueB = static_cast<int>(2.0 * RulesClass::Instance->VeteranRatio * pType->GetCost());
		break;
	}
	case Info::Occupants:
	{
		if (!isBuilding)
			return;
		BuildingTypeClass* pBuildingType = abstract_cast<BuildingTypeClass*>(pType);
		BuildingClass* pBuilding = abstract_cast<BuildingClass*>(pThis);
		if (!pBuildingType->CanBeOccupied)
			return;
		ValueA = pBuilding->Occupants.Count;
		ValueB = pBuildingType->MaxNumberOccupants;
		break;
	}
	default:
	{
		ValueA = pThis->Health;
		ValueB = pType->Strength;
		break;
	}
	}

	HealthBarAnchors Anchor = (this->Anchoring & DigitalDisplayTypeClass::AnchorType::Right
		? HealthBarAnchors::TopRight : HealthBarAnchors::TopLeft);
	Point2D Pos = TechnoExt::GetHealthBarPosition(pThis, Shield, Anchor);

	if (pExt->Shield == nullptr ||
		pExt->Shield->IsBrokenAndNonRespawning())
	{
		Pos.X += Offset_WithoutShield.Get(Offset.Get()).X;
		Pos.Y += Offset_WithoutShield.Get(Offset.Get()).Y;
	}
	else
	{
		Pos.X += Offset.Get().X;
		Pos.Y += Offset.Get().Y;
	}

	if (UseSHP)
	{
		if (isBuilding)
			Pos.X -= 4 + (!Shield ? 6 : 0);

		if (Shield)
			Pos.Y -= SHPFile->Height - 3;

		DigitalDisplaySHP(pThis, Pos, ValueA, ValueB);
	}
	else
	{
		if (Shield)
			Pos.Y -= 10;

		if (isBuilding)
			Pos.X -= 4 + (!Shield ? 6 : 0);

		DigitalDisplayText(pThis, Pos, ValueA, ValueB);
	}
}

void DigitalDisplayTypeClass::DigitalDisplayText(TechnoClass* pThis, Point2D Pos, int ValueA, int ValueB)
{
	bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
	wchar_t Healthpoint[0x20];

	if (Percentage.Get())
	{
		swprintf_s(Healthpoint, L"%d", static_cast<int>((static_cast<double>(ValueA) / ValueB) * 100));
		wcscat_s(Healthpoint, L"%%");
	}
	else if (HideStrength.Get())
	{
		swprintf_s(Healthpoint, L"%d", ValueA);
	}
	else
	{
		swprintf_s(Healthpoint, L"%d/%d", ValueA, ValueB);
	}

	COLORREF Color;
	double ratio = static_cast<double>(ValueA) / ValueB;

	if (ratio > RulesClass::Instance->ConditionYellow)
		Color = Drawing::RGB2DWORD(Text_ColorHigh.Get());
	else if (ratio > RulesClass::Instance->ConditionRed)
		Color = Drawing::RGB2DWORD(Text_ColorMid.Get());
	else
		Color = Drawing::RGB2DWORD(Text_ColorLow.Get());

	bool ShowBackground = Text_Background;
	RectangleStruct rect = { 0,0,0,0 };
	DSurface::Temp->GetRect(&rect);
	COLORREF BackColor = 0;
	TextPrintType PrintType;
	int TextLength = wcslen(Healthpoint);

	if (Anchoring & DigitalDisplayTypeClass::AnchorType::Top)
		Pos.Y -= 20;

	switch (Alignment)
	{
	case DigitalDisplayTypeClass::AlignType::Left:
	{
		PrintType = TextPrintType::FullShadow;
		if (Anchoring == DigitalDisplayTypeClass::AnchorType::Left)
			Pos.X -= TextLength * 6;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Right:
	{
		PrintType = TextPrintType(int(TextPrintType::Right) + int(TextPrintType::FullShadow));
		if (Anchoring == DigitalDisplayTypeClass::AnchorType::Right)
			Pos.X += TextLength * 6;
		if (pThis->WhatAmI() == AbstractType::Building)
			Pos.X += 6;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Center:
	{
		PrintType = TextPrintType::Center;

		if (isBuilding)
		{
			BuildingTypeClass* pBuildingType = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			int Height = pBuildingType->GetFoundationHeight(true);
			int iLength = Height * 7 + Height / 2;
			Pos.X += iLength * 4;
			Pos.Y -= iLength * 2 + wcslen(Healthpoint) * 2;
		}
	}
	default:
		if (isBuilding)
		{
			PrintType = TextPrintType::Right;
		}
		else
		{
			PrintType = TextPrintType::Center;
			int iLength = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;
			Pos.X += iLength;
		}
		break;
	}

	if (Anchoring & DigitalDisplayTypeClass::AnchorType::Top)
		Pos.Y -= 4;

	//0x400 is TextPrintType::Background pr#563 YRpp
	PrintType = TextPrintType(int(PrintType) + (ShowBackground ? 0x400 : 0));

	//DSurface::Temp->DrawText(Healthpoint, vPosH.X, vPosH.Y, ShowHPColor);
	DSurface::Temp->DrawTextA(Healthpoint, &rect, &Pos, Color, BackColor, PrintType);
}

void DigitalDisplayTypeClass::DigitalDisplaySHP(TechnoClass* pThis, Point2D Pos, int ValueA, int ValueB)
{
	DynamicVectorClass<char>vA;
	DynamicVectorClass<char>vB;
	bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
	const Vector2D<int> Interval = (isBuilding ? SHP_Interval_Building.Get() : SHP_Interval.Get());
	int iLength = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;

	if (isBuilding)
	{
		BuildingTypeClass* pBuildingType = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
		int Height = pBuildingType->GetFoundationHeight(true);
		iLength = Height * 7 + Height / 2;
	}

	if (SHPFile == nullptr || PALFile == nullptr)
		return;

	if (Anchoring & DigitalDisplayTypeClass::AnchorType::Top)
	{
		Pos.Y -= isBuilding ? 14 : 8;
		Pos.Y -= SHPFile->Height;
	}

	if (Percentage.Get())
	{
		vA = IntToVector(static_cast<int>(static_cast<double>(ValueA) / ValueB * 100));
	}
	else
	{
		vA = IntToVector(ValueA);

		if (!HideStrength.Get())
			vB = IntToVector(ValueB);
	}

	bool LeftToRight = true;

	switch (Alignment)
	{
	case DigitalDisplayTypeClass::AlignType::Left:
	{
		if (Anchoring == DigitalDisplayTypeClass::AnchorType::Left)
			Pos.X -= (vA.Count + vB.Count + (Percentage || !HideStrength)) * Interval.X + 2;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Right:
	{
		LeftToRight = false;
		if (Anchoring == DigitalDisplayTypeClass::AnchorType::Right)
			Pos.X += (vA.Count + vB.Count + (Percentage || !HideStrength)) * Interval.X + 2;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Center:
	{
		int FixValueX = 0;
		int FixValueY = 0;

		if (isBuilding)
		{
			Pos.X += iLength * 2;
			Pos.Y += iLength;
		}
		else
		{
			Pos.X += iLength;
		}

		if (Percentage)
		{
			FixValueX = (vA.Count + 1) * Interval.X / 2;
			FixValueY = (vA.Count + 1) * Interval.Y / 2;
		}
		else if (HideStrength)
		{
			FixValueX = vA.Count * Interval.X / 2;
			FixValueY = vA.Count * Interval.Y / 2;
		}
		else
		{
			FixValueX = (vA.Count + vB.Count + 1) * Interval.X / 2;
			FixValueY = (vA.Count + vB.Count + 1) * Interval.Y / 2;
		}

		if (Anchoring & DigitalDisplayTypeClass::AnchorType::Right)
			Pos.X += FixValueX;
		else
			Pos.X -= FixValueX;

		Pos.Y -= FixValueY;
	}
	break;
	default:
	{
		if (!isBuilding)
		{
			int FixValueX = 0;
			int FixValueY = 0;

			Pos.X += iLength;

			if (Percentage)
			{
				FixValueX = (vA.Count + 1) * Interval.X / 2;
				FixValueY = (vA.Count + 1) * Interval.Y / 2;
			}
			else if (HideStrength)
			{
				FixValueX = vA.Count * Interval.X / 2;
				FixValueY = vA.Count * Interval.Y / 2;
			}
			else
			{
				FixValueX = (vA.Count + vB.Count + 1) * Interval.X / 2;
				FixValueY = (vA.Count + vB.Count + 1) * Interval.Y / 2;
			}

			if (int(Anchoring) & int(DigitalDisplayTypeClass::AnchorType::Right))
				Pos.X += FixValueX;
			else
				Pos.X -= FixValueX;

			Pos.Y -= FixValueY;
		}
	}
	break;
	}

	int base = 0;
	int signframe = 30;
	double ratio = static_cast<double>(ValueA) / ValueB;

	if (ratio > RulesClass::Instance->ConditionYellow)
		base = 0;
	else if (ratio > RulesClass::Instance->ConditionRed)
		base = 10;
	else
		base = 20;

	if (base == 10)
		signframe = 31;
	else if (base == 20)
		signframe = 32;

	if (Percentage)
		signframe += 3;

	if (LeftToRight)
	{
		for (int i = vA.Count - 1; i >= 0; i--)
		{
			int num = base + vA.GetItem(i);

			DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &Pos, &DSurface::ViewBounds,
				BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			Pos.X += Interval.X;
			Pos.Y -= Interval.Y;
		}

		if (!Percentage && HideStrength)
			return;

		DSurface::Composite->DrawSHP(PALFile, SHPFile, signframe, &Pos, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		Pos.X += Interval.X;
		Pos.Y -= Interval.Y;

		if (Percentage)
			return;

		for (int i = vB.Count - 1; i >= 0; i--)
		{
			int num = base + vB.GetItem(i);

			DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &Pos, &DSurface::ViewBounds,
				BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			Pos.X += Interval.X;
			Pos.Y -= Interval.Y;
		}
	}
	else
	{
		if (Percentage || HideStrength)
		{
			if (Percentage)
			{
				DSurface::Composite->DrawSHP(PALFile, SHPFile, signframe, &Pos, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				Pos.X -= Interval.X;
				Pos.Y += Interval.Y;
			}

			for (int i = 0; i < vA.Count; i++)
			{
				int num = base + vA.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &Pos, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				Pos.X -= Interval.X;
				Pos.Y += Interval.Y;
			}
		}
		else
		{
			for (int i = 0; i < vB.Count; i++)
			{
				int num = base + vB.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &Pos, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				Pos.X -= Interval.X;
				Pos.Y += Interval.Y;
			}

			DSurface::Composite->DrawSHP(PALFile, SHPFile, signframe, &Pos, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			Pos.X -= Interval.X;
			Pos.Y += Interval.Y;

			for (int i = 0; i < vA.Count; i++)
			{
				int num = base + vA.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &Pos, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				Pos.X -= Interval.X;
				Pos.Y += Interval.Y;
			}
		}
	}
}


template <typename T>
void DigitalDisplayTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Text_ColorHigh)
		.Process(this->Text_ColorMid)
		.Process(this->Text_ColorLow)
		.Process(this->Text_Background)
		.Process(this->Offset)
		.Process(this->Offset_WithoutShield)
		.Process(this->UseSHP)
		.Process(this->SHP_SHPFile)
		.Process(this->SHP_PALFile)
		.Process(this->SHP_Interval)
		.Process(this->SHP_Interval_Building)
		.Process(this->Align)
		.Process(this->Alignment)
		.Process(this->Anchor)
		.Process(this->Anchoring)
		.Process(this->Percentage)
		.Process(this->HideStrength)
		.Process(this->InfoType)
		.Process(this->InfoClass)
		;
}

void DigitalDisplayTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);

	if (UseSHP.Get())
	{
		if (strcmp(SHP_SHPFile.data(), "") != 0)
			SHPFile = FileSystem::LoadSHPFile(SHP_SHPFile.data());

		if (strcmp(SHP_PALFile.data(), "") != 0)
			PALFile = FileSystem::LoadPALFile(SHP_PALFile.data(), DSurface::Composite);
		else
			PALFile = FileSystem::PALETTE_PAL;
	}
}

void DigitalDisplayTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
