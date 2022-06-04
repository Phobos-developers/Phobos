#include "PhobosGlobal.h"

//GlobalObject initial
PhobosGlobal PhobosGlobal::GlobalObject;

PhobosGlobal* PhobosGlobal::Global()
{
	return &GlobalObject;
}

void PhobosGlobal::Clear()
{
	GlobalObject.Reset();
}

void PhobosGlobal::Reset()
{
	RandomTriggerPool.clear();
}

//Save/Load
#pragma region save/load

template <typename T>
bool PhobosGlobal::Serialize(T& stm)
{
	return stm
		.Process(this->RandomTriggerPool)
		.Success();
}

template <typename T>
bool PhobosGlobal::SerializeGlobal(T& stm)
{
	bool res = true;
	res &= ProcessTechnoType(stm);
	res &= ProcessTechno(stm);
	return res;
}

template <typename T>
bool PhobosGlobal::ProcessTechnoType(T& stm)
{
	//for (int i = 0; i < TechnoTypeClass::Array->Count; i++)
	//{
	//	TechnoTypeClass* pItem = TechnoTypeClass::Array->GetItem(i);
	//	TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pItem);
	//	{// Process region
	//		stm
	//			.Process(pExt->AttachmentData)
	//			;
	//	}
	//}
	return stm.Success();
}

template <typename T>
bool PhobosGlobal::ProcessTechno(T& stm)
{
	//for (int i = 0; i < TechnoClass::Array->Count; i++)
	//{
	//	TechnoClass* pItem = TechnoClass::Array->GetItem(i);
	//	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pItem);
	//	{// Process region
	//		stm
	//			;
	//	}
	//}
	return stm.Success();
}

bool PhobosGlobal::Save(PhobosStreamWriter& stm)
{
	return Serialize(stm);
}

bool PhobosGlobal::Load(PhobosStreamReader& stm)
{
	return Serialize(stm);
}

bool PhobosGlobal::SaveGlobals(PhobosStreamWriter& stm)
{
	return
		SerializeGlobal(stm)
		&& GlobalObject.Save(stm);
}

bool PhobosGlobal::LoadGlobals(PhobosStreamReader& stm)
{
	return
		SerializeGlobal(stm)
		&& GlobalObject.Load(stm);
}

#pragma endregion save/load