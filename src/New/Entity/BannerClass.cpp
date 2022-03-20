#include "BannerClass.h"

DynamicVectorClass<BannerClass*> BannerClass::Instances;

BannerClass::BannerClass() : Id { 0 }
, Type { BannerType::PCX }
, Position { CoordStruct() }
, Source { 0 }
{ }

BannerClass::BannerClass(int id, CoordStruct position, BannerType type, char source[32]) : Id { id }
, Type { BannerType::PCX }
, Position { position }
{
	strcpy(this->Source, source);
	this->LoadContent();
	BannerClass::Instances.AddItem(this);
}

template <typename T>
bool BannerClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Id)
		.Process(this->Type)
		.Process(this->Position)
		.Process(this->Source)
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

void BannerClass::LoadContent()
{
	switch (this->Type)
	{
	case BannerType::PCX:
		// load pcx
		PCX::Instance->LoadFile(this->Source);
		break;
	case BannerType::CSF:
		// TODO load csf ???
		break;
	default:
		break;
	}
}

void BannerClass::Render()
{
	switch (this->Type)
	{
	case BannerType::PCX:
		if (auto pcx = PCX::Instance->GetSurface(this->Source))
		{
			RectangleStruct bounds = { this->Position.X, this->Position.X, pcx->Width, pcx->Height };
			PCX::Instance->BlitToSurface(&bounds, DSurface::Composite, pcx);
		}
		break;
	case BannerType::CSF:
		// TODO print csf
		break;
	default:
		break;
	}

}
