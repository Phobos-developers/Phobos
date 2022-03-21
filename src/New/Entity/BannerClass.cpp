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
	int x = (int)((double)this->Position.X / 100.f) * DSurface::Composite->Width;
	int y = (int)((double)this->Position.Y / 100.f) * DSurface::Composite->Height;
	switch (this->Type->Type)
	{
	case BannerType::PCX:
		BSurface* pcx;
		pcx = PCX::Instance->GetSurface(this->Type->Banner_PCX.Get().data());
		if (pcx)
		{
			RectangleStruct bounds = { x, y, pcx->Width, pcx->Height };
			PCX::Instance->BlitToSurface(&bounds, DSurface::Composite, pcx);
		}
		break;
	case BannerType::CSF:
		ColorStruct clr;
		RectangleStruct vRect;
		vRect = { 0, 0, 0, 0 };
		DSurface::Composite->GetRect(&vRect);
		Point2D vPos = Point2D{ x, y };
		const wchar_t* a;
		a = this->Type->Banner_CSF.Get().Text;

		DSurface::Composite->DrawText(a, &vRect, &vPos, Drawing::RGB2DWORD(clr), 0,
			TextPrintType::UseGradPal | TextPrintType::Center | TextPrintType::Metal12);
		break;
	default:
		break;
	}

}
