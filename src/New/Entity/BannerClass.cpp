#include "BannerClass.h"

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

void BannerClass::Render()
{
	int x = (int)((double)this->Position.X / 100.f * (double)DSurface::Composite->Width);
	int y = (int)((double)this->Position.Y / 100.f * (double)DSurface::Composite->Height);
	int xsize = 0;
	int ysize = 0;
	switch (this->Type->Type)
	{
	case BannerType::PCX:
		BSurface* pcx;
		char filename[0x20];
		strcpy(filename, this->Type->Banner_PCX.data());
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
		break;
	case BannerType::CSF:
		ColorStruct clr;
		RectangleStruct vRect;
		vRect = { 0, 0, 0, 0 };
		DSurface::Composite->GetRect(&vRect);
		Point2D vPos = Point2D{ x, y };
		const wchar_t* text;
		text = this->Type->Banner_CSF.Get().Text;

		DSurface::Composite->DrawText(text, &vRect, &vPos, Drawing::RGB2DWORD(clr), 0,
			TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12);
		break;
	default:
		break;
	}

}
