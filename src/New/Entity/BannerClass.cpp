#include "BannerClass.h"

#include <Ext/Scenario/Body.h>

DynamicVectorClass<BannerClass*> BannerClass::Array;

template <typename T>
bool BannerClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Id)
		.Process(this->Type)
		.Process(this->Position)
		.Process(this->Variable)
		.Process(this->IsGlobalVariable)
		.Success();
}

bool BannerClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool BannerClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<BannerClass*>(this)->Serialize(Stm);
}

void BannerClass::RenderPCX(int x, int y)
{
	int xsize = 0;
	int ysize = 0;
	BSurface* pcx;
	char filename[0x20];
	strcpy(filename, this->Type->Content_PCX.data());
	_strlwr_s(filename);
	pcx = PCX::Instance->GetSurface(filename);
	if (pcx)
	{
		x = x - pcx->Width / 2;
		y = y - pcx->Height / 2;
		xsize = pcx->Width;
		ysize = pcx->Height;
		RectangleStruct bounds = { x, y, xsize, ysize };

		PCX::Instance->BlitToSurface(&bounds, DSurface::Composite, pcx);
	}
}

void BannerClass::RenderSHP(int x, int y)
{
	if (this->Type->ImageSHP && this->Type->Palette)
	{
		x = x - this->Type->ImageSHP->Width / 2;
		y = y - this->Type->ImageSHP->Height / 2;
		Point2D vPos = { x, y };

		DSurface::Composite->DrawSHP(this->Type->Palette, this->Type->ImageSHP, 0, &vPos, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}
}

void BannerClass::RenderCSF(int x, int y)
{
	RectangleStruct vRect = { 0, 0, 0, 0 };
	DSurface::Composite->GetRect(&vRect);
	Point2D vPos = Point2D{ x, y };
	wchar_t text[10];
	wchar_t text2[266];

	auto& variables = ScenarioExt::Global()->Variables[this->IsGlobalVariable != 0];
	auto itr = variables.find(this->Variable);
	if (itr != variables.end())
	{
		swprintf_s(text, L" %d", itr->second.Value);
	}
	wcscpy(text2, this->Type->Text);
	wcscat(text2, text);

	TextPrintType textFlags = TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12 |
		(this->Type->Content_CSF_DrawBackground ? TextPrintType::Background : (TextPrintType)0);

	DSurface::Composite->DrawText(text2, &vRect, &vPos,
		Drawing::RGB2DWORD(this->Type->Content_CSF_Color.Get(Drawing::TooltipColor())), 0, textFlags);
}

void BannerClass::Render()
{
	int x = (int)((double)this->Position.X / 100.f * (double)DSurface::Composite->Width);
	int y = (int)((double)this->Position.Y / 100.f * (double)DSurface::Composite->Height);
	switch (this->Type->Type)
	{
	case BannerType::PCX:
		this->RenderPCX(x, y);
		break;
	case BannerType::SHP:
		this->RenderSHP(x, y);
		break;
	case BannerType::CSF:
		this->RenderCSF(x, y);
		break;
	default:
		break;
	}

}
