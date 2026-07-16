#pragma once

#include <ParticleTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ParticleTypeExt
{
public:
	using base_type = ParticleTypeClass;

	static constexpr DWORD Canary = 0xEAFEEAFE;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public ObjectTypeClassExtension
	{
	public:
		// typed owner accessor
		ParticleTypeClass* OwnerObject() const
		{
			return static_cast<ParticleTypeClass*>(this->GetAttachedObject());
		}

		Valueable<int> Gas_MaxDriftSpeed;

		ExtData(ParticleTypeClass* OwnerObject) : ObjectTypeClassExtension(OwnerObject)
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

// top-level name for the ParticleTypeExt extension
using ParticleTypeClassExtension = ParticleTypeExt::ExtData;
