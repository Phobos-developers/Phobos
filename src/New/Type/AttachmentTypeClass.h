#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

#include <TechnoTypeClass.h>

class AttachmentTypeClass final : public Enumerable<AttachmentTypeClass>
{
public:
	Valueable<bool> RestoreAtCreation; // whether to spawn the attachment initially
	Valueable<bool> InheritCommands;
	Valueable<bool> InheritOwner; // aka mind control inheritance
	Valueable<bool> InheritStateEffects; // phasing out, stealth etc.
	Valueable<bool> InheritDestruction;
	Valueable<bool> LowSelectionPriority;
	Valueable<AttachmentYSortPosition> YSortPosition;
	// Valueable<bool> CanBeForceDetached;
	// Nullable<WeaponTypeClass*> ForceDetachWeapon_Child;
	// Nullable<WeaponTypeClass*> ForceDetachWeapon_Parent;
	Nullable<WeaponTypeClass*> DestructionWeapon_Child;
	Nullable<WeaponTypeClass*> DestructionWeapon_Parent;
	Nullable<Mission> ParentDestructionMission;
	Nullable<Mission> ParentDetachmentMission;

	AttachmentTypeClass(const char* pTitle = NONE_STR) : Enumerable<AttachmentTypeClass>(pTitle)
		, RestoreAtCreation { true }
		, InheritCommands { true }
		, InheritOwner { true }
		, InheritStateEffects { true }
		, InheritDestruction { true }
		, LowSelectionPriority { true }
		, YSortPosition { AttachmentYSortPosition::Default }
		// , CanBeForceDetached { false }
		// , ForceDetachWeapon_Child { }
		// , ForceDetachWeapon_Parent { }
		, DestructionWeapon_Child { }
		, DestructionWeapon_Parent { }
		, ParentDestructionMission { }
		, ParentDetachmentMission { }
	{ }

	virtual ~AttachmentTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
