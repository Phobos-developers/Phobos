#pragma once

#include <ParticleTypeClass.h>

#include <Ext/ObjectType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class ParticleTypeExt final : public ObjectTypeExt
{
public:
	using base_type = ParticleTypeClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = ParticleTypeExt;

	static constexpr DWORD Canary = 0xEAFEEAFE;

public:
	// typed owner accessor
	ParticleTypeClass* OwnerObject() const
	{
		return static_cast<ParticleTypeClass*>(this->GetAttachedObject());
	}

	Valueable<int> Gas_MaxDriftSpeed;

	ParticleTypeExt(ParticleTypeClass* OwnerObject) : ObjectTypeExt(OwnerObject)
		, Gas_MaxDriftSpeed { 2 }
	{ }

	virtual ~ParticleTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<ParticleTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static ParticleTypeExt* Fetch(const ParticleTypeClass* pThis)
	{
		return AbstractExt::Fetch<ParticleTypeExt>(pThis);
	}

	static ParticleTypeExt* TryFetch(const ParticleTypeClass* pThis)
	{
		return AbstractExt::TryFetch<ParticleTypeExt>(pThis);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};

