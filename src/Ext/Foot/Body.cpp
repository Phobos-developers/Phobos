#include "Body.h"

// =============================
// load / save

template <typename T>
void FootExt::Serialize(T& Stm)
{
}

void FootExt::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoExt::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void FootExt::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoExt::SaveToStream(Stm);
	this->Serialize(Stm);
}
