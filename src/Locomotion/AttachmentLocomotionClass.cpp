#include "AttachmentLocomotionClass.h"

#include <CellSpread.h>
#include <ScenarioClass.h>

#include <ParticleSystemClass.h>
#include <ParticleSystemTypeClass.h>

#include <AnimClass.h>
#include <AircraftClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Ext/Techno/Body.h>
#include <New/Entity/AttachmentClass.h>
#include <New/Type/AttachmentTypeClass.h>

#include <cmath>

// TODO maybe some macros for repeated parent function calls?

bool AttachmentLocomotionClass::Is_Moving()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco && pParentLoco->Is_Moving();
}

Matrix3D AttachmentLocomotionClass::Draw_Matrix(VoxelIndexKey* key)
{
	if (auto const pParentFoot = abstract_cast<FootClass*>(this->GetAttachmentParent()))
	{
		Matrix3D mtx = pParentFoot->Locomotor->Draw_Matrix(key);

		// adjust for the real facing which is the source of truth for hor. rotation
		double childRotation = this->LinkedTo->PrimaryFacing.Current().GetRadian<32>();
		double parentRotation = pParentFoot->PrimaryFacing.Current().GetRadian<32>();
		float adjustmentAngle = (float)(childRotation - parentRotation);

		mtx.RotateZ(adjustmentAngle);

		if (key && key->Is_Valid_Key())
			key->MainVoxel.FrameIndex = this->LinkedTo->PrimaryFacing.Current().GetFacing<32>();

		return mtx;
	}

	return LocomotionClass::Draw_Matrix(key);
}

Matrix3D AttachmentLocomotionClass::Shadow_Matrix(VoxelIndexKey* key)
{
	if (auto const pParentFoot = abstract_cast<FootClass*>(this->GetAttachmentParent()))
	{
		Matrix3D mtx = pParentFoot->Locomotor->Shadow_Matrix(key);

		// adjust for the real facing which is the source of truth for hor. rotation
		double childRotation = this->LinkedTo->PrimaryFacing.Current().GetRadian<32>();
		double parentRotation = pParentFoot->PrimaryFacing.Current().GetRadian<32>();
		float adjustmentAngle = (float)(childRotation - parentRotation);

		mtx.RotateZ(adjustmentAngle);

		// should be shadow key in fact but I kerbo is lazy to properly define shadow key
		if (key && key->Is_Valid_Key())
			key->MainVoxel.FrameIndex = this->LinkedTo->PrimaryFacing.Current().GetFacing<32>();

		return mtx;
	}

	return LocomotionClass::Shadow_Matrix(key);
}

Point2D AttachmentLocomotionClass::Draw_Point()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Draw_Point()
		: LocomotionClass::Draw_Point();
}

// Point2D AttachmentLocomotionClass::Shadow_Point()
// {
// 	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
// 	return pParentLoco
// 		? pParentLoco->Shadow_Point()
// 		: LocomotionClass::Shadow_Point();
// }

VisualType AttachmentLocomotionClass::Visual_Character(bool raw)
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Visual_Character(raw)
		: LocomotionClass::Visual_Character(raw);
}

int AttachmentLocomotionClass::Z_Adjust()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Z_Adjust()
		: LocomotionClass::Z_Adjust();
}

ZGradient AttachmentLocomotionClass::Z_Gradient()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Z_Gradient()
		: LocomotionClass::Z_Gradient();
}


bool AttachmentLocomotionClass::Process()
{
	if (this->LinkedTo->IsAlive)
    {
        Layer newLayer = this->In_Which_Layer();
        if (this->PreviousLayer != newLayer)
            DisplayClass::Instance->Submit(this->LinkedTo);
    }

	return LocomotionClass::Process();
}

bool AttachmentLocomotionClass::Is_Powered()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		&& pParentLoco->Is_Powered();
}

bool AttachmentLocomotionClass::Is_Ion_Sensitive()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		&& pParentLoco->Is_Ion_Sensitive()
		|| LocomotionClass::Is_Ion_Sensitive();
}

