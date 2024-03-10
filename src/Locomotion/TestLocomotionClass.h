#pragma once

// This file (and the corresponding .cpp one) contains an example implementation
// of a custom locomotor that goes in circles around the target. It's intended to
// be a reference for other developers in implementing their own locomotors.

// Use Shift+Ctrl+F (search across files) or see all references to the preprocessor
// symbols listed below. All of the symbol usages have brief explanations to suit as
// a sort of an implementation guide that you can view in search results.

// Use the preprocessor directives below to toggle code related to the example loco.
//                                                                      - Kerbiter

#pragma region Example locomotor toggles

// Comment/uncomment to toggle all the example locomotor code.
// #define CUSTOM_LOCO_EXAMPLE_ENABLED // Toggle TestLocomotionClass

// Comment/uncomment to toggle all the code related to piggybacking
// (temporarily replacing any locomotor with this one).
// #define CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Toggle IPiggyback impl.

// Comment/uncomment to toggle InflictLocomotor/RemoveInflictedLocomotor
// to test IPiggyback (and the tweak to disable automatic remove of the loco).
// #define LOCO_TEST_WARHEADS // Toggle loco piggyback testing

#pragma endregion


#ifdef CUSTOM_LOCO_EXAMPLE_ENABLED // Declare the loco

#include <LocomotionClass.h>
#include <FootClass.h>
#include <VocClass.h>
#include <Utilities/Debug.h>
#include <MapClass.h>

#include <Interfaces.h>

#include <comip.h>
#include <comdef.h>


class __declspec(uuid("74FC2B59-C2D3-47D7-9D10-93436A34EBB9")) // you can use any GUID generator for your loco
	TestLocomotionClass : public LocomotionClass
#ifdef CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Implement IPiggyback
	, public IPiggyback
#endif
{
public:

	//IUnknown
#ifdef CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Re-implement/override IUnknown w/ account for IPiggyback
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
#else  // no need to reimplement IUnknown since no new IUnknown interface was inherited
	// virtual HRESULT __stdcall QueryInterface(REFIID iid, LPVOID* ppvObject) override;
	// virtual ULONG __stdcall AddRef() override;
	//  virtual ULONG __stdcall Release() override;
#endif


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
#ifdef CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Cleanup Piggybacker ptr after load
			this->Piggybacker.Detach();
#endif
			// this reconstructs the object in-place, no-init constructor just refreshes
			// the virtual function table pointers because most likely they will
			// point to incorrect place due to different base address or code changes
			new (this) TestLocomotionClass(noinit_t());
		}

#ifdef CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Deserialize Piggybacker if present
		bool piggybackerPresent;
		hr = pStm->Read(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);
		if (!piggybackerPresent)
			return hr;

		hr = OleLoadFromStream(pStm, __uuidof(ILocomotion), reinterpret_cast<LPVOID*>(&this->Piggybacker));
#endif
		return hr;
	}

	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty)
	{
		// This saves the whole object
		HRESULT hr = LocomotionClass::Save(pStm, fClearDirty);
#ifdef CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Serialize Piggybacker if present
		if (FAILED(hr))
			return hr;

		// Piggybacker handling
		bool piggybackerPresent = this->Piggybacker != nullptr;
		hr = pStm->Write(&piggybackerPresent, sizeof(piggybackerPresent), nullptr);

		if (!piggybackerPresent)
			return hr;

		IPersistStreamPtr piggyPersist(this->Piggybacker);
		hr = OleSaveToStream(piggyPersist, pStm);
#endif
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
	// 		Debug::Log("TestLocomotionClass - Sucessfully linked to \"%s\"\n", Owner->get_ID());

	// 	return hr;
	// }

	virtual bool __stdcall Is_Moving() override;
	virtual CoordStruct __stdcall Destination() override;
	virtual CoordStruct __stdcall Head_To_Coord() override;
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override;
	//virtual bool __stdcall Is_To_Have_Shadow() override;
	//virtual Matrix3D __stdcall Draw_Matrix(VoxelIndexKey* key) override;
	//virtual Matrix3D __stdcall Shadow_Matrix(VoxelIndexKey* key) override;
	//virtual Point2D __stdcall Draw_Point() override;
	//virtual Point2D __stdcall Shadow_Point() override;
	//virtual VisualType __stdcall Visual_Character(bool raw) override;
	//virtual int __stdcall Z_Adjust() override;
	//virtual ZGradient __stdcall Z_Gradient() override;
	virtual bool __stdcall Process() override;
	virtual void __stdcall Move_To(CoordStruct to) override;
	virtual void __stdcall Stop_Moving() override;
	virtual void __stdcall Do_Turn(DirStruct coord) override;
	// virtual void __stdcall Unlimbo() override;
	//virtual void __stdcall Tilt_Pitch_AI() override;
	//virtual bool __stdcall Power_On() override;
	//virtual bool __stdcall Power_Off() override;
	//virtual bool __stdcall Is_Powered() override;
	//virtual bool __stdcall Is_Ion_Sensitive() override;
	//virtual bool __stdcall Push(DirStruct dir) override;
	//virtual bool __stdcall Shove(DirStruct dir) override;
	//virtual void __stdcall Force_Track(int track, CoordStruct coord) override;
	virtual Layer __stdcall In_Which_Layer() override;
	virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) override;
	//virtual void __stdcall Force_New_Slope(int ramp) override;
	virtual bool __stdcall Is_Moving_Now() override;
	//virtual int __stdcall Apparent_Speed() override;
	//virtual int __stdcall Drawing_Code() override;
	//virtual FireError __stdcall Can_Fire() override;
	//virtual int __stdcall Get_Status() override;
	//virtual void __stdcall Acquire_Hunter_Seeker_Target() override;
	//virtual bool __stdcall Is_Surfacing() override;
	virtual void __stdcall Mark_All_Occupation_Bits(MarkType mark) override;
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) override;
	//virtual bool __stdcall Will_Jump_Tracks() override;
	virtual bool __stdcall Is_Really_Moving_Now() override;
	//virtual void __stdcall Stop_Movement_Animation() override;
	virtual void __stdcall Limbo() override;
	//virtual void __stdcall Lock() override;
	//virtual void __stdcall Unlock() override;
	//virtual int __stdcall Get_Track_Number() override;
	//virtual int __stdcall Get_Track_Index() override;
	//virtual int __stdcall Get_Speed_Accum() override;

#ifdef CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Declare IPiggyback functions
	//IPiggy
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override;
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override;
	virtual bool __stdcall Is_Ok_To_End() override;
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override;
	virtual bool __stdcall Is_Piggybacking() override;
#endif

public:
	inline TestLocomotionClass() : LocomotionClass { }
		, DestinationCoord { CoordStruct::Empty }
		, HeadToCoord { CoordStruct::Empty }
		, CenterCoord { CoordStruct::Empty }
		, Angle { 0.0 }
		, IsMoving { false }
#ifdef CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Init Piggybacker
		, Piggybacker { nullptr }
#endif
	{ }

	inline TestLocomotionClass(noinit_t) : LocomotionClass { noinit_t() } { }

	inline virtual ~TestLocomotionClass() override = default;
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

#ifdef CUSTOM_LOCO_EXAMPLE_PIGGYBACK // Declare Piggybacker
		// The piggybacking locomotor.
		ILocomotionPtr Piggybacker;
#endif
};

#endif //CUSTOM_LOCO_EXAMPLE_ENABLED
