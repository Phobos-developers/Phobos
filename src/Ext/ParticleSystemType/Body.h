#pragma once
#include <ParticleSystemTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ParticleSystemTypeExt
{
public:
	using base_type = ParticleSystemTypeClass;

	static constexpr DWORD Canary = 0xF9984EFE;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public ObjectTypeClassExtension
	{
	public:
		// typed owner accessor
		ParticleSystemTypeClass* OwnerObject() const
		{
			return static_cast<ParticleSystemTypeClass*>(this->GetAttachedObject());
		}

		Valueable<bool> AdjustTargetCoordsOnRotation;

		ExtData(ParticleSystemTypeClass* OwnerObject) : ObjectTypeClassExtension(OwnerObject)
			, AdjustTargetCoordsOnRotation { true }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ParticleSystemTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

// top-level name for the ParticleSystemTypeExt extension
using ParticleSystemTypeClassExtension = ParticleSystemTypeExt::ExtData;
