#include "DigitalDisplayTypeClass.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

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

DynamicVectorClass<char> IntToVector(int num)
{
	DynamicVectorClass<char>res;
	if (num == 0)
	{
		res.AddItem(0);
		return res;
	}
	while (num)
	{
		res.AddItem(num % 10);
		num /= 10;
	}
	return res;
}

int operator & (DigitalDisplayTypeClass::AnchorType a, DigitalDisplayTypeClass::AnchorType b)
{
	return static_cast<int>(a) & static_cast<int>(b);
}

void DigitalDisplayTypeClass::Draw(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield)
{
	bool Shield = InfoClass == Info::Shield;

	if (!hasShield)
	{
		posDraw.X += Offset_WithoutShield.Get(Offset.Get()).X;
		posDraw.Y += Offset_WithoutShield.Get(Offset.Get()).Y;
	}
	else
	{
		posDraw.X += Offset.Get().X;
		posDraw.Y += Offset.Get().Y;
	}

	if (UseSHP)
	{
		if (isBuilding)
			posDraw.X -= 4 + (!Shield ? 6 : 0);

		if (Shield)
			posDraw.Y -= SHPFile->Height - 3;

		DisplayShape(posDraw, iLength, iCur, iMax, isBuilding, hasShield);
	}
	else
	{
		if (Shield)
			posDraw.Y -= 10;

		if (isBuilding)
			posDraw.X -= 4 + (!Shield ? 6 : 0);

		DisplayText(posDraw, iLength, iCur, iMax, isBuilding, hasShield);
	}
}

