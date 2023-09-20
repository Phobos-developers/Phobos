#include "HugeBar.h"

#include <HouseClass.h>
#include <TechnoClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/ShapeTextPrinter.h>
#include <Utilities/TemplateDef.h>

std::vector<TechnoClass*> HugeBar::Technos;

HugeBar::HugeBar(DisplayInfoType infoType)
	: HugeBar_RectWidthPercentage(0.82)
	, HugeBar_RectWH({ -1, 30 })
	, HugeBar_Frame(-1)
	, HugeBar_Pips_Frame(-1)
	, HugeBar_Pips_Num(100)
	, Value_Shape_Spacing(8)
	, Value_Num_BaseFrame(0)
	, Value_Sign_BaseFrame(30)
	, DisplayValue(true)
	, Anchor(HorizontalPosition::Center, VerticalPosition::Top)
	, InfoType(infoType)
	, VisibleToHouses(AffectedHouse::All)
	, VisibleToHouses_Observer(true)
{
	switch (infoType)
	{
	case DisplayInfoType::Health:
		HugeBar_Pips_Color1 = Damageable<ColorStruct>({ 0, 255, 0 }, { 255, 255, 0 }, { 255, 0, 0 });
		HugeBar_Pips_Color2 = Damageable<ColorStruct>({ 0, 216, 0 }, { 255, 180, 0 }, { 216, 0, 0 });
		Value_Text_Color = Damageable<ColorStruct>({ 0, 255, 0 }, { 255, 180, 0 }, { 255, 0, 0 });
		break;
	case DisplayInfoType::Shield:
		HugeBar_Pips_Color1 = Damageable<ColorStruct>({ 0, 0, 255 });
		HugeBar_Pips_Color2 = Damageable<ColorStruct>({ 0, 0, 216 });
		Value_Text_Color = Damageable<ColorStruct>({ 0, 0, 216 });
		break;
	default:
		break;
	}
}

void HugeBar::LoadFromINI(CCINIClass* pINI)
{
	char typeName[0x20];

	switch (InfoType)
	{
	case DisplayInfoType::Health:
		strcpy_s(typeName, "Health");
		break;
	case DisplayInfoType::Shield:
		strcpy_s(typeName, "Shield");
		break;
	default:
		return;
	}

	char section[0x20];
	sprintf_s(section, "HugeBar_%s", typeName);
	INI_EX exINI(pINI);

	this->HugeBar_RectWidthPercentage.Read(exINI, section, "HugeBar.RectWidthPercentage");
	this->HugeBar_RectWH.Read(exINI, section, "HugeBar.RectWH");
	this->HugeBar_Pips_Color1.Read(exINI, section, "HugeBar.Pips.Color1.%s");
	this->HugeBar_Pips_Color2.Read(exINI, section, "HugeBar.Pips.Color2.%s");

	this->HugeBar_Shape.Read(exINI, section, "HugeBar.Shape");
	this->HugeBar_Palette.LoadFromINI(pINI, section, "HugeBar.Palette");
	this->HugeBar_Frame.Read(exINI, section, "HugeBar.Frame.%s");
	this->HugeBar_Pips_Shape.Read(exINI, section, "HugeBar.Pips.Shape");
	this->HugeBar_Pips_Palette.LoadFromINI(pINI, section, "HugeBar.Pips.Palette");
	this->HugeBar_Pips_Frame.Read(exINI, section, "HugeBar.Pips.Frame.%s");
	this->HugeBar_Pips_Spacing.Read(exINI, section, "HugeBar.Pips.Spacing");

	this->HugeBar_Offset.Read(exINI, section, "HugeBar.Offset");
	this->HugeBar_Pips_Offset.Read(exINI, section, "HugeBar.Pips.Offset");
	this->HugeBar_Pips_Num.Read(exINI, section, "HugeBar.Pips.Num");

	this->Value_Text_Color.Read(exINI, section, "Value.Text.Color.%s");

	this->Value_Shape.Read(exINI, section, "Value.Shape");
	this->Value_Palette.LoadFromINI(pINI, section, "Value.Palette");
	this->Value_Num_BaseFrame.Read(exINI, section, "Value.Num.BaseFrame");
	this->Value_Sign_BaseFrame.Read(exINI, section, "Value.Sign.BaseFrame");
	this->Value_Shape_Spacing.Read(exINI, section, "Value.Shape.Spacing");

	this->DisplayValue.Read(exINI, section, "DisplayValue");
	this->Value_Offset.Read(exINI, section, "Value.Offset");
	this->Value_Percentage.Read(exINI, section, "Value.Percentage");
	this->Anchor.Read(exINI, section, "Anchor.%s");

	this->VisibleToHouses.Read(exINI, section, "VisibleToHouses");
	this->VisibleToHouses_Observer.Read(exINI, section, "VisibleToHouses.Observer");
}

