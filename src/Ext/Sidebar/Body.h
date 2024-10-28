#pragma once

#include <SidebarClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <map>

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

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

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
		Allocate(SidebarClass::Instance);
	}

	static void PointerGotInvalid(void* ptr, bool removed)
	{
		Global()->InvalidatePointer(ptr, removed);
	}
};
