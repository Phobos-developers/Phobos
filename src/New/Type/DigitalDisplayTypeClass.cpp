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
	this->Shape.Read(exINI, section, "Shape");
	this->Palette.LoadFromINI(pINI, section, "Palette");
	this->Shape_Interval.Read(exINI, section, "Shape.Interval");
	this->Percentage.Read(exINI, section, "Percentage");
	this->HideMaxValue.Read(exINI, section, "HideMaxValue");
	this->CanSee_Observer.Read(exINI, section, "CanSee.Observer");
	this->CanSee.Read(exINI, section, "CanSee");
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

	if (isBuilding)
	{
		if (AnchorType_Building == BuildingSelectBracketPosition::Top)
			posDraw.Y -= 4; //Building's pips height
		else if (AnchorType_Building == BuildingSelectBracketPosition::LeftTop || AnchorType_Building == BuildingSelectBracketPosition::LeftBottom)
			posDraw.X -= 8; //anchor to the left border of pips
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
	COLORREF color = Drawing::RGB_To_Int(Text_Color.Get(ratio));
	RectangleStruct rect = { 0, 0, 0, 0 };
	DSurface::Composite->GetRect(&rect);
	TextPrintType ePrintType;
	const int iTextHeight = 12;
	const int iPipHeight = 4;
	const int iBuildingPipHeight = 8;

	if (AnchorType.Vertical == VerticalPosition::Top)
		posDraw.Y -= iTextHeight + iPipHeight * 2; // upper of healthbar and shieldbar

	ePrintType = (Align == TextAlign::None ? (isBuilding ? TextPrintType::Right : TextPrintType::Center) : static_cast<TextPrintType>(Align.Get()))
		| TextPrintType::FullShadow
		| (Text_Background ? TextPrintType::Background : TextPrintType::LASTPOINT);

	DSurface::Composite->DrawTextA(text, &rect, &posDraw, color, 0, ePrintType);
}

void DigitalDisplayTypeClass::DisplayShape(Point2D& posDraw, int iLength, int iCur, int iMax, bool isBuilding)
{
	std::string sCur(std::move(Percentage ?
		GeneralUtils::IntToDigits(static_cast<int>(static_cast<double>(iCur) / iMax * 100)) :
		GeneralUtils::IntToDigits(iCur)
	));
	std::string sMax(!Percentage && !HideMaxValue ?
		std::move(GeneralUtils::IntToDigits(iMax)) :
		""
	);
	Vector2D<int> vInterval = (
		Shape_Interval.isset() ?
		Shape_Interval.Get() :
		(isBuilding ? Vector2D<int> { 8, -4 } : Vector2D<int> { 8, 0 }) // default
	);
	std::string text = sCur;
	const int iPipHeight = 4;
	const int iBuildingPipHeight = 8;

	if (Percentage)
		text.push_back('%');
	else if (!HideMaxValue)
		text += '/' + sMax;

	if (AnchorType.Vertical == VerticalPosition::Top)
		posDraw.Y -= Shape->Height + iPipHeight * 2; // upper of healthbar and shieldbar

	switch (Align)
	{
	case TextAlign::Left:
	{

	}break;
	case TextAlign::Center:
	{
		posDraw.X -= text.length() * vInterval.X / 2;
		posDraw.Y -= text.length() * vInterval.Y / 2;
	}break;
	case TextAlign::Right:
	{
		posDraw.X -= vInterval.X;
	}break;
	default:
	{
		if (!isBuilding)
		{
			posDraw.X -= text.length() * vInterval.X / 2;
			posDraw.Y -= text.length() * vInterval.Y / 2;
		}
	}break;
	}

	bool bLeftToRight = Align != TextAlign::Right;
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
	RectangleStruct rect = DSurface::Composite->GetRect();
	ShapeTextPrinter::PrintShape(text.c_str(), shapeTextPrintData, posDraw, rect, DSurface::Composite);
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
		.Process(this->Shape)
		.Process(this->Palette)
		.Process(this->Shape_Interval)
		.Process(this->Percentage)
		.Process(this->HideMaxValue)
		.Process(this->CanSee_Observer)
		.Process(this->CanSee)
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