void HugeBar::InvalidatePointer(void* ptr, bool removed)
{
	if (removed)
	{
		const auto it = std::find(Technos.cbegin(), Technos.cend(), ptr);

		if (it != Technos.end())
			Technos.erase(it);
	}
}

template <typename T>
bool HugeBar::Serialize(T& stm)
{
	return stm
		.Process(this->HugeBar_RectWidthPercentage)
		.Process(this->HugeBar_RectWH)
		.Process(this->HugeBar_Pips_Color1)
		.Process(this->HugeBar_Pips_Color2)

		.Process(this->HugeBar_Shape)
		.Process(this->HugeBar_Palette)
		.Process(this->HugeBar_Frame)
		.Process(this->HugeBar_Pips_Shape)
		.Process(this->HugeBar_Pips_Palette)
		.Process(this->HugeBar_Pips_Frame)
		.Process(this->HugeBar_Pips_Spacing)

		.Process(this->HugeBar_Offset)
		.Process(this->HugeBar_Pips_Offset)
		.Process(this->HugeBar_Pips_Num)

		.Process(this->Value_Text_Color)

		.Process(this->Value_Shape)
		.Process(this->Value_Palette)
		.Process(this->Value_Num_BaseFrame)
		.Process(this->Value_Sign_BaseFrame)
		.Process(this->Value_Shape_Spacing)

		.Process(this->DisplayValue)
		.Process(this->Value_Offset)
		.Process(this->Value_Percentage)
		.Process(this->Anchor)
		.Process(this->InfoType)

		.Process(this->VisibleToHouses)
		.Process(this->VisibleToHouses_Observer)

		.Success()
		;
}

bool HugeBar::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool HugeBar::Save(PhobosStreamWriter& stm) const
{
	return const_cast<HugeBar*>(this)->Serialize(stm);
}

bool HugeBar::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(Technos)
		.Success();
}

bool HugeBar::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(Technos)
		.Success();
}

void HugeBar::InitializeHugeBar(TechnoClass* pTechno)
{
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

	if (pTypeExt->HugeBar)
	{
		const auto it = std::find(Technos.cbegin(), Technos.cend(), pTechno);

		if (it != Technos.cend())
			Technos.emplace_back(pTechno);
	}
}

void HugeBar::ProcessHugeBar()
{
	if (Technos.empty())
		return;

	const auto& configs = RulesExt::Global()->HugeBar_Config;

	for (size_t i = 0; i < configs.size(); i++)
	{
		TechnoClass* pTechno = nullptr;
		int priority = MIN(int);

		for (TechnoClass* pTmpTechno : Technos)
		{
			if (HouseClass::IsCurrentPlayerObserver() && !configs[i]->VisibleToHouses_Observer)
				continue;

			if (!HouseClass::IsCurrentPlayerObserver()
				&& !EnumFunctions::CanTargetHouse(
					configs[i]->VisibleToHouses,
					pTmpTechno->GetOwningHouse(),
					HouseClass::CurrentPlayer))
				continue;

			const auto pTmpTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTmpTechno->GetTechnoType());

			if (pTmpTechnoTypeExt->HugeBar_Priority > priority)
			{
				priority = pTmpTechnoTypeExt->HugeBar_Priority;
				pTechno = pTmpTechno;
			}
		}

		if (pTechno == nullptr)
			continue;

		int iCurrent = -1;
		int iMax = -1;
		TechnoExt::GetValuesForDisplay(pTechno, configs[i]->InfoType, iCurrent, iMax);

		if (iCurrent != -1)
			configs[i]->DrawHugeBar(iCurrent, iMax);
	}
}

