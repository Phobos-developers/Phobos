#include "DigitalDisplayTypeClass.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <Utilities/ShapeTextPrinter.h>

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

	this->Text_Color.Read(exINI, section, "Text.Color.%s");
	this->Text_Background.Read(exINI, section, "Text.Background");
	this->Offset.Read(exINI, section, "Offset");
	this->Offset_ShieldDelta.Read(exINI, section, "Offset.ShieldDelta");
	this->Align.Read(exINI, section, "Align");
	this->AnchorType.Read(exINI, section, "Anchor.%s");
	this->AnchorType_Building.Read(exINI, section, "Anchor.Building");
	this->Border.Read(exINI, section, "Border");
	this->Shape.Read(exINI, section, "Shape");
	this->Palette.LoadFromINI(pINI, section, "Palette");
	this->Shape_Interval.Read(exINI, section, "Shape.Interval");
	this->Percentage.Read(exINI, section, "Percentage");
	this->HideMaxValue.Read(exINI, section, "HideMaxValue");
	this->InfoType.Read(exINI, section, "InfoType");
}

void DigitalDisplayTypeClass::Draw(Point2D posDraw, int iLength, int iCur, int iMax, bool isBuilding, bool hasShield)
{
	posDraw += Offset;

	if (hasShield)
	{
		if (Offset_ShieldDelta.isset())
			posDraw += Offset_ShieldDelta;
		else if (InfoType == DisplayInfoType::Shield)
			posDraw.Y -= 10;	//default
	}

	if (Shape != nullptr)
		DisplayShape(posDraw, iLength, iCur, iMax, isBuilding);
	else
		DisplayText(posDraw, iLength, iCur, iMax, isBuilding);
}

void DigitalDisplayTypeClass::DisplayText(Point2D& posDraw, int iLength, int iCur, int iMax, bool isBuilding)
{
	wchar_t text[0x20];

	if (Percentage.Get())
	{
		swprintf_s(text, L"%d", static_cast<int>((static_cast<double>(iCur) / iMax) * 100));
		wcscat_s(text, L"%%");
	}
	else if (HideMaxValue.Get())
	{
		swprintf_s(text, L"%d", iCur);
	}
	else
	{
		swprintf_s(text, L"%d/%d", iCur, iMax);
	}

	double ratio = static_cast<double>(iCur) / iMax;
	COLORREF color = Drawing::RGB2DWORD(Text_Color.Get(ratio));
	bool ShowBackground = Text_Background;
	RectangleStruct rect = { 0, 0, 0, 0 };
	DSurface::Temp->GetRect(&rect);
	TextPrintType ePrintType;
	int textLength = wcslen(text);
	const int iTextHeight = 12;
	const int iPipHeight = 4;
	const int iBuildingPipWidth = 4;
	const int iBuildingPipHeight = 2;
	const int iTextWidth = 6;

	if (Border == BorderPosition::Top)
		posDraw.Y -= iTextHeight + iPipHeight;

	switch (Align)
	{
	case TextAlign::Left:
	{
		ePrintType = TextPrintType::FullShadow;

		if (Border == BorderPosition::Left)
			posDraw.X -= textLength * iTextWidth;
	}
	break;
	case TextAlign::Right:
	{
		ePrintType = TextPrintType::Right & TextPrintType::FullShadow;

		if (Border == BorderPosition::Right)
			posDraw.X += textLength * iTextWidth;
	}
	break;
	case TextAlign::Center:
	{
		ePrintType = TextPrintType::Center;
	}
	default:
		if (isBuilding)
		{
			ePrintType = TextPrintType::Right;
		}
		else
		{
			ePrintType = TextPrintType::Center;
			posDraw.X += iLength;
		}
		break;
	}

	ePrintType = ePrintType | (ShowBackground ? TextPrintType::Background : TextPrintType::LASTPOINT);

	DSurface::Temp->DrawTextA(text, &rect, &posDraw, color, 0, ePrintType);
}

