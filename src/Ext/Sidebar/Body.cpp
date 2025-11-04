#include "Body.h"

std::unique_ptr<SidebarExt::ExtData> SidebarExt::Data = nullptr;

SHPStruct* SidebarExt::TabProducingProgress[4];

void SidebarExt::Allocate(SidebarClass* pThis)
{
	Data = std::make_unique<SidebarExt::ExtData>(pThis);
}

void SidebarExt::Remove(SidebarClass* pThis)
{
	Data = nullptr;
}

// =============================
// load / save

template <typename T>
void SidebarExt::ExtData::Serialize(T& Stm)
{
	Stm
		;
}

void SidebarExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<SidebarClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void SidebarExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<SidebarClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}


// =============================
// container hooks

DEFINE_HOOK(0x6A4F0B, SidebarClass_CTOR, 0x5)
{
	GET(SidebarClass*, pItem, EAX);

	SidebarExt::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6AC82F, SidebarClass_DTOR, 0x5)
{
	GET(SidebarClass*, pItem, EBX);

	SidebarExt::Remove(pItem);
	return 0;
}

IStream* SidebarExt::g_pStm = nullptr;

DEFINE_HOOK_AGAIN(0x6AC5D0, SidebarClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x6AC5E0, SidebarClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(IStream*, pStm, 0x4);

	SidebarExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x6AC5DA, SidebarClass_Load_Suffix, 0x6)
{
	auto buffer = SidebarExt::Global();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(SidebarExt::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(SidebarExt::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x6AC5EA, SidebarClass_Save_Suffix, 0x6)
{
	auto buffer = SidebarExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(SidebarExt::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(SidebarExt::g_pStm);

	return 0;
}