void HugeBar::DrawHugeBar(int iCurrent, int iMax)
{
	double ratio = static_cast<double>(iCurrent) / iMax;
	int iPipNumber = std::max(static_cast<int>(ratio * this->HugeBar_Pips_Num), (iCurrent == 0 ? 0 : 1));
	RectangleStruct rectDraw = DSurface::Composite->GetRect();
	rectDraw.Height -= 32;
	Point2D posDraw = this->HugeBar_Offset.Get() + this->Anchor.OffsetPosition(rectDraw);
	Point2D posDrawValue = posDraw;
	RectangleStruct rBound = std::move(DSurface::Composite->GetRect());

	if (this->HugeBar_Shape != nullptr
		&& this->HugeBar_Pips_Shape != nullptr
		&& this->HugeBar_Frame.Get(ratio) >= 0
		&& this->HugeBar_Pips_Frame.Get(ratio) >= 0)
	{
		SHPStruct* pShp_Bar = this->HugeBar_Shape;
		ConvertClass* pPal_Bar = this->HugeBar_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
		SHPStruct* pShp_Pips = this->HugeBar_Pips_Shape;
		ConvertClass* pPal_Pips = this->HugeBar_Pips_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
		int iPipFrame = this->HugeBar_Pips_Frame.Get(ratio);

		switch (this->Anchor.Horizontal)
		{
		case HorizontalPosition::Left:
			posDrawValue.X += pShp_Bar->Width / 2;
			break;

		case HorizontalPosition::Center:
			posDrawValue.X = posDraw.X;
			posDraw.X -= pShp_Bar->Width / 2;
			break;

		case HorizontalPosition::Right:
			posDraw.X -= pShp_Bar->Width;
			posDrawValue.X -= pShp_Bar->Width / 2;
			break;

		default:
			break;
		}

		switch (this->Anchor.Vertical)
		{
		case VerticalPosition::Top:
			posDrawValue.Y += pShp_Bar->Height;
			break;

		case VerticalPosition::Center:
			posDraw.Y -= pShp_Bar->Height / 2;
			posDrawValue.Y += pShp_Bar->Height;
			break;

		case VerticalPosition::Bottom:
			posDraw.Y -= pShp_Bar->Height;
			posDrawValue.Y -= pShp_Bar->Height;
			break;

		default:
			break;
		}

		DSurface::Composite->DrawSHP
		(
			pPal_Bar,
			pShp_Bar,
			this->HugeBar_Frame.Get(ratio),
			&posDraw,
			&rBound,
			BlitterFlags::None,
			0,
			0,
			ZGradient::Ground,
			1000,
			0,
			nullptr,
			0,
			0,
			0
		);

		posDraw += this->HugeBar_Pips_Offset.Get();

		for (int i = 0; i < iPipNumber; i++)
		{
			DSurface::Composite->DrawSHP
			(
				pPal_Pips,
				pShp_Pips,
				iPipFrame,
				&posDraw,
				&rBound,
				BlitterFlags::None,
				0,
				0,
				ZGradient::Ground,
				1000,
				0,
				nullptr,
				0,
				0,
				0
			);

			posDraw.X += this->HugeBar_Pips_Spacing;
		}
	}
	else
	{
		COLORREF color1 = Drawing::RGB_To_Int(this->HugeBar_Pips_Color1.Get(ratio));
		COLORREF color2 = Drawing::RGB_To_Int(this->HugeBar_Pips_Color2.Get(ratio));
		Vector2D<int> rectWH = this->HugeBar_RectWH;

		if (rectWH.X < 0)
		{
			rectWH.X = static_cast<int>(this->HugeBar_RectWidthPercentage * rBound.Width);
			// make sure width is even
			rectWH.X += rectWH.X & 1;
		}

		switch (this->Anchor.Horizontal)
		{
		case HorizontalPosition::Left:
			posDrawValue.X += rectWH.X / 2;
			break;

		case HorizontalPosition::Center:
			posDrawValue.X = posDraw.X;
			posDraw.X -= rectWH.X / 2;
			break;

		case HorizontalPosition::Right:
			posDraw.X -= rectWH.X;
			posDrawValue.X -= rectWH.X / 2;
			break;

		default:
			break;
		}

		switch (this->Anchor.Vertical)
		{
		case VerticalPosition::Top:
			posDrawValue.Y += rectWH.Y;
			break;

		case VerticalPosition::Center:
			posDraw.Y -= rectWH.Y / 2;
			posDrawValue.Y += rectWH.Y;
			break;

		case VerticalPosition::Bottom:
			posDraw.Y -= rectWH.Y;
			posDrawValue.Y -= rectWH.Y;
			break;

		default:
			break;
		}

		RectangleStruct rect = { posDraw.X, posDraw.Y, rectWH.X, rectWH.Y };
		DSurface::Composite->DrawRect(&rect, color2);
		int iPipWidth = 0;
		int iPipHeight = 0;
		int iPipTotal = this->HugeBar_Pips_Num;

		if (this->HugeBar_Pips_Offset.isset())
		{
			Point2D offset = this->HugeBar_Pips_Offset.Get();
			posDraw += offset;
			//center
			iPipWidth = (rectWH.X - offset.X * 2) / iPipTotal;
			iPipHeight = rectWH.Y - offset.Y * 2;
		}
		else
		{
			// default has 5 interval between border and pips at least
			const int iInterval = 5;
			iPipWidth = (rectWH.X - iInterval * 2) / iPipTotal;
			iPipHeight = rectWH.Y - iInterval * 2;
			posDraw.X += (rectWH.X - iPipTotal * iPipWidth) / 2;
			posDraw.Y += (rectWH.Y - iPipHeight) / 2;
		}

		if (iPipWidth <= 0 || iPipHeight <= 0)
			return;

		// Color1 75% Color2 25%, simulated healthbar
		int iPipColor1Width = std::max(static_cast<int>(iPipWidth * 0.75), std::min(3, iPipWidth));
		int iPipColor2Width = iPipWidth - iPipColor1Width;
		rect = { posDraw.X, posDraw.Y, iPipColor1Width , iPipHeight };

		for (int i = 0; i < iPipNumber; i++)
		{
			DSurface::Composite->FillRect(&rect, color1);
			rect.X += iPipColor1Width;
			rect.Width = iPipColor2Width;
			DSurface::Composite->FillRect(&rect, color2);
			rect.X += iPipColor2Width;
			rect.Width = iPipColor1Width;
		}
	}

	this->HugeBar_DrawValue(posDrawValue, iCurrent, iMax);
}

