#include "DigitalDisplayTypeClass.h"

#include <TacticalClass.h>
#include <SpawnManagerClass.h>

#include <Utilities/ShapeTextPrinter.h>

#include <Ext/Techno/Body.h>

template<>
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
	this->Shape_Spacing.Read(exINI, section, "Shape.Spacing");
	this->Shape_PercentageFrame.Read(exINI, section, "Shape.PercentageFrame");
	this->Percentage.Read(exINI, section, "Percentage");
	this->HideMaxValue.Read(exINI, section, "HideMaxValue");
	this->VisibleToHouses_Observer.Read(exINI, section, "VisibleToHouses.Observer");
	this->VisibleToHouses.Read(exINI, section, "VisibleToHouses");
	this->InfoType.Read(exINI, section, "InfoType");
}

void DigitalDisplayTypeClass::Draw(Point2D position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield)
{
	position.X += Offset.Get().X;
	position.Y -= Offset.Get().Y;

	if (hasShield)
	{
		if (Offset_ShieldDelta.isset())
		{
			position.X += Offset_ShieldDelta.Get().X;
			position.Y -= Offset_ShieldDelta.Get().Y;
		}
		else if (InfoType == DisplayInfoType::Shield)
		{
			position.Y -= 10;	//default
		}
	}

	if (isBuilding)
	{
		if (AnchorType_Building == BuildingSelectBracketPosition::Top)
			position.Y -= 4; //Building's pips height
		else if (AnchorType_Building == BuildingSelectBracketPosition::LeftTop || AnchorType_Building == BuildingSelectBracketPosition::LeftBottom)
			position.X -= 8; //anchor to the left border of pips
	}

	if (Shape != nullptr)
		DisplayShape(position, length, value, maxValue, isBuilding, isInfantry, hasShield);
	else
		DisplayText(position, length, value, maxValue, isBuilding, isInfantry, hasShield);
}

void DigitalDisplayTypeClass::DisplayText(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield)
{
	wchar_t text[0x20];
	double ratio = static_cast<double>(value) / maxValue;

	if (Percentage.Get())
	{
		swprintf_s(text, L"%d", static_cast<int>(ratio * 100));
		wcscat_s(text, L"%%");
	}
	else if (HideMaxValue.Get(isInfantry))
	{
		swprintf_s(text, L"%d", value);
	}
	else
	{
		swprintf_s(text, L"%d/%d", value, maxValue);
	}

	COLORREF color = Drawing::RGB_To_Int(Text_Color.Get(ratio));
	RectangleStruct rect = { 0, 0, 0, 0 };
	DSurface::Composite->GetRect(&rect);
	rect.Height -= 32; // account for bottom bar
	const int textHeight = 12;
	const int pipsHeight = hasShield ? 4 : 0;

	if (AnchorType.Vertical == VerticalPosition::Top)
		position.Y -= textHeight + pipsHeight; // upper of healthbar and shieldbar

	TextPrintType printType = static_cast<TextPrintType>(Align.Get())
		| TextPrintType::FullShadow
		| (Text_Background ? TextPrintType::Background : TextPrintType::LASTPOINT);

	DSurface::Composite->DrawTextA(text, &rect, &position, color, 0, printType);
}

void DigitalDisplayTypeClass::DisplayShape(Point2D& position, int length, int value, int maxValue, bool isBuilding, bool isInfantry, bool hasShield)
{
	double ratio = static_cast<double>(value) / maxValue;
	std::string valueString(std::move(Percentage ?
		GeneralUtils::IntToDigits(static_cast<int>(ratio * 100)) :
		GeneralUtils::IntToDigits(value)
	));
	std::string maxValueString(!Percentage && !HideMaxValue.Get(isInfantry) ?
		std::move(GeneralUtils::IntToDigits(maxValue)) :
		""
	);
	Vector2D<int> spacing = (
		Shape_Spacing.isset() ?
		Shape_Spacing.Get() :
		(isBuilding ? Vector2D<int> { 4, -2 } : Vector2D<int> { 4, 0 }) // default
	);
	const int pipsHeight = hasShield ? 4 : 0;

	if (Percentage)
		valueString.push_back('%');
	else if (!HideMaxValue.Get(isInfantry))
		valueString += '/' + maxValueString;

	if (AnchorType.Vertical == VerticalPosition::Top)
		position.Y -= Shape->Height + pipsHeight; // upper of healthbar and shieldbar

	switch (Align)
	{
	case TextAlign::Left:
	{
		break;
	}
	case TextAlign::Center:
	{
		if (Shape_PercentageFrame)
		{
			position.X -= static_cast<int>(Shape->Width) / 2;
		}
		else
		{
			position.X -= static_cast<int>(valueString.length()) * spacing.X / 2;
			position.Y += static_cast<int>(valueString.length()) * spacing.Y / 2;
		}

		break;
	}
	case TextAlign::Right:
	{
		if (Shape_PercentageFrame)
			position.X -= static_cast<int>(Shape->Width);
		else
			position.X -= spacing.X;

		break;
	}
	}

	const int greenBaseFrame = 0;
	const int yellowBaseFrame = 10;
	const int redBaseFrame = 20;
	const int greenExtraFrame = 30;
	const int yellowExtraFrame = 32;
	const int redExtraFrame = 34;
	int numberBaseFrame = greenBaseFrame;
	int extraBaseFrame = greenExtraFrame;

	if (ratio > RulesClass::Instance->ConditionYellow)
		numberBaseFrame = greenBaseFrame;
	else if (ratio > RulesClass::Instance->ConditionRed)
		numberBaseFrame = yellowBaseFrame;
	else
		numberBaseFrame = redBaseFrame;

	if (numberBaseFrame == yellowBaseFrame)
		extraBaseFrame = yellowExtraFrame;
	else if (numberBaseFrame == redBaseFrame)
		extraBaseFrame = redExtraFrame;

	if (Align == TextAlign::Right)
	{
		std::reverse(valueString.begin(), valueString.end());
		spacing.X = -spacing.X;
	}

	ShapeTextPrintData shapeTextPrintData
	(
		Shape.Get(),
		Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL),
		numberBaseFrame,
		extraBaseFrame,
		spacing
	);

	RectangleStruct rect = DSurface::Composite->GetRect();
	rect.Height -= 32; // account for bottom bar

	if (Shape_PercentageFrame)
	{
		DSurface::Composite->DrawSHP
		(
			const_cast<ConvertClass*>(Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL)),
			const_cast<SHPStruct*>(Shape.Get()),
			static_cast<int>(Math::clamp(ratio, 0, 1) * (Shape->Frames - 1) + 0.5),
			&position, &rect, BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0
		);
	}
	else
	{
		ShapeTextPrinter::PrintShape(valueString.c_str(), shapeTextPrintData, position, rect, DSurface::Composite);
	}
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
		.Process(this->Shape_Spacing)
		.Process(this->Shape_PercentageFrame)
		.Process(this->Percentage)
		.Process(this->HideMaxValue)
		.Process(this->VisibleToHouses_Observer)
		.Process(this->VisibleToHouses)
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
