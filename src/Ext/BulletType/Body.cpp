#include "Body.h"

BulletTypeExt::ExtContainer BulletTypeExt::ExtMap;

double BulletTypeExt::GetAdjustedGravity(BulletTypeClass* pType)
{
	auto const pData = BulletTypeExt::ExtMap.Find(pType);
	auto const nGravity = pData->Gravity.Get(RulesClass::Instance->Gravity);
	return pType->Floater ? nGravity * 0.5 : nGravity;
}

BulletTypeClass* BulletTypeExt::GetDefaultBulletType()
{
	BulletTypeClass* pType = BulletTypeClass::Find(NONE_STR);

	if (pType)
		return pType;

	return GameCreate<BulletTypeClass>(NONE_STR);
}

// =============================
// load / save

void BulletTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->Armor.Read(exINI, pSection, "Armor");
	this->Interceptable.Read(exINI, pSection, "Interceptable");
	this->Interceptable_DeleteOnIntercept.Read(exINI, pSection, "Interceptable.DeleteOnIntercept");
	this->Interceptable_WeaponOverride.Read<true>(exINI, pSection, "Interceptable.WeaponOverride");
	this->Gravity.Read(exINI, pSection, "Gravity");

	PhobosTrajectoryType::CreateType(this->TrajectoryType, pINI, pSection, "Trajectory");
	this->Trajectory_Speed.Read(exINI, pSection, "Trajectory.Speed");

	this->Shrapnel_AffectsGround.Read(exINI, pSection, "Shrapnel.AffectsGround");
	this->Shrapnel_AffectsBuildings.Read(exINI, pSection, "Shrapnel.AffectsBuildings");
	this->Shrapnel_UseWeaponTargeting.Read(exINI, pSection, "Shrapnel.UseWeaponTargeting");
	this->ClusterScatter_Min.Read(exINI, pSection, "ClusterScatter.Min");
	this->ClusterScatter_Max.Read(exINI, pSection, "ClusterScatter.Max");
	this->SubjectToLand.Read(exINI, pSection, "SubjectToLand");
	this->SubjectToLand_Detonate.Read(exINI, pSection, "SubjectToLand.Detonate");
	this->SubjectToWater.Read(exINI, pSection, "SubjectToWater");
	this->SubjectToWater_Detonate.Read(exINI, pSection, "SubjectToWater.Detonate");
	this->AAOnly.Read(exINI, pSection, "AAOnly");
	this->Arcing_AllowElevationInaccuracy.Read(exINI, pSection, "Arcing.AllowElevationInaccuracy");
	this->ReturnWeapon.Read<true>(exINI, pSection, "ReturnWeapon");

	// Ares 0.7
	this->BallisticScatter_Min.Read(exINI, pSection, "BallisticScatter.Min");
	this->BallisticScatter_Max.Read(exINI, pSection, "BallisticScatter.Max");

	INI_EX exArtINI(CCINIClass::INI_Art);

	if (strlen(pThis->ImageFile))
		pSection = pThis->ImageFile;

	this->LaserTrail_Types.Read(exArtINI, pSection, "LaserTrail.Types");

	this->TrajectoryValidation();
}

void BulletTypeExt::ExtData::TrajectoryValidation() const
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	// Trajectory validation combined with other projectile behaviour.
	if (this->TrajectoryType)
	{
		if (pThis->Arcing)
		{
			Debug::Log("[Developer warning] [%s] has Trajectory set together with Arcing. Arcing has been set to false.\n", pSection);
			pThis->Arcing = false;
		}

		if (pThis->Inviso)
		{
			Debug::Log("[Developer warning] [%s] has Trajectory set together with Inviso. Inviso has been set to false.\n", pSection);
			pThis->Inviso = false;
		}

		if (pThis->ROT)
		{
			Debug::Log("[Developer warning] [%s] has Trajectory set together with ROT value other than 0. ROT has been set to 0.\n", pSection);
			pThis->ROT = 0;
		}

		if (pThis->Vertical)
		{
			Debug::Log("[Developer warning] [%s] has Trajectory set together with Vertical. Vertical has been set to false.\n", pSection);
			pThis->Vertical = false;
		}
	}
}

template <typename T>
void BulletTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Armor)
		.Process(this->Interceptable)
		.Process(this->Interceptable_DeleteOnIntercept)
		.Process(this->Interceptable_WeaponOverride)
		.Process(this->LaserTrail_Types)
		.Process(this->Gravity)
		.Process(this->Trajectory_Speed)
		.Process(this->Shrapnel_AffectsGround)
		.Process(this->Shrapnel_AffectsBuildings)
		.Process(this->Shrapnel_UseWeaponTargeting)
		.Process(this->ClusterScatter_Min)
		.Process(this->ClusterScatter_Max)
		.Process(this->BallisticScatter_Min)
		.Process(this->BallisticScatter_Max)
		.Process(this->SubjectToLand)
		.Process(this->SubjectToLand_Detonate)
		.Process(this->SubjectToWater)
		.Process(this->SubjectToWater_Detonate)
		.Process(this->AAOnly)
		.Process(this->Arcing_AllowElevationInaccuracy)
		.Process(this->ReturnWeapon)
		;

	this->TrajectoryType = PhobosTrajectoryType::ProcessFromStream(Stm, this->TrajectoryType);
}

void BulletTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BulletTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BulletTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BulletTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}


// =============================
// container

BulletTypeExt::ExtContainer::ExtContainer() : Container("BulletTypeClass") { }

BulletTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x46BDD9, BulletTypeClass_CTOR, 0x5)
{
	GET(BulletTypeClass*, pItem, EAX);

	BulletTypeExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x46C8B6, BulletTypeClass_SDDTOR, 0x6)
{
	GET(BulletTypeClass*, pItem, ESI);

	if (auto pType = BulletTypeExt::ExtMap.Find(pItem)->TrajectoryType)
		DLLDelete(pType);

	BulletTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46C730, BulletTypeClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46C6A0, BulletTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x46C722, BulletTypeClass_Load_Suffix, 0x4)
{
	BulletTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46C74A, BulletTypeClass_Save_Suffix, 0x3)
{
	BulletTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x46C429, BulletTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x46C41C, BulletTypeClass_LoadFromINI, 0xA)
{
	GET(BulletTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x90);

	BulletTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
