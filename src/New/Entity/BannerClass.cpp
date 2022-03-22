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

void RenderPCX(BannerTypeClass* pType, int x, int y)
{
	int xsize = 0;
	int ysize = 0;
	BSurface* pcx;
	char filename[0x20];
	strcpy(filename, pType->Banner_PCX.data());
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

void RenderCSF(BannerTypeClass* pType, int x, int y)
{
	ColorStruct clr = { 255, 0, 0 };
	RectangleStruct vRect = { 0, 0, 0, 0 };
	DSurface::Composite->GetRect(&vRect);
	Point2D vPos = Point2D{ x, y };
	wchar_t text[10];
	wchar_t text2[266];

	auto& variables = ScenarioExt::Global()->Variables[1];
	auto itr = variables.find(1);
	if (itr != variables.end())
	{
		swprintf_s(text, L" %d", itr->second.Value);
	}
	wcscpy(text2, pType->Text);
	wcscat(text2, text);

	DSurface::Composite->DrawText(text2, &vRect, &vPos, Drawing::RGB2DWORD(clr), 0,
		TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12 | TextPrintType::Background);
}


void BannerClass::Render()
{
	int x = (int)((double)this->Position.X / 100.f * (double)DSurface::Composite->Width);
	int y = (int)((double)this->Position.Y / 100.f * (double)DSurface::Composite->Height);
	switch (this->Type->Type)
	{
	case BannerType::PCX:
		RenderPCX(this->Type, x, y);
		break;
	case BannerType::CSF:
		RenderCSF(this->Type, x, y);
		break;
	default:
		break;
	}

}
