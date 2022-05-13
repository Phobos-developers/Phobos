#include "DigitalDisplayTypeClass.h"

#include <TacticalClass.h>

#include <PhobosHelper/Helper.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>

Enumerable<DigitalDisplayTypeClass>::container_t Enumerable<DigitalDisplayTypeClass>::Array;

const char* Enumerable<DigitalDisplayTypeClass>::GetMainSection()
{
	return "DigitalDisplayTypes";
}

void DigitalDisplayTypeClass::LoadFromINI(CCINIClass * pINI)
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


void DigitalDisplayTypeClass::DigitalDisplay(TechnoClass* pThis, Point2D* pLocation, bool Shield)
{//pos use for reference ShieldClass::DrawShieldBar_Building

	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	Point2D Loc = *pLocation;

	if (Shield && (pExt->Shield == nullptr || pExt->Shield->IsBrokenAndNonRespawning()))
		return;

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		Loc.X -= 5;
		Loc.Y -= 3;
	}

	Loc.Y -= 5;

	DigitalDisplayTypeClass* pDisplayType = nullptr;
	AbstractType ThisAbstractType = pThis->WhatAmI();

	switch (ThisAbstractType)
	{
	case AbstractType::Building:
		pDisplayType = Shield ?
			pTypeExt->DigitalDisplayType_Shield.Get(pExt->Shield->GetType()->DigitalDisplayType.Get(RulesExt::Global()->Buildings_DefaultDigitalDisplayTypeSP.Get()))
			: pTypeExt->DigitalDisplayType.Get(RulesExt::Global()->Buildings_DefaultDigitalDisplayTypeHP.Get());
		break;
	case AbstractType::Infantry:
		pDisplayType = Shield ?
			pTypeExt->DigitalDisplayType_Shield.Get(pExt->Shield->GetType()->DigitalDisplayType.Get(RulesExt::Global()->Infantrys_DefaultDigitalDisplayTypeSP.Get()))
			: pTypeExt->DigitalDisplayType.Get(RulesExt::Global()->Infantrys_DefaultDigitalDisplayTypeHP.Get());
		break;
	case AbstractType::Unit:
		pDisplayType = Shield ?
			pTypeExt->DigitalDisplayType_Shield.Get(pExt->Shield->GetType()->DigitalDisplayType.Get(RulesExt::Global()->Units_DefaultDigitalDisplayTypeSP.Get()))
			: pTypeExt->DigitalDisplayType.Get(RulesExt::Global()->Units_DefaultDigitalDisplayTypeHP.Get());
		break;
	case AbstractType::Aircraft:
		pDisplayType = Shield ?
			pTypeExt->DigitalDisplayType_Shield.Get(pExt->Shield->GetType()->DigitalDisplayType.Get(RulesExt::Global()->Aircrafts_DefaultDigitalDisplayTypeSP.Get()))
			: pTypeExt->DigitalDisplayType.Get(RulesExt::Global()->Aircrafts_DefaultDigitalDisplayTypeHP.Get());
		break;
	default:
		break;
	}

	if (pDisplayType == nullptr)
		return;

	Point2D Pos = { 0, 0 };

	if (ThisAbstractType == AbstractType::Building)
	{
		BuildingTypeClass* pBuildingType = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
		CoordStruct Coords = { 0, 0, 0 };
		pBuildingType->Dimension2(&Coords);
		Point2D Pos2 = { 0, 0 };
		CoordStruct Coords2 = { -Coords.X / 2, Coords.Y / 2, Coords.Z };
		TacticalClass::Instance->CoordsToScreen(&Pos2, &Coords2);

		Pos.X = Pos2.X + Loc.X + 4 * 17 - 70;
		Pos.Y = Pos2.Y + Loc.Y - 2 * 17 + 30;

		if (int(pDisplayType->Anchoring) & int(DigitalDisplayTypeClass::AnchorType::Right))
		{
			int Height = pBuildingType->GetFoundationHeight(true);
			int iLength = Height * 7 + Height / 2;
			Pos.X += iLength * 4;
			Pos.Y -= iLength * 2;
		}
	}
	else
	{
		int iLength = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;
		Pos.X = Loc.X - iLength;
		Pos.Y = Loc.Y - 24;
		Pos.Y += pType->PixelSelectionBracketDelta;

		if (int(pDisplayType->Anchoring) & int(DigitalDisplayTypeClass::AnchorType::Right))
			Pos.X += iLength * 2;
	}


	if (pExt->Shield == nullptr ||
		pExt->Shield->IsBrokenAndNonRespawning())
	{
		Pos.X += pDisplayType->Offset_WithoutShield.Get(pDisplayType->Offset.Get()).X;
		Pos.Y += pDisplayType->Offset_WithoutShield.Get(pDisplayType->Offset.Get()).Y;
	}
	else
	{
		Pos.X += pDisplayType->Offset.Get().X;
		Pos.Y += pDisplayType->Offset.Get().Y;
	}

	bool UseSHP = pDisplayType->UseSHP;

	RectangleStruct rect = { Pos.X,Pos.Y,4,4 };
	DSurface::Composite->DrawRect(&rect, Drawing::RGB2DWORD(0, 0, 255));

	if (UseSHP)
		DigitalDisplaySHP(pThis, pDisplayType, Pos, Shield);
	else
		DigitalDisplayText(pThis, pDisplayType, Pos, Shield);
}