void HugeBar::HugeBar_DrawValue(Point2D& posDraw, int iCurrent, int iMax)
{
	RectangleStruct rBound = std::move(DSurface::Composite->GetRect());
	double ratio = static_cast<double>(iCurrent) / iMax;
	posDraw += this->Value_Offset;

	if (this->Value_Shape != nullptr)
	{
		SHPStruct* pShp = this->Value_Shape;
		ConvertClass* pPal = this->Value_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

		if (this->Anchor.Vertical == VerticalPosition::Bottom)
			posDraw.Y -= pShp->Height * (static_cast<int>(this->InfoType) + 1);
		else
			posDraw.Y += pShp->Height * static_cast<int>(this->InfoType);

		std::string text;

		if (this->Value_Percentage)
			text = GeneralUtils::IntToDigits(static_cast<int>(ratio * 100));
		else
			text = GeneralUtils::IntToDigits(iCurrent) + "/" + GeneralUtils::IntToDigits(iMax);

		int iNumBaseFrame = this->Value_Num_BaseFrame;
		int iSignBaseFrame = this->Value_Sign_BaseFrame;

		if (ratio <= RulesClass::Instance->ConditionYellow)
		{
			// number 0-9
			iNumBaseFrame += 10;
			// sign /%
			iSignBaseFrame += 2;
		}

		if (ratio <= RulesClass::Instance->ConditionRed)
		{
			iNumBaseFrame += 10;
			iSignBaseFrame += 2;
		}

		posDraw.X -= text.length() * this->Value_Shape_Spacing / 2;

		ShapeTextPrintData printData
		(
			pShp,
			pPal,
			iNumBaseFrame,
			iSignBaseFrame,
			Point2D({ this->Value_Shape_Spacing, 0 })
		);
		ShapeTextPrinter::PrintShape(text.c_str(), printData, posDraw, rBound, DSurface::Composite);
	}
	else
	{
		const int iTextHeight = 15;

		if (this->Anchor.Vertical == VerticalPosition::Bottom)
			posDraw.Y -= iTextHeight * (static_cast<int>(this->InfoType) + 1);
		else
			posDraw.Y += iTextHeight * static_cast<int>(this->InfoType);

		wchar_t text[0x16] = L"";

		if (this->Value_Percentage)
		{
			swprintf_s(text, L"%d", static_cast<int>(ratio * 100));
			wcscat_s(text, L"%%");
		}
		else
		{
			swprintf_s(text, L"%d/%d", iCurrent, iMax);
		}

		COLORREF color = Drawing::RGB_To_Int(this->Value_Text_Color.Get(ratio));
		DSurface::Composite->DrawTextA(text, &rBound, &posDraw, color, COLOR_BLACK, TextPrintType::Center);
	}
}

DEFINE_HOOK(0x4F4583, GScreenClass_Render_ProcessHugeBar, 0x6)
{
	HugeBar::ProcessHugeBar();

	return 0;
}
