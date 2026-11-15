#pragma once

#include <SidebarClass.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SidebarExt
{
public:
	using base_type = SidebarClass;

	static constexpr DWORD Canary = 0x51DEBA12;

	class ExtData final : public Extension<SidebarClass>
	{
	public:

		ExtData(SidebarClass* OwnerObject) : Extension<SidebarClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static SHPStruct* TabProducingProgress[4];

	static void Allocate(SidebarClass* pThis);
	static void Remove(SidebarClass* pThis);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(&SidebarClass::Instance);
	}

	static bool __stdcall AresTabCameo_RemoveCameo(BuildType* pItem);
};
