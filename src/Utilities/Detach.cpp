#include "Detach.h"

#include <type_traits>

#include <AbstractClass.h>
#include <ObjectClass.h>
#include <MissionClass.h>
#include <RadioClass.h>
#include <AircraftClass.h>
#include <AircraftTypeClass.h>
#include <AirstrikeClass.h>
#include <AnimClass.h>
#include <AnimTypeClass.h>
#include <BuildingClass.h>
#include <BuildingTypeClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <CellClass.h>
#include <FactoryClass.h>
#include <FootClass.h>
#include <HouseClass.h>
#include <HouseTypeClass.h>
#include <InfantryClass.h>
#include <InfantryTypeClass.h>
#include <OverlayClass.h>
#include <OverlayTypeClass.h>
#include <ParticleClass.h>
#include <ParticleSystemClass.h>
#include <ParticleSystemTypeClass.h>
#include <ParticleTypeClass.h>
#include <RadSiteClass.h>
#include <ScriptClass.h>
#include <ScriptTypeClass.h>
#include <SideClass.h>
#include <SmudgeClass.h>
#include <SmudgeTypeClass.h>
#include <SuperClass.h>
#include <SuperWeaponTypeClass.h>
#include <TActionClass.h>
#include <TEventClass.h>
#include <TagClass.h>
#include <TagTypeClass.h>
#include <TaskForceClass.h>
#include <TeamClass.h>
#include <TeamTypeClass.h>
#include <TechnoClass.h>
#include <TechnoTypeClass.h>
#include <TerrainClass.h>
#include <TerrainTypeClass.h>
#include <TiberiumClass.h>
#include <TriggerClass.h>
#include <TriggerTypeClass.h>
#include <UnitClass.h>
#include <UnitTypeClass.h>
#include <VoxelAnimClass.h>
#include <VoxelAnimTypeClass.h>
#include <WarheadTypeClass.h>
#include <WaveClass.h>
#include <WeaponTypeClass.h>

namespace Detach
{
	template <typename... Ts>
	struct TypeList { };

	// Every type a Listener<T> may watch. A listener is notified when an object of T
	// or of any class derived from T is detached.
	using NotifiableTypes = TypeList<
		AbstractClass,
		ObjectClass, MissionClass, RadioClass, TechnoClass, FootClass,
		UnitClass, AircraftClass, InfantryClass, BuildingClass,
		AnimClass, BulletClass, OverlayClass, ParticleClass, ParticleSystemClass,
		SmudgeClass, TerrainClass, VoxelAnimClass, WaveClass,
		CellClass, FactoryClass, HouseClass, SuperClass, TeamClass,
		TriggerClass, TagClass, ScriptClass, TActionClass, TEventClass,
		AirstrikeClass, RadSiteClass,
		AbstractTypeClass, ObjectTypeClass, TechnoTypeClass,
		UnitTypeClass, AircraftTypeClass, InfantryTypeClass, BuildingTypeClass,
		AnimTypeClass, BulletTypeClass, OverlayTypeClass, ParticleTypeClass,
		ParticleSystemTypeClass, SmudgeTypeClass, TerrainTypeClass, VoxelAnimTypeClass,
		HouseTypeClass, SideClass, TeamTypeClass, TaskForceClass, ScriptTypeClass,
		TriggerTypeClass, TagTypeClass, TiberiumClass, SuperWeaponTypeClass,
		WeaponTypeClass, WarheadTypeClass
	>;

	template <typename TConcrete, typename... Ts>
	static void NotifyChain(TypeList<Ts...>, AbstractClass* const pTarget, const bool removed)
	{
		const auto visit = [&]<typename T>()
		{
			if constexpr (std::is_base_of_v<T, TConcrete>)
				Registry<T>::Notify(static_cast<T*>(pTarget), removed);
		};

		(visit.template operator()<Ts>(), ...);
	}

	template <typename TConcrete>
	static void NotifyChain(AbstractClass* const pTarget, const bool removed)
	{
		NotifyChain<TConcrete>(NotifiableTypes {}, pTarget, removed);
	}