Layer AttachmentLocomotionClass::In_Which_Layer()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->In_Which_Layer()
		: Layer::Ground;
}

bool AttachmentLocomotionClass::Is_Moving_Now()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco && pParentLoco->Is_Moving_Now();
}

int AttachmentLocomotionClass::Apparent_Speed()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Apparent_Speed()
		: LocomotionClass::Apparent_Speed();
}

FireError AttachmentLocomotionClass::Can_Fire()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Can_Fire()
		: LocomotionClass::Can_Fire();
}

int AttachmentLocomotionClass::Get_Status()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		? pParentLoco->Get_Status()
		: LocomotionClass::Get_Status();
}

bool AttachmentLocomotionClass::Is_Surfacing()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco
		&& pParentLoco->Is_Surfacing()
		|| LocomotionClass::Is_Surfacing();
}

bool AttachmentLocomotionClass::Is_Really_Moving_Now()
{
	ILocomotionPtr pParentLoco = this->GetAttachmentParentLoco();
	return pParentLoco && pParentLoco->Is_Really_Moving_Now();
}

HRESULT AttachmentLocomotionClass::Begin_Piggyback(ILocomotion* pointer)
{
	if (!pointer)
		return E_POINTER;

	if (this->Piggybacker)
		return E_FAIL;

	this->Piggybacker = pointer;

	return S_OK;
}

HRESULT AttachmentLocomotionClass::End_Piggyback(ILocomotion** pointer)
{
	if (!pointer)
		return E_POINTER;

	if (!this->Piggybacker)
		return S_FALSE;

	// since pointer is a dumb pointer, we don't need to call Release,
	// hence we use Detach, otherwise the locomotor gets trashed
	*pointer = this->Piggybacker.Detach();

	// in order to play nice with IsLocomotor warheads probably also should
	// handle IsAttackedByLocomotor etc. warheads here, but none of the vanilla
	// warheads do this (except JumpjetLocomotionClass::End_Piggyback)

	return S_OK;
}

bool AttachmentLocomotionClass::Is_Ok_To_End()
{
	// Actually a confusing name, should return true only if the piggybacking should be ended.
	return this->Piggybacker
		&& !this->GetAttachmentParent();
}

HRESULT AttachmentLocomotionClass::Piggyback_CLSID(GUID* classid)
{
	HRESULT hr;

	if (classid == nullptr)
		return E_POINTER;

	if (this->Piggybacker)
	{
		IPersistStreamPtr piggyAsPersist(this->Piggybacker);

		hr = piggyAsPersist->GetClassID(classid);
	}
	else
	{
		if (reinterpret_cast<IPiggyback*>(this) == nullptr)
			return E_FAIL;

		IPersistStreamPtr thisAsPersist(this);

		if (thisAsPersist == nullptr)
			return E_FAIL;

		hr = thisAsPersist->GetClassID(classid);
	}

	return hr;
}

bool AttachmentLocomotionClass::Is_Piggybacking()
{
	return this->Piggybacker != nullptr;
}

// non-virtuals

AttachmentClass* AttachmentLocomotionClass::GetAttachment()
{
	AttachmentClass* result = nullptr;

	if (this->LinkedTo)
	{
		if (auto const pExt = TechnoExt::ExtMap.Find(this->LinkedTo))
			result = pExt->ParentAttachment;
	}

	return result;
}

TechnoClass* AttachmentLocomotionClass::GetAttachmentParent()
{
	TechnoClass* result = nullptr;

	if (auto const pAttachment = this->GetAttachment())
		result = pAttachment->Parent;

	return result;
}

ILocomotionPtr AttachmentLocomotionClass::GetAttachmentParentLoco()
{
	ILocomotionPtr result { };

	if (auto const pTechno = this->GetAttachmentParent())
	{
		if (auto const pFoot = abstract_cast<FootClass*>(pTechno))
			result = pFoot->Locomotor;
	}

	return result;
}
