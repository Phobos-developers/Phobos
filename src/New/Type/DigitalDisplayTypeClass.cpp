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
	this->Align.Read(exINI, section, "Align");
	this->AnchorType.Read(exINI, section, "Anchor.%s");
	this->AnchorType_Building.Read(exINI, section, "Anchor.Building");
	this->Border.Read(exINI, section, "Border");
	this->UseShape.Read(exINI, section, "UseShape");
	this->Shape.Read(exINI, section, "Shape");
	this->Palette.Read(pINI, section, "Palette");
	this->Shape_Interval.Read(exINI, section, "Shape.Interval");
	this->Shape_Interval_Building.Read(exINI, section, "Shape.Interval.Building");
	this->Percentage.Read(exINI, section, "Percentage");
	this->HideMaxValue.Read(exINI, section, "HideMaxValue");
	this->InfoType.Read(exINI, section, "InfoType");

	if (UseShape)
	{
		if (Shape.Get() == nullptr)
			Shape = FileSystem::LoadSHPFile("number.shp");

		if (strcmp(Palette.data(), "") != 0)
			PALFile = FileSystem::LoadPALFile(Palette.data(), DSurface::Composite);
		else
			PALFile = FileSystem::PALETTE_PAL;
	}
}

DynamicVectorClass<__int8> IntToDigits(int num)
{
	DynamicVectorClass<__int8>res;
	
	if (num == 0)
	{
		res.AddItem(static_cast<__int8>(0));
		return res;
	}

	while (num)
	{
		res.AddItem(static_cast<__int8>(num % 10));
		num /= 10;
	}
	
	return res;
}

void DigitalDisplayTypeClass::Draw(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield)
{
	bool bShield = InfoType == DisplayInfoType::Shield;

	if (bShield)
	{
		posDraw.Y -= 6;

		if (isBuilding)
			posDraw.X -= 6;
	}

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

	if (UseShape)
	{
		if (isBuilding)
		{
			posDraw.X -= 4 + (bShield ? 0 : 6);
			posDraw.Y += 4;
		}
		else
		{
			posDraw.X += 1;
		}

		if (bShield)
			posDraw.Y -= Shape->Height - 4;

		DisplayShape(posDraw, iLength, iCur, iMax, isBuilding);
	}
	else
	{
		if (bShield)
			posDraw.Y -= 10;

		if (isBuilding)
			posDraw.X -= 4 + (bShield ? 0 : 6);

		DisplayText(posDraw, iLength, iCur, iMax, isBuilding);
	}
}