void DigitalDisplayTypeClass::DisplayText(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield)
{
	wchar_t Healthpoint[0x20];

	if (Percentage.Get())
	{
		swprintf_s(Healthpoint, L"%d", static_cast<int>((static_cast<double>(iCur) / iMax) * 100));
		wcscat_s(Healthpoint, L"%%");
	}
	else if (HideStrength.Get())
	{
		swprintf_s(Healthpoint, L"%d", iCur);
	}
	else
	{
		swprintf_s(Healthpoint, L"%d/%d", iCur, iMax);
	}

	COLORREF Color;
	double ratio = static_cast<double>(iCur) / iMax;

	if (ratio > RulesClass::Instance->ConditionYellow)
		Color = Drawing::RGB2DWORD(Text_ColorHigh.Get());
	else if (ratio > RulesClass::Instance->ConditionRed)
		Color = Drawing::RGB2DWORD(Text_ColorMid.Get());
	else
		Color = Drawing::RGB2DWORD(Text_ColorLow.Get());

	bool ShowBackground = Text_Background;
	RectangleStruct rect = { 0,0,0,0 };
	DSurface::Temp->GetRect(&rect);
	TextPrintType PrintType;
	int textLength = wcslen(Healthpoint);

	if (Anchoring & DigitalDisplayTypeClass::AnchorType::Top)
		posDraw.Y -= 20;

	switch (Alignment)
	{
	case DigitalDisplayTypeClass::AlignType::Left:
	{
		PrintType = TextPrintType::FullShadow;
		if (Anchoring == DigitalDisplayTypeClass::AnchorType::Left)
			posDraw.X -= textLength * 6;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Right:
	{
		PrintType = TextPrintType(int(TextPrintType::Right) + int(TextPrintType::FullShadow));
		if (Anchoring == DigitalDisplayTypeClass::AnchorType::Right)
			posDraw.X += textLength * 6;
		if (isBuilding)
			posDraw.X += 6;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Center:
	{
		PrintType = TextPrintType::Center;

		if (isBuilding)
		{
			posDraw.X += iLength * 4;
			posDraw.Y -= iLength * 2 + wcslen(Healthpoint) * 2;
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
			posDraw.X += iLength;
		}
		break;
	}

	if (Anchoring & DigitalDisplayTypeClass::AnchorType::Top)
		posDraw.Y -= 4;

	//0x400 is TextPrintType::Background pr#563 YRpp
	PrintType = PrintType | (ShowBackground ? TextPrintType::Background : TextPrintType::LASTPOINT);

	//DSurface::Temp->DrawText(Healthpoint, vPosH.X, vPosH.Y, ShowHPColor);
	DSurface::Temp->DrawTextA(Healthpoint, &rect, &posDraw, Color, 0, PrintType);
}

void DigitalDisplayTypeClass::DisplayShape(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield)
{
	DynamicVectorClass<char>vA;
	DynamicVectorClass<char>vB;
	const Vector2D<int> Interval = (isBuilding ? SHP_Interval_Building.Get() : SHP_Interval.Get());

	if (SHPFile == nullptr || PALFile == nullptr)
		return;

	if (Anchoring & DigitalDisplayTypeClass::AnchorType::Top)
	{
		posDraw.Y -= isBuilding ? 14 : 8;
		posDraw.Y -= SHPFile->Height;
	}

	if (Percentage.Get())
	{
		vA = IntToVector(static_cast<int>(static_cast<double>(iCur) / iMax * 100));
	}
	else
	{
		vA = IntToVector(iCur);

		if (!HideStrength.Get())
			vB = IntToVector(iMax);
	}

	bool LeftToRight = true;

	switch (Alignment)
	{
	case DigitalDisplayTypeClass::AlignType::Left:
	{
		if (Anchoring == DigitalDisplayTypeClass::AnchorType::Left)
			posDraw.X -= (vA.Count + vB.Count + (Percentage || !HideStrength)) * Interval.X + 2;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Right:
	{
		LeftToRight = false;
		if (Anchoring == DigitalDisplayTypeClass::AnchorType::Right)
			posDraw.X += (vA.Count + vB.Count + (Percentage || !HideStrength)) * Interval.X + 2;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Center:
	{
		int FixValueX = 0;
		int FixValueY = 0;

		if (isBuilding)
		{
			posDraw.X += iLength * 2;
			posDraw.Y += iLength;
		}
		else
		{
			posDraw.X += iLength;
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
			posDraw.X += FixValueX;
		else
			posDraw.X -= FixValueX;

		posDraw.Y -= FixValueY;
	}
	break;
	default:
	{
		if (!isBuilding)
		{
			int FixValueX = 0;
			int FixValueY = 0;

			posDraw.X += iLength;

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
				posDraw.X += FixValueX;
			else
				posDraw.X -= FixValueX;

			posDraw.Y -= FixValueY;
		}
	}
	break;
	}

	int base = 0;
	int signframe = 30;
	double ratio = static_cast<double>(iCur) / iMax;

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

			DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &posDraw, &DSurface::ViewBounds,
				BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			posDraw.X += Interval.X;
			posDraw.Y -= Interval.Y;
		}

		if (!Percentage && HideStrength)
			return;

		DSurface::Composite->DrawSHP(PALFile, SHPFile, signframe, &posDraw, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		posDraw.X += Interval.X;
		posDraw.Y -= Interval.Y;

		if (Percentage)
			return;

		for (int i = vB.Count - 1; i >= 0; i--)
		{
			int num = base + vB.GetItem(i);

			DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &posDraw, &DSurface::ViewBounds,
				BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			posDraw.X += Interval.X;
			posDraw.Y -= Interval.Y;
		}
	}
	else
	{
		if (Percentage || HideStrength)
		{
			if (Percentage)
			{
				DSurface::Composite->DrawSHP(PALFile, SHPFile, signframe, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				posDraw.X -= Interval.X;
				posDraw.Y += Interval.Y;
			}

			for (int i = 0; i < vA.Count; i++)
			{
				int num = base + vA.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				posDraw.X -= Interval.X;
				posDraw.Y += Interval.Y;
			}
		}
		else
		{
			for (int i = 0; i < vB.Count; i++)
			{
				int num = base + vB.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				posDraw.X -= Interval.X;
				posDraw.Y += Interval.Y;
			}

			DSurface::Composite->DrawSHP(PALFile, SHPFile, signframe, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			posDraw.X -= Interval.X;
			posDraw.Y += Interval.Y;

			for (int i = 0; i < vA.Count; i++)
			{
				int num = base + vA.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				posDraw.X -= Interval.X;
				posDraw.Y += Interval.Y;
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
