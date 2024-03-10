#pragma once

#include <Utilities/Template.h>

class DroppodTypeClass
{
public:
	Nullable<int> Speed {};
	Nullable<double> Angle {};
	Nullable<int> Height {};
	Nullable<WeaponTypeClass*> Weapon {};
	Nullable<AnimTypeClass*> GroundAnim[2] { {},{} };
	Nullable<AnimTypeClass*> Puff {};
	Nullable<AnimTypeClass*> Trailer {};
	Valueable<int> Trailer_SpawnDelay { 6 };
	Nullable<AnimTypeClass*> AtmosphereEntry {};
	Nullable<SHPStruct*> AirImage { };
	Valueable<bool> Trailer_Attached { false };
	Valueable<bool> Weapon_HitLandOnly { false };

	DroppodTypeClass() = default;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};