void DigitalDisplayTypeClass::DisplayShape(Point2D& posDraw, int iLength, int iCur, int iMax, bool isBuilding)
{
	std::string sCur;
	std::string sMax;
	Vector2D<int> vInterval = (
		Shape_Interval.isset() ?
		Shape_Interval.Get() :
		(isBuilding ? Vector2D<int> { 8, -4 } : Vector2D<int> { 8, 0 }) // default
	);
	
	if (Border == BorderPosition::Top)
		posDraw.Y -= Shape->Height * 2;	// upper of healthbar and shieldbar
	else if (!isBuilding && Border == BorderPosition::Left)
		posDraw.X -= vInterval.X;	// DrawSHP use pos for LeftTop of shape bounds

	if (isBuilding)
		posDraw.X -= 10; // aligned to healthbar left 

	if (Percentage)
	{
		sCur = std::move(GeneralUtils::IntToDigits(static_cast<int>(static_cast<double>(iCur) / iMax * 100)));
	}
	else
	{
		sCur = std::move(GeneralUtils::IntToDigits(iCur));

		if (!HideMaxValue)
			sMax = std::move(GeneralUtils::IntToDigits(iMax));
	}

	bool bLeftToRight = true;

	switch (Align)
	{
	case TextAlign::Left:
	{
		if (Border == BorderPosition::Left)
			posDraw.X -= (sCur.length() + sMax.length() + (Percentage || !HideMaxValue) + 1) * vInterval.X + 2;
	}
	break;
	case TextAlign::Right:
	{
		bLeftToRight = false;

		if (Border == BorderPosition::Right)
			posDraw.X += (sCur.length() + sMax.length() + (Percentage || !HideMaxValue)) * vInterval.X + 2;
	}
	break;
	case TextAlign::Center:
	{
		int iFixX = 0;
		int iFixY = 0;

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
			iFixX = (sCur.length() + 1) * vInterval.X / 2;
			iFixY = (sCur.length() + 1) * vInterval.Y / 2;
		}
		else if (HideMaxValue)
		{
			iFixX = sCur.length() * vInterval.X / 2;
			iFixY = sCur.length() * vInterval.Y / 2;
		}
		else
		{
			iFixX = (sCur.length() + sMax.length() + 1) * vInterval.X / 2;
			iFixY = (sCur.length() + sMax.length() + 1) * vInterval.Y / 2;
		}

		if (AnchorType.Horizontal == HorizontalPosition::Right)
			posDraw.X += iFixX;
		else if (AnchorType.Horizontal == HorizontalPosition::Left)
			posDraw.X -= iFixX;

		posDraw.Y -= iFixY;
	}
	break;
	default:
	{
		if (!isBuilding)
		{
			posDraw.X += iLength;

			int iFixX = 0;
			int iFixY = 0;

			// +1 for sign
			if (Percentage)
			{
				iFixX = (sCur.length() + 1) * vInterval.X / 2;
				iFixY = (sCur.length() + 1) * vInterval.Y / 2;
			}
			else if (HideMaxValue)
			{
				iFixX = sCur.length() * vInterval.X / 2;
				iFixY = sCur.length() * vInterval.Y / 2;
			}
			else
			{
				iFixX = (sCur.length() + sMax.length() + 1) * vInterval.X / 2;
				iFixY = (sCur.length() + sMax.length() + 1) * vInterval.Y / 2;
			}

			if (AnchorType.Horizontal == HorizontalPosition::Right)
				posDraw.X += iFixX;
			else if (AnchorType.Horizontal == HorizontalPosition::Left)
				posDraw.X -= iFixX;

			posDraw.Y -= iFixY;
		}
	}
	break;
	}

	const int iGreenNumberBaseFrame = 0;
	const int iYellowNumberBaseFrame = 10;
	const int iRedNumberBaseFrame = 20;
	const int iGreenSignBaseFrame = 30;
	const int iYellowSignBaseFrame = 32;
	const int iRedSignBaseFrame = 34;

	int iNumberBaseFrame = iGreenNumberBaseFrame;
	int iSignBaseFrame = iGreenSignBaseFrame;
	double ratio = static_cast<double>(iCur) / iMax;

	if (ratio > RulesClass::Instance->ConditionYellow)
		iNumberBaseFrame = iGreenNumberBaseFrame;
	else if (ratio > RulesClass::Instance->ConditionRed)
		iNumberBaseFrame = iYellowNumberBaseFrame;
	else
		iNumberBaseFrame = iRedNumberBaseFrame;

	if (iNumberBaseFrame == iYellowNumberBaseFrame)
		iSignBaseFrame = iYellowSignBaseFrame;
	else if (iNumberBaseFrame == iRedNumberBaseFrame)
		iSignBaseFrame = iRedSignBaseFrame;

	std::string text = sCur;

	if (Percentage)
		text.push_back('%');
	else if (!HideMaxValue)
		text += '/' + sMax;

	if (!bLeftToRight)
	{
		std::reverse(text.begin(), text.end());
		vInterval.X = -vInterval.X;
	}

	ShapeTextPrintData shapeTextPrintData
	(
		Shape.Get(),
		Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL),
		iNumberBaseFrame,
		iSignBaseFrame,
		vInterval
	);
	RectangleStruct rBound;
	DSurface::Temp->GetRect(&rBound);
	ShapeTextPrinter::PrintShape(text.c_str(), shapeTextPrintData, posDraw, rBound, DSurface::Temp);
}


template <typename T>
void DigitalDisplayTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Text_Color)
		.Process(this->Text_Background)
		.Process(this->Offset)
		.Process(this->Offset_ShieldDelta)
		.Process(this->Align)
		.Process(this->AnchorType)
		.Process(this->AnchorType_Building)
		.Process(this->Border)
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Shape_Interval)
		.Process(this->Percentage)
		.Process(this->HideMaxValue)
		.Process(this->InfoType)
		;
}

void DigitalDisplayTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void DigitalDisplayTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