void DigitalDisplayTypeClass::DigitalDisplayText(TechnoClass* pThis, DigitalDisplayTypeClass* pDisplayType, Point2D Pos, bool Shield)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	wchar_t Healthpoint[0x20];

	if (pDisplayType->Percentage.Get())
	{
		if (Shield)
			swprintf_s(Healthpoint, L"%d%%", int(pExt->Shield->GetHealthRatio() * 100));
		else
			swprintf_s(Healthpoint, L"%d%%", int(pThis->GetHealthPercentage() * 100));
	}
	else if (pDisplayType->HideStrength.Get())
	{
		if (Shield)
			swprintf_s(Healthpoint, L"%d", pExt->Shield->GetHP());
		else
			swprintf_s(Healthpoint, L"%d", pThis->Health);
	}
	else
	{
		if (Shield)
			swprintf_s(Healthpoint, L"%d/%d", pExt->Shield->GetHP(), pExt->Shield->GetType()->Strength.Get());
		else
			swprintf_s(Healthpoint, L"%d/%d", pThis->Health, pThis->GetTechnoType()->Strength);
	}
	COLORREF HPColor;

	if (pThis->IsGreenHP())
		HPColor = Drawing::RGB2DWORD(pDisplayType->Text_ColorHigh.Get());
	else if (pThis->IsYellowHP())
		HPColor = Drawing::RGB2DWORD(pDisplayType->Text_ColorMid.Get());
	else
		HPColor = Drawing::RGB2DWORD(pDisplayType->Text_ColorLow.Get());

	bool ShowBackground = pDisplayType->Text_Background;
	RectangleStruct rect = { 0,0,0,0 };
	DSurface::Temp->GetRect(&rect);
	COLORREF BackColor = 0;
	TextPrintType PrintType;
	int TextLength = wcslen(Healthpoint);

	if (int(pDisplayType->Anchoring) & int(DigitalDisplayTypeClass::AnchorType::Top))
		Pos.Y -= 20;

	if (Shield)
		Pos.Y -= 10;

	RectangleStruct recttmp = { Pos.X,Pos.Y,4,4 };
	DSurface::Composite->DrawRect(&recttmp, Drawing::RGB2DWORD(255, 0, 255));

	switch (pDisplayType->Alignment)
	{
	case DigitalDisplayTypeClass::AlignType::Left:
	{
		PrintType = TextPrintType::FullShadow;
		if (pDisplayType->Anchoring == DigitalDisplayTypeClass::AnchorType::Left)
			Pos.X -= TextLength * 6;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Right:
	{
		PrintType = TextPrintType(int(TextPrintType::Right) + int(TextPrintType::FullShadow));
		if (pDisplayType->Anchoring == DigitalDisplayTypeClass::AnchorType::Right)
			Pos.X += TextLength * 6;
		if (pThis->WhatAmI() == AbstractType::Building)
			Pos.X += 6;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Center:
	{
		PrintType = TextPrintType::Center;

		if (pThis->WhatAmI() == AbstractType::Building)
		{
			BuildingTypeClass* pBuildingType = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			int Height = pBuildingType->GetFoundationHeight(true);
			int iLength = Height * 7 + Height / 2;
			Pos.X += iLength * 4;
			Pos.Y -= iLength * 2 + wcslen(Healthpoint) * 2;
		}
	}
	default:
		if (pThis->WhatAmI() == AbstractType::Building)
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

	//0x400 is TextPrintType::Background pr#563 YRpp
	PrintType = TextPrintType(int(PrintType) + (ShowBackground ? 0x400 : 0));

	//DSurface::Temp->DrawText(Healthpoint, vPosH.X, vPosH.Y, ShowHPColor);
	DSurface::Temp->DrawTextA(Healthpoint, &rect, &Pos, HPColor, BackColor, PrintType);
}

void DigitalDisplayTypeClass::DigitalDisplaySHP(TechnoClass* pThis, DigitalDisplayTypeClass* pDisplayType, Point2D Pos, bool Shield)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	DynamicVectorClass<char>vStrength;
	DynamicVectorClass<char>vHealth;
	bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
	const Vector2D<int> Interval = (isBuilding ? pDisplayType->SHP_Interval_Building.Get() : pDisplayType->SHP_Interval.Get());
	SHPStruct* SHPFile = pDisplayType->SHPFile;
	ConvertClass* PALFile = pDisplayType->PALFile;
	bool Percentage = pDisplayType->Percentage.Get();
	bool HideStrength = pDisplayType->HideStrength.Get();
	int iLength = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;

	if (isBuilding)
	{
		BuildingTypeClass* pBuilding = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
		int Height = pBuilding->GetFoundationHeight(true);
		iLength = Height * 7 + Height / 2;
	}

	if (SHPFile == nullptr || PALFile == nullptr)
		return;

	if ((int(pDisplayType->Anchoring) & int(DigitalDisplayTypeClass::AnchorType::Top)))
		Pos.Y -= isBuilding ? 3 : 15;

	if (Shield)
		Pos.Y -= 2 + SHPFile->Height;

	if (Percentage)
	{
		if (Shield)
			vHealth = IntToVector(int(pExt->Shield->GetHealthRatio() * 100));
		else
			vHealth = IntToVector(int(pThis->GetHealthPercentage() * 100));
	}
	else
	{
		if (Shield)
		{
			vHealth = IntToVector(pExt->Shield->GetHP());

			if (!HideStrength)
				vStrength = IntToVector(pExt->Shield->GetType()->Strength.Get());
		}
		else
		{
			vHealth = IntToVector(pThis->Health);

			if (!HideStrength)
				vStrength = IntToVector(pThis->GetTechnoType()->Strength);
		}
	}

	bool LeftToRight = true;

	switch (pDisplayType->Alignment)
	{
	case DigitalDisplayTypeClass::AlignType::Left:
	{
		if (!isBuilding && pDisplayType->Anchoring == DigitalDisplayTypeClass::AnchorType::Left)
			Pos.X -= (vHealth.Count + vStrength.Count + (Percentage || !HideStrength)) * Interval.X + 2;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Right:
	{
		LeftToRight = false;
		if (!isBuilding && pDisplayType->Anchoring == DigitalDisplayTypeClass::AnchorType::Right)
			Pos.X += (vHealth.Count + vStrength.Count + (Percentage || !HideStrength)) * Interval.X + 2;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Center:
	{
		int FixValueX = 0;
		int FixValueY = 0;

		if (isBuilding)
			Pos.X += iLength * 2;
		else
			Pos.X += iLength;

		if (Percentage)
		{
			FixValueX = (vHealth.Count + 1) * Interval.X / 2;
			FixValueY = (vHealth.Count + 1) * Interval.Y / 2;
		}
		else if (HideStrength)
		{
			FixValueX = vHealth.Count * Interval.X / 2;
			FixValueY = vHealth.Count * Interval.Y / 2;
		}
		else
		{
			FixValueX = (vHealth.Count + vStrength.Count + 1) * Interval.X / 2;
			FixValueY = (vHealth.Count + vStrength.Count + 1) * Interval.Y / 2;
		}
		
		if (int(pDisplayType->Anchoring) & int(DigitalDisplayTypeClass::AnchorType::Right))
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
				FixValueX = (vHealth.Count + 1) * Interval.X / 2;
				FixValueY = (vHealth.Count + 1) * Interval.Y / 2;
			}
			else if (HideStrength)
			{
				FixValueX = vHealth.Count * Interval.X / 2;
				FixValueY = vHealth.Count * Interval.Y / 2;
			}
			else
			{
				FixValueX = (vHealth.Count + vStrength.Count + 1) * Interval.X / 2;
				FixValueY = (vHealth.Count + vStrength.Count + 1) * Interval.Y / 2;
			}

			if (int(pDisplayType->Anchoring) & int(DigitalDisplayTypeClass::AnchorType::Right))
				Pos.X += FixValueX;
			else
				Pos.X -= FixValueX;

			Pos.Y -= FixValueY;
		}
	}
	break;
	}

	RectangleStruct rect = { Pos.X,Pos.Y,4,4 };
	DSurface::Composite->DrawRect(&rect, Drawing::RGB2DWORD(255, 0, 0));

	int base = 0;
	int signframe = 30;

	if (pThis->IsYellowHP())
		base = 10;
	else if (pThis->IsRedHP())
		base = 20;

	if (base == 10)
		signframe = 31;
	else if (base == 20)
		signframe = 32;

	if (Percentage)
		signframe += 3;

	if (LeftToRight)
	{
		for (int i = vHealth.Count - 1; i >= 0; i--)
		{
			int num = base + vHealth.GetItem(i);

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

		for (int i = vStrength.Count - 1; i >= 0; i--)
		{
			int num = base + vStrength.GetItem(i);

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
			}

			for (int i = 0; i < vHealth.Count; i++)
			{
				int num = base + vHealth.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &Pos, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				Pos.X -= Interval.X;
				Pos.Y += Interval.Y;
			}
		}
		else
		{
			for (int i = 0; i < vStrength.Count; i++)
			{
				int num = base + vStrength.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &Pos, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				Pos.X -= Interval.X;
				Pos.Y += Interval.Y;
			}

			DSurface::Composite->DrawSHP(PALFile, SHPFile, signframe, &Pos, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			Pos.X -= Interval.X;
			Pos.Y += Interval.Y;

			for (int i = 0; i < vHealth.Count; i++)
			{
				int num = base + vHealth.GetItem(i);

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
