#pragma once

#include <ParticleTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Macro.h>

class ParticleTypeExt
{
public:
	using base_type = ParticleTypeClass;

	static constexpr DWORD Canary = 0xEAFEEAFE;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<ParticleTypeClass>
	{
	public:
		Valueable<int> Gas_MaxDriftSpeed;

		ExtData(ParticleTypeClass* OwnerObject) : Extension<ParticleTypeClass>(OwnerObject)
			, Gas_MaxDriftSpeed { 2 }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