	void NotifyAbstract(AbstractClass* const pTarget, const bool removed)
	{
		switch (pTarget->WhatAmI())
		{
		case AbstractType::Unit: NotifyChain<UnitClass>(pTarget, removed); break;
		case AbstractType::Aircraft: NotifyChain<AircraftClass>(pTarget, removed); break;
		case AbstractType::Infantry: NotifyChain<InfantryClass>(pTarget, removed); break;
		case AbstractType::Building: NotifyChain<BuildingClass>(pTarget, removed); break;
		case AbstractType::Anim: NotifyChain<AnimClass>(pTarget, removed); break;
		case AbstractType::Bullet: NotifyChain<BulletClass>(pTarget, removed); break;
		case AbstractType::Overlay: NotifyChain<OverlayClass>(pTarget, removed); break;
		case AbstractType::Particle: NotifyChain<ParticleClass>(pTarget, removed); break;
		case AbstractType::ParticleSystem: NotifyChain<ParticleSystemClass>(pTarget, removed); break;
		case AbstractType::Smudge: NotifyChain<SmudgeClass>(pTarget, removed); break;
		case AbstractType::Terrain: NotifyChain<TerrainClass>(pTarget, removed); break;
		case AbstractType::VoxelAnim: NotifyChain<VoxelAnimClass>(pTarget, removed); break;
		case AbstractType::Wave: NotifyChain<WaveClass>(pTarget, removed); break;
		case AbstractType::Cell: NotifyChain<CellClass>(pTarget, removed); break;
		case AbstractType::Factory: NotifyChain<FactoryClass>(pTarget, removed); break;
		case AbstractType::House: NotifyChain<HouseClass>(pTarget, removed); break;
		case AbstractType::Super: NotifyChain<SuperClass>(pTarget, removed); break;
		case AbstractType::Team: NotifyChain<TeamClass>(pTarget, removed); break;
		case AbstractType::Trigger: NotifyChain<TriggerClass>(pTarget, removed); break;
		case AbstractType::Tag: NotifyChain<TagClass>(pTarget, removed); break;
		case AbstractType::Script: NotifyChain<ScriptClass>(pTarget, removed); break;
		case AbstractType::Action: NotifyChain<TActionClass>(pTarget, removed); break;
		case AbstractType::Event: NotifyChain<TEventClass>(pTarget, removed); break;
		case AbstractType::Airstrike: NotifyChain<AirstrikeClass>(pTarget, removed); break;
		case AbstractType::RadSite: NotifyChain<RadSiteClass>(pTarget, removed); break;
		case AbstractType::UnitType: NotifyChain<UnitTypeClass>(pTarget, removed); break;
		case AbstractType::AircraftType: NotifyChain<AircraftTypeClass>(pTarget, removed); break;
		case AbstractType::InfantryType: NotifyChain<InfantryTypeClass>(pTarget, removed); break;
		case AbstractType::BuildingType: NotifyChain<BuildingTypeClass>(pTarget, removed); break;
		case AbstractType::AnimType: NotifyChain<AnimTypeClass>(pTarget, removed); break;
		case AbstractType::BulletType: NotifyChain<BulletTypeClass>(pTarget, removed); break;
		case AbstractType::OverlayType: NotifyChain<OverlayTypeClass>(pTarget, removed); break;
		case AbstractType::ParticleType: NotifyChain<ParticleTypeClass>(pTarget, removed); break;
		case AbstractType::ParticleSystemType: NotifyChain<ParticleSystemTypeClass>(pTarget, removed); break;
		case AbstractType::SmudgeType: NotifyChain<SmudgeTypeClass>(pTarget, removed); break;
		case AbstractType::TerrainType: NotifyChain<TerrainTypeClass>(pTarget, removed); break;
		case AbstractType::VoxelAnimType: NotifyChain<VoxelAnimTypeClass>(pTarget, removed); break;
		case AbstractType::HouseType: NotifyChain<HouseTypeClass>(pTarget, removed); break;
		case AbstractType::Side: NotifyChain<SideClass>(pTarget, removed); break;
		case AbstractType::TeamType: NotifyChain<TeamTypeClass>(pTarget, removed); break;
		case AbstractType::TaskForce: NotifyChain<TaskForceClass>(pTarget, removed); break;
		case AbstractType::ScriptType: NotifyChain<ScriptTypeClass>(pTarget, removed); break;
		case AbstractType::TriggerType: NotifyChain<TriggerTypeClass>(pTarget, removed); break;
		case AbstractType::TagType: NotifyChain<TagTypeClass>(pTarget, removed); break;
		case AbstractType::Tiberium: NotifyChain<TiberiumClass>(pTarget, removed); break;
		case AbstractType::SuperWeaponType: NotifyChain<SuperWeaponTypeClass>(pTarget, removed); break;
		case AbstractType::WeaponType: NotifyChain<WeaponTypeClass>(pTarget, removed); break;
		case AbstractType::WarheadType: NotifyChain<WarheadTypeClass>(pTarget, removed); break;
		default: Registry<AbstractClass>::Notify(pTarget, removed); break;
		}
	}
}
