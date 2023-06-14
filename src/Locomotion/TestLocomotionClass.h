#pragma once

#include <LocomotionClass.h>
#include <FootClass.h>
#include <VocClass.h>
#include <Utilities/Debug.h>
#include <MapClass.h>

#include <Interfaces.h>

#include <comip.h>
#include <comdef.h>


class __declspec(uuid("74FC2B59-C2D3-47D7-9D10-93436A34EBB9"))
	TestLocomotionClass : public LocomotionClass
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
	// virtual HRESULT __stdcall IsDirty() { return LocomotionClass::IsDirty(); }
	virtual HRESULT __stdcall Load(IStream* pStm)
	{
		// This loads the whole object
		HRESULT hr = LocomotionClass::Load(pStm);
		if (FAILED(hr))
			return hr;

		if (this)
		{
			// clean up the loaded Piggybacker pointer
			this->Piggybacker.Detach();
			new (this) TestLocomotionClass(noinit_t());
		}

		// Piggybacker handling
		bool piggybackerPresent;
		hr = pStm->Read(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);
		if (!piggybackerPresent)
			return hr;

		return OleLoadFromStream(pStm, __uuidof(ILocomotion), reinterpret_cast<LPVOID*>(&this->Piggybacker));
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
		return OleSaveToStream(piggyPersist, pStm);
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
	// 		Debug::Log("TestLocomotionClass - Sucessfully linked to \"%s\"\n", Owner->get_ID());

	// 	return hr;
	// }

	virtual bool __stdcall Is_Moving() override;
	virtual CoordStruct __stdcall Destination() override;
	virtual CoordStruct __stdcall Head_To_Coord() override;
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override;
	//virtual bool __stdcall Is_To_Have_Shadow() override { return LocomotionClass::Is_To_Have_Shadow(); }
	//virtual Matrix3D __stdcall Draw_Matrix(VoxelIndexKey* key) override { return LocomotionClass::Draw_Matrix(key); }
	//virtual Matrix3D __stdcall Shadow_Matrix(VoxelIndexKey* key) override { return LocomotionClass::Shadow_Matrix(key); }
	//virtual Point2D __stdcall Draw_Point() override	{ return LocomotionClass::Draw_Point(); }
	//virtual Point2D __stdcall Shadow_Point() override { return LocomotionClass::Shadow_Point(); }
	//virtual VisualType __stdcall Visual_Character(bool raw) override;
	//virtual int __stdcall Z_Adjust() override { return LocomotionClass::Z_Adjust(); }
	//virtual ZGradient __stdcall Z_Gradient() override { return LocomotionClass::Z_Gradient(); }
	virtual bool __stdcall Process() override;
	virtual void __stdcall Move_To(CoordStruct to) override;
	virtual void __stdcall Stop_Moving() override;
	virtual void __stdcall Do_Turn(DirStruct coord) override;
	// virtual void __stdcall Unlimbo() override;
	//virtual void __stdcall Tilt_Pitch_AI() override;
	//virtual bool __stdcall Power_On() override { return LocomotionClass::Power_On(); }
	//virtual bool __stdcall Power_Off() override { return LocomotionClass::Power_Off(); }
	//virtual bool __stdcall Is_Powered() override { return Powered; }
	//virtual bool __stdcall Is_Ion_Sensitive() override { return false; }
	//virtual bool __stdcall Push(DirStruct dir) override { return false; }
	//virtual bool __stdcall Shove(DirStruct dir) override { return false; }
	//virtual void __stdcall Force_Track(int track, CoordStruct coord) override { }
	virtual Layer __stdcall In_Which_Layer() override { return Layer::Ground; }
	virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) override;
	//virtual void __stdcall Force_New_Slope(int ramp) override { }
	virtual bool __stdcall Is_Moving_Now() override;
	//virtual int __stdcall Apparent_Speed() override { return LinkedTo->GetCurrentSpeed(); }
	//virtual int __stdcall Drawing_Code() override { return 0; }
	//virtual FireError __stdcall Can_Fire() override { return FireError::OK; }
	//virtual int __stdcall Get_Status() override { return 0; }
	//virtual void __stdcall Acquire_Hunter_Seeker_Target() override { }
	//virtual bool __stdcall Is_Surfacing() override { return LocomotionClass::Is_Surfacing(); }
	virtual void __stdcall Mark_All_Occupation_Bits(MarkType mark) override;
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) override;
	//virtual bool __stdcall Will_Jump_Tracks() override { return LocomotionClass::Will_Jump_Tracks(); }
	virtual bool __stdcall Is_Really_Moving_Now() override;
	//virtual void __stdcall Stop_Movement_Animation() override { LocomotionClass::Stop_Movement_Animation(); }
	virtual void __stdcall Clear_Coords() override;
	//virtual void __stdcall Lock() override { LocomotionClass::Lock(); }
	//virtual void __stdcall Unlock() override { LocomotionClass::Unlock(); }
	//virtual int __stdcall Get_Track_Number() override { return LocomotionClass::Get_Track_Number(); }
	//virtual int __stdcall Get_Track_Index() override { return LocomotionClass::Get_Track_Index(); }
	//virtual int __stdcall Get_Speed_Accum() override { return LocomotionClass::Get_Speed_Accum(); }

	//IPiggy
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override;
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override;
	virtual bool __stdcall Is_Ok_To_End() override;
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override;
	virtual bool __stdcall Is_Piggybacking() override;

public:
	inline TestLocomotionClass() : LocomotionClass { }
		, DestinationCoord { }
		, HeadToCoord { }
		, CenterCoord { }
		, Angle { 0.0 }
		, IsMoving { false }
		, Piggybacker { nullptr }
	{ }

	inline TestLocomotionClass(noinit_t) : LocomotionClass {noinit_t()}
	{ }

	inline virtual ~TestLocomotionClass() override = default; // should be SDDTOR in fact
	virtual int Size() override { return sizeof(*this); }

public:
		// This is the desired destination coordinate of the object.
		CoordStruct DestinationCoord;

		// This is the coordinate that the unit is heading to as an immediate
		// destination. This coordinate is never further than once cell (or track)
		// from the unit's location. When this coordinate is reached, then the
		// next location in the path list becomes the next HeadTo coordinate.
		CoordStruct HeadToCoord;

		//  This is the logical coordinate for the object. It is the center of
		//  the circle when calculating the rotation.
		CoordStruct CenterCoord;

		//  The current rotation angle.
		double Angle;

		//  If this object is moving, then this flag will be true.
		bool IsMoving;

		// The piggybacking locomotor.
		ILocomotionPtr Piggybacker;
};
