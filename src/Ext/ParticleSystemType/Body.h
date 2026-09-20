#pragma once
#include <ParticleSystemTypeClass.h>

#include <Ext/ParticleType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ParticleSystemTypeExt final : public ObjectTypeExt
{
public:
	using base_type = ParticleSystemTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = ParticleSystemTypeExt;

	static constexpr DWORD Canary = 0xF9984EFE;

public:
	// typed owner accessor
	ParticleSystemTypeClass* OwnerObject() const
	{
		return static_cast<ParticleSystemTypeClass*>(this->GetAttachedObject());
	}

	Valueable<bool> AdjustTargetCoordsOnRotation;

	ParticleSystemTypeExt(ParticleSystemTypeClass* OwnerObject) : ObjectTypeExt(OwnerObject)
		, AdjustTargetCoordsOnRotation { true }
	{ }

	virtual ~ParticleSystemTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<ParticleSystemTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static ParticleSystemTypeExt* Fetch(const ParticleSystemTypeClass* pThis)
	{
		return AbstractExt::Fetch<ParticleSystemTypeExt>(pThis);
	}

	static ParticleSystemTypeExt* TryFetch(const ParticleSystemTypeClass* pThis)
	{
		return AbstractExt::TryFetch<ParticleSystemTypeExt>(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

