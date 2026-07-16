#pragma once
#include <TeamTypeClass.h>

#include <Ext/AbstractType/Body.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class TeamTypeExt final : public AbstractTypeExt
{
public:
	using base_type = TeamTypeClass;
	using ExtData = TeamTypeExt;

	static constexpr DWORD Canary = 0xABCDEF01;

public:
	// typed owner accessor
	TeamTypeClass* OwnerObject() const
	{
		return static_cast<TeamTypeClass*>(this->GetAttachedObject());
	}

	TeamTypeExt(TeamTypeClass* OwnerObject) : AbstractTypeExt(OwnerObject)
		, SetRecruitableOnLiberate { }
	{ }

	virtual ~TeamTypeExt() = default;

	virtual void LoadFromINIFile(CCINIClass* pINI) override;
	// virtual void Initialize() override;

	Nullable<int> SetRecruitableOnLiberate;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<TeamTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static TeamTypeExt* Fetch(const TeamTypeClass* pThis)
	{
		return ExtMap.Find(pThis);
	}

	static TeamTypeExt* TryFetch(const TeamTypeClass* pThis)
	{
		return ExtMap.TryFind(pThis);
	}
};

