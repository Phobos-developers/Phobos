#pragma once
#include <AnimClass.h>
#include <ParticleSystemClass.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Object/Body.h>

class AnimExt final : public ObjectExt
{
public:
	using base_type = AnimClass;

	// deprecated: the pre-rework nested data class is now the extension class itself
	using ExtData [[deprecated("use the extension class itself instead")]] = AnimExt;

	static constexpr DWORD Canary = 0xAAAAAAAA;

public:
	// typed owner accessor
	AnimClass* OwnerObject() const
	{
		return static_cast<AnimClass*>(this->GetAttachedObject());
	}

	DirType DeathUnitFacing;
	DirStruct DeathUnitTurretFacing;
	bool FromDeathUnit;
	bool DeathUnitHasTurret;
	TechnoClass* Invoker;
	HouseClass* InvokerHouse;
	ParticleSystemClass* AttachedSystem;
	BuildingClass* ParentBuilding; // Only set on building anims, used for tinting the anims etc. especially when not on same cell as building
	bool IsTechnoTrailerAnim;
	bool DelayedFireRemoveOnNoDelay;
	bool IsAttachedEffectAnim;
	bool IsShieldIdleAnim;
	WeaponTypeClass* FiringAnim_Weapon;
	int FiringAnim_WeaponIndex;
	int FiringAnim_BurstIndex;
	DirStruct FiringAnim_LastFacing;
	CoordStruct FiringAnim_LastCoords;
	double FirepowerMult;
		Point2D AEDrawOffset;

	AnimExt(AnimClass* OwnerObject) : ObjectExt(OwnerObject)
		, DeathUnitFacing { 0 }
		, DeathUnitTurretFacing {}
		, FromDeathUnit { false }
		, DeathUnitHasTurret { false }
		, Invoker {}
		, InvokerHouse {}
		, AttachedSystem {}
		, ParentBuilding {}
		, IsTechnoTrailerAnim { false }
		, DelayedFireRemoveOnNoDelay { false }
		, IsAttachedEffectAnim { false }
		, IsShieldIdleAnim { false }
		, FiringAnim_Weapon {}
		, FiringAnim_WeaponIndex {}
		, FiringAnim_BurstIndex {}
		, FiringAnim_LastFacing {}
		, FiringAnim_LastCoords {}
		, FirepowerMult { 1.0 }
			, AEDrawOffset { Point2D::Empty }
	{ }

	void SetInvoker(TechnoClass* pInvoker);
	void SetInvoker(TechnoClass* pInvoker, HouseClass* pInvokerHouse);
	void CreateAttachedSystem();
	void DeleteAttachedSystem();

	void UpdateAsFiringAnim();

	virtual ~AnimExt() override;

	virtual void InitializeConstants() override;

	virtual void LoadFromStream(PhobosStreamReader& Stm) override;
	virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	virtual void PostLoad() override;

private:
	template <typename T>
	void Serialize(T& Stm);

public:
	class ExtContainer final : public Container<AnimExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void Clear()
	{
		AnimExt::AnimsWithAttachedParticles.clear();
	}

	static std::vector<AnimClass*> AnimsWithAttachedParticles;
	static ExtContainer ExtMap;

	static AnimExt* Fetch(const AnimClass* pThis)
	{
		return AbstractExt::Fetch<AnimExt>(pThis);
	}

	static AnimExt* TryFetch(const AnimClass* pThis)
	{
		return AbstractExt::TryFetch<AnimExt>(pThis);
	}

	static bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = false, bool defaultToInvokerOwner = false);
	static HouseClass* GetOwnerHouse(AnimClass* pAnim, HouseClass* pDefaultOwner = nullptr);
	static void VeinAttackAI(AnimClass* pAnim);
	static void ChangeAnimType(AnimClass* pAnim, AnimTypeClass* pNewType, bool resetLoops, bool restart);
	static void HandleDebrisImpact(AnimTypeClass* pExpireAnim, const std::vector<AnimTypeClass*>& pWakeAnim, Iterator<AnimTypeClass*> splashAnims, HouseClass* pOwner, WarheadTypeClass* pWarhead, int nDamage,
	CellClass* pCell, CoordStruct nLocation, bool heightFlag, bool isMeteor, bool warheadDetonate, bool explodeOnWater, bool splashAnimsPickRandom);

	static void SpawnFireAnims(AnimClass* pThis);

	static void InvalidateTechnoPointers(TechnoClass* pTechno);
	static void InvalidateParticleSystemPointers(ParticleSystemClass* pParticleSystem);
	static void CreateRandomAnim(const std::vector<AnimTypeClass*>& AnimList, CoordStruct coords, TechnoClass* pTechno = nullptr, HouseClass* pHouse = nullptr, bool invoker = false, bool ownedObject = false);
};

