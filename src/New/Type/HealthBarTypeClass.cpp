#include "HealthBarTypeClass.h"

template<>
const char* Enumerable<HealthBarTypeClass>::GetMainSection()
{
	return "HealthBarTypes";
}

void HealthBarTypeClass::DrawBuildingBar(Point2D* pLocation, RectangleStruct* pBounds, const int pipsTotal, const int pipsLength, const int frame, const int emptyFrame) const
{
	const auto pPipsPalette = this->PipsPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	const auto pPipsShape = this->PipsShape.Get();
	const auto interval = this->PipsInterval_Building.Get();
	HealthBarTypeClass::DrawBuildingBar(pPipsPalette, pPipsShape, pLocation, pBounds, interval, pipsTotal, pipsLength, frame, emptyFrame);
}

void HealthBarTypeClass::DrawOtherBar(SHPStruct* pBrdShape, Point2D* pLocation, RectangleStruct* pBounds, const int brdXOffset, const int pipsTotal, const int frame, const int brdFrame) const
{
	const auto pBrdPalette = this->PipBrdPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	const auto pPipsPalette = this->PipsPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	const auto pPipsShape = this->PipsShape.Get();
	const auto pipsInterval = this->PipsInterval.Get();
	HealthBarTypeClass::DrawOtherBar(pBrdPalette, pBrdShape, pPipsPalette, pPipsShape, pLocation, pBounds, brdXOffset, pipsInterval, pipsTotal, frame, brdFrame);
}

void HealthBarTypeClass::DrawAnimatedBar(SHPStruct* pBrdShape, Point2D* pLocation, RectangleStruct* pBounds, const int brdXOffset, const double ratio, const int brdFrame) const
{
	const auto pBrdPalette = this->PipBrdPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	const auto pPipsPalette = this->PipsPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	const auto pPipsShape = this->PipsShape.Get();
	HealthBarTypeClass::DrawAnimatedBar(pBrdPalette, pBrdShape, pPipsPalette, pPipsShape, pLocation, pBounds, brdXOffset, ratio, brdFrame);
}

void HealthBarTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;

	if (!_stricmp(pSection, NONE_STR))
		return;

	INI_EX exINI(pINI);

	this->Pips.Read(exINI, pSection, "Pips");
	this->Pips_Building.Read(exINI, pSection, "Pips.Building");
	this->PipsEmpty.Read(exINI, pSection, "PipsEmpty");
	this->PipsInterval.Read(exINI, pSection, "PipsInterval");
	this->PipsInterval_Building.Read(exINI, pSection, "PipsInterval.Building");
	this->PipsLength.Read(exINI, pSection, "PipsLength");
	this->PipsShape.Read(exINI, pSection, "PipsShape");
	this->PipsPalette.LoadFromINI(pINI, pSection, "PipsPalette");

	this->PipBrd.Read(exINI, pSection, "PipBrd");
	this->PipBrdShape.Read(exINI, pSection, "PipBrdShape");
	this->PipBrdPalette.LoadFromINI(pINI, pSection, "PipBrdPalette");
	this->PipBrdXOffset.Read(exINI, pSection, "PipBrdXOffset");

	this->IsAnimated.Read(exINI, pSection, "IsAnimated");
	this->IsAnimated_Reverse.Read(exINI, pSection, "IsAnimated.Reverse");
	this->XOffset.Read(exINI, pSection, "XOffset");

	this->InfoType.Read(exINI, pSection, "InfoType");
	this->InfoIndex.Read(exINI, pSection, "InfoIndex");
}

template <typename T>
void HealthBarTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Pips)
		.Process(this->Pips_Building)
		.Process(this->PipsEmpty)
		.Process(this->PipsInterval)
		.Process(this->PipsInterval_Building)
		.Process(this->PipsLength)
		.Process(this->PipsShape)
		.Process(this->PipsPalette)
		.Process(this->PipBrd)
		.Process(this->PipBrdShape)
		.Process(this->PipBrdPalette)
		.Process(this->PipBrdXOffset)
		.Process(this->IsAnimated)
		.Process(this->IsAnimated_Reverse)
		.Process(this->XOffset)
		.Process(this->InfoType)
		.Process(this->InfoIndex)
		;
}

void HealthBarTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void HealthBarTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

void HealthBarTypeClass::DrawBuildingBar(ConvertClass* pPalette, SHPStruct* pShape, Point2D* pLocation, RectangleStruct* pBounds, Point2D interval, const int pipsTotal, const int pipsLength, const int frame, const int emptyFrame)
{
	if (pipsTotal > 0)
	{
		Point2D drawPoint = *pLocation + Point2D { 3 + 4 * pipsLength, 4 - 2 * pipsLength };
		BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400;

		for (int idx = pipsTotal; idx; --idx, drawPoint += interval)
			DSurface::Composite->DrawSHP(pPalette, pShape, frame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	if (pipsTotal < pipsLength)
	{
		int idx = pipsLength - pipsTotal, deltaX = 4 * pipsTotal, deltaY = -2 * pipsTotal;
		Point2D drawPoint = *pLocation + Point2D { 3 + 4 * pipsLength - deltaX, 4 - 2 * pipsLength - deltaY };
		BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400;

		for (; idx; --idx, drawPoint += interval)
			DSurface::Composite->DrawSHP(pPalette, pShape, emptyFrame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void HealthBarTypeClass::DrawOtherBar(ConvertClass* pBrdPalette, SHPStruct* pBrdShape, ConvertClass* pPipsPalette, SHPStruct* pPipsShape, Point2D* pLocation, RectangleStruct* pBounds, const int brdXOffset, Point2D interval, const int pipsTotal, const int brdFrame, const int frame)
{
	if (brdFrame >= 0)
	{
		Point2D drawPoint = *pLocation + Point2D { brdXOffset + 16, -1 };
		BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400 | BlitterFlags::Alpha;

		DSurface::Composite->DrawSHP(pBrdPalette, pBrdShape, brdFrame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	Point2D drawPoint = *pLocation;
	BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400;

	for (int idx = 0; idx < pipsTotal; ++idx, drawPoint += interval)
		DSurface::Composite->DrawSHP(pPipsPalette, pPipsShape, frame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
}

void HealthBarTypeClass::DrawAnimatedBar(ConvertClass* pBrdPalette, SHPStruct* pBrdShape, ConvertClass* pPipsPalette, SHPStruct* pPipsShape, Point2D* pLocation, RectangleStruct* pBounds, const int brdXOffset, const double ratio, const int brdFrame)
{
	if (pBrdShape && brdFrame != -1)
	{
		Point2D drawPoint = *pLocation + Point2D { brdXOffset + 16, -1 };
		BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400 | BlitterFlags::Alpha;

		DSurface::Temp->DrawSHP(pBrdPalette, pBrdShape, brdFrame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	const int frame = static_cast<int>((pPipsShape->Frames - 1) * (1.0 - ratio));

	Point2D drawPoint = *pLocation;
	BlitterFlags blitterFlags = BlitterFlags::Centered | BlitterFlags::bf_400;

	DSurface::Temp->DrawSHP(pPipsPalette, pPipsShape, frame, &drawPoint, pBounds, blitterFlags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
}