void DigitalDisplayTypeClass::DisplayText(Point2D& posDraw, int iLength, int iCur, int iMax, bool isBuilding)
{
	wchar_t Healthpoint[0x20];

	if (Percentage.Get())
	{
		swprintf_s(Healthpoint, L"%d", static_cast<int>((static_cast<double>(iCur) / iMax) * 100));
		wcscat_s(Healthpoint, L"%%");
	}
	else if (HideMaxValue.Get())
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

	if (Border == BorderPosition::Top)
		posDraw.Y -= 16;

	switch (Align)
	{
	case TextAlign::Left:
	{
		PrintType = TextPrintType::FullShadow;

		if (Border == BorderPosition::Left)
			posDraw.X -= textLength * 6;
	}
	break;
	case TextAlign::Right:
	{
		PrintType = TextPrintType::Right & TextPrintType::FullShadow;

		if (Border == BorderPosition::Right)
			posDraw.X += textLength * 6;
		
		if (isBuilding)
			posDraw.X += 6;
	}
	break;
	case TextAlign::Center:
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

	PrintType = PrintType | (ShowBackground ? TextPrintType::Background : TextPrintType::LASTPOINT);

	DSurface::Temp->DrawText(Healthpoint, &rect, &posDraw, Color, 0, PrintType);
}

void DigitalDisplayTypeClass::DisplayShape(Point2D& posDraw, int iLength, int iCur, int iMax, bool isBuilding)
{
	DynamicVectorClass<__int8> vCur;
	DynamicVectorClass<__int8> vMax;
	const Vector2D<int> Interval = (isBuilding ? Shape_Interval_Building.Get() : Shape_Interval.Get());
	
	if (Shape == nullptr || PALFile == nullptr)
		return;

	if (Border == BorderPosition::Top)
	{
		posDraw.Y -= isBuilding ? 14 : 8;
		posDraw.Y -= Shape->Height;
	}
	else if (Border == BorderPosition::Bottom)
	{
		posDraw.Y += isBuilding ? 0 : Shape->Height;
	}
	else if (!isBuilding && Border == BorderPosition::Left)
	{
		posDraw.X -= Interval.X;
	}

	if (Percentage.Get())
	{
		vCur = IntToDigits(static_cast<int>(static_cast<double>(iCur) / iMax * 100));
	}
	else
	{
		vCur = IntToDigits(iCur);

		if (!HideMaxValue)
			vMax = IntToDigits(iMax);
	}

	bool LeftToRight = true;

	switch (Align)
	{
	case TextAlign::Left:
	{
		if (Border == BorderPosition::Left)
			posDraw.X -= (vCur.Count + vMax.Count + (Percentage || !HideMaxValue) + 1) * Interval.X + 2;
	}
	break;
	case TextAlign::Right:
	{
		LeftToRight = false;

		if (Border == BorderPosition::Right)
			posDraw.X += (vCur.Count + vMax.Count + (Percentage || !HideMaxValue)) * Interval.X + 2;
	}
	break;
	case TextAlign::Center:
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
			FixValueX = (vCur.Count + 1) * Interval.X / 2;
			FixValueY = (vCur.Count + 1) * Interval.Y / 2;
		}
		else if (HideMaxValue)
		{
			FixValueX = vCur.Count * Interval.X / 2;
			FixValueY = vCur.Count * Interval.Y / 2;
		}
		else
		{
			FixValueX = (vCur.Count + vMax.Count + 1) * Interval.X / 2;
			FixValueY = (vCur.Count + vMax.Count + 1) * Interval.Y / 2;
		}

		if (AnchorType.Horizontal == HorizontalPosition::Right)
			posDraw.X += FixValueX;
		else if (AnchorType.Horizontal == HorizontalPosition::Left)
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
				FixValueX = (vCur.Count + 1) * Interval.X / 2;
				FixValueY = (vCur.Count + 1) * Interval.Y / 2;
			}
			else if (HideMaxValue)
			{
				FixValueX = vCur.Count * Interval.X / 2;
				FixValueY = vCur.Count * Interval.Y / 2;
			}
			else
			{
				FixValueX = (vCur.Count + vMax.Count + 1) * Interval.X / 2;
				FixValueY = (vCur.Count + vMax.Count + 1) * Interval.Y / 2;
			}

			if (AnchorType.Horizontal == HorizontalPosition::Right)
				posDraw.X += FixValueX;
			else if (AnchorType.Horizontal == HorizontalPosition::Left)
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
		for (int i = vCur.Count - 1; i >= 0; i--)
		{
			int num = base + vCur.GetItem(i);

			DSurface::Composite->DrawSHP(PALFile, Shape, num, &posDraw, &DSurface::ViewBounds,
				BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			posDraw.X += Interval.X;
			posDraw.Y += Interval.Y;
		}

		if (!Percentage && HideMaxValue)
			return;

		DSurface::Composite->DrawSHP(PALFile, Shape, signframe, &posDraw, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		posDraw.X += Interval.X;
		posDraw.Y += Interval.Y;

		if (Percentage)
			return;

		for (int i = vMax.Count - 1; i >= 0; i--)
		{
			int num = base + vMax.GetItem(i);

			DSurface::Composite->DrawSHP(PALFile, Shape, num, &posDraw, &DSurface::ViewBounds,
				BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			posDraw.X += Interval.X;
			posDraw.Y += Interval.Y;
		}
	}
	else
	{
		if (Percentage || HideMaxValue)
		{
			if (Percentage)
			{
				DSurface::Composite->DrawSHP(PALFile, Shape, signframe, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				posDraw.X -= Interval.X;
				posDraw.Y -= Interval.Y;
			}

			for (int i = 0; i < vCur.Count; i++)
			{
				int num = base + vCur.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, Shape, num, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				posDraw.X -= Interval.X;
				posDraw.Y -= Interval.Y;
			}
		}
		else
		{
			for (int i = 0; i < vMax.Count; i++)
			{
				int num = base + vMax.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, Shape, num, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				posDraw.X -= Interval.X;
				posDraw.Y -= Interval.Y;
			}

			DSurface::Composite->DrawSHP(PALFile, Shape, signframe, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			posDraw.X -= Interval.X;
			posDraw.Y -= Interval.Y;

			for (int i = 0; i < vCur.Count; i++)
			{
				int num = base + vCur.GetItem(i);

				DSurface::Composite->DrawSHP(PALFile, Shape, num, &posDraw, &DSurface::ViewBounds,
					BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
				posDraw.X -= Interval.X;
				posDraw.Y -= Interval.Y;
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
		.Process(this->Align)
		.Process(this->AnchorType)
		.Process(this->AnchorType_Building)
		.Process(this->Border)
		.Process(this->UseShape)
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Shape_Interval)
		.Process(this->Shape_Interval_Building)
		.Process(this->Percentage)
		.Process(this->HideMaxValue)
		.Process(this->InfoType)
		;
}

void DigitalDisplayTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);

	if (UseShape)
	{
		if (strcmp(Palette.data(), "") != 0)
			PALFile = FileSystem::LoadPALFile(Palette.data(), DSurface::Composite);
		else
			PALFile = FileSystem::PALETTE_PAL;
	}
}

void DigitalDisplayTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
