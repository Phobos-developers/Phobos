#pragma once

#include <LocomotionClass.h>
#include <FootClass.h>
#include <VocClass.h>
#include <Utilities/Debug.h>
#include <MapClass.h>

#include <Interfaces.h>

#include <comip.h>
#include <comdef.h>

class AttachmentClass;


class __declspec(uuid("C5D54B98-8C98-4275-8CE4-EF75CB0CBE3E"))
	AttachmentLocomotionClass : public LocomotionClass
	, public IPiggyback
{
public:

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID* ppvObject)
	{
		HRESULT hr = LocomotionClass::QueryInterface(iid, ppvObject);
		if (hr != E_NOINTERFACE)
			return hr;

		if (iid == __uuidof(IPiggyback))
		{
			*ppvObject = static_cast<IPiggyback*>(this);
			this->AddRef();
			return S_OK;
		}

		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	virtual ULONG __stdcall AddRef() { return LocomotionClass::AddRef(); }
	virtual ULONG __stdcall Release() { return LocomotionClass::Release(); }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID)
	{
		if (pClassID == nullptr)
			return E_POINTER;

		*pClassID = __uuidof(this);

		return S_OK;
	}

	//IPersistStream
	// virtual HRESULT __stdcall IsDirty() override;

	virtual HRESULT __stdcall Load(IStream* pStm)
	{
		// This loads the whole object
		HRESULT hr = LocomotionClass::Load(pStm);
		if (FAILED(hr))
			return hr;

		if (this)
		{
			this->Piggybacker.Detach();
			// this reconstructs the object in-place, no-init constructor just refreshes
			// the virtual function table pointers because most likely they will
			// point to incorrect place due to different base address or code changes
			new (this) AttachmentLocomotionClass(noinit_t());
		}

		bool piggybackerPresent;
		hr = pStm->Read(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);
		if (!piggybackerPresent)
			return hr;

		hr = OleLoadFromStream(pStm, __uuidof(ILocomotion), reinterpret_cast<LPVOID*>(&this->Piggybacker));
		return hr;
	}

	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty)
	{
		// This saves the whole object
		HRESULT hr = LocomotionClass::Save(pStm, fClearDirty);
		if (FAILED(hr))
			return hr;

		// Piggybacker handling
		bool piggybackerPresent = this->Piggybacker != nullptr;
		hr = pStm->Write(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);

		if (!piggybackerPresent)
			return hr;

		IPersistStreamPtr piggyPersist(this->Piggybacker);
		hr = OleSaveToStream(piggyPersist, pStm);
		return hr;
	}

	// virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize)
	// {
	// 	if (pcbSize == nullptr)
	// 		return E_POINTER;

	// 	return LocomotionClass::GetSizeMax(pcbSize);
	// }

	// virtual HRESULT __stdcall Link_To_Object(void* pointer) override
	// {
	// 	HRESULT hr = LocomotionClass::Link_To_Object(pointer);

	// 	if (SUCCEEDED(hr))
	// 		Debug::Log("AttachmentLocomotionClass - Sucessfully linked to \"%s\"\n", Owner->get_ID());

	// 	return hr;
	// }

	virtual bool __stdcall Is_Moving() override;
	// virtual CoordStruct __stdcall Destination() override;
	// virtual CoordStruct __stdcall Head_To_Coord() override;
	// virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override;
	//virtual bool __stdcall Is_To_Have_Shadow() override;
	virtual Matrix3D __stdcall Draw_Matrix(VoxelIndexKey* key) override;
	// virtual Matrix3D __stdcall Shadow_Matrix(VoxelIndexKey* key) override;
	virtual Point2D __stdcall Draw_Point() override;
	// virtual Point2D __stdcall Shadow_Point() override;
	virtual VisualType __stdcall Visual_Character(bool raw) override;
	virtual int __stdcall Z_Adjust() override;
	virtual ZGradient __stdcall Z_Gradient() override;
	virtual bool __stdcall Process() override;
	// virtual void __stdcall Move_To(CoordStruct to) override;
	// virtual void __stdcall Stop_Moving() override;
	// virtual void __stdcall Do_Turn(DirStruct coord) override;
	// virtual void __stdcall Unlimbo() override;
	//virtual void __stdcall Tilt_Pitch_AI() override;
	//virtual bool __stdcall Power_On() override;
	//virtual bool __stdcall Power_Off() override;
	virtual bool __stdcall Is_Powered() override;
	virtual bool __stdcall Is_Ion_Sensitive() override;
	//virtual bool __stdcall Push(DirStruct dir) override;
	//virtual bool __stdcall Shove(DirStruct dir) override;
	//virtual void __stdcall Force_Track(int track, CoordStruct coord) override;
	virtual Layer __stdcall In_Which_Layer() override;
	//virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) override;
	//virtual void __stdcall Force_New_Slope(int ramp) override;
	virtual bool __stdcall Is_Moving_Now() override;
	virtual int __stdcall Apparent_Speed() override;
	//virtual int __stdcall Drawing_Code() override;
	virtual FireError __stdcall Can_Fire() override;
	virtual int __stdcall Get_Status() override;
	//virtual void __stdcall Acquire_Hunter_Seeker_Target() override;
	virtual bool __stdcall Is_Surfacing() override;
	// virtual void __stdcall Mark_All_Occupation_Bits(MarkType mark) override;
	// virtual bool __stdcall Is_Moving_Here(CoordStruct to) override;
	//virtual bool __stdcall Will_Jump_Tracks() override;
	virtual bool __stdcall Is_Really_Moving_Now() override;
	//virtual void __stdcall Stop_Movement_Animation() override;
	//virtual void __stdcall Clear_Coords() override;
	//virtual void __stdcall Lock() override;
	//virtual void __stdcall Unlock() override;
	//virtual int __stdcall Get_Track_Number() override;
	//virtual int __stdcall Get_Track_Index() override;
	//virtual int __stdcall Get_Speed_Accum() override;

	//IPiggy
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override;
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override;
	virtual bool __stdcall Is_Ok_To_End() override;
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override;
	virtual bool __stdcall Is_Piggybacking() override;

private:
	// Shortcut to attachment the LinkedTo is attached to.
	AttachmentClass* GetAttachment();

	// Shortcut to parent techno of this locomotor's owner.
	TechnoClass* GetAttachmentParent();

	// Shortcut to parent techno of this locomotor's owner.
	ILocomotionPtr GetAttachmentParentLoco();

public:
	inline AttachmentLocomotionClass() : LocomotionClass { }
		, PreviousLayer { Layer::None }
		, Piggybacker { nullptr }
	{ }

	inline AttachmentLocomotionClass(noinit_t) : LocomotionClass { noinit_t() } { }

	inline virtual ~AttachmentLocomotionClass() override = default;
	virtual int Size() override { return sizeof(*this); }

public:
		// The layer this locomotor's user was in previously.
		// Used for resubmitting the FootClass to another layer.
		Layer PreviousLayer;

		// The piggybacking locomotor.
		ILocomotionPtr Piggybacker;
};

