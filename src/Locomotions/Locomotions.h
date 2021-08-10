/*
* This header is the base of our new locomotors, it's based on VK's locomotor code at
* https://www.ppmforums.com/topic-15345/new-locomotor/?highlight=locomotor
* - secsome
*/

#pragma once

#include <LocomotionClass.h>

// __declspec(uuid("0C2F47D2-34F5-445a-A38A-D66C70329646"))
DEFINE_GUID(PhobosLocomotionClass_CLSID, 0xC2F47D2, 0x34F5, 0x445A, 0xA3, 0x8A, 0xD6, 0x6C, 0x70, 0x32, 0x96, 0x46);
class PhobosLocomotionClass : public LocomotionClass
{
public:
	static constexpr CLSID ClassID { 0xC2F47D2, 0x34F5, 0x445A, {0xA3, 0x8A, 0xD6, 0x6C, 0x70, 0x32, 0x96, 0x46} };

	template<typename T>
	static YRComPtr<T> CreateInstance(const CLSID& rclsid)
	{
		static_assert(std::is_base_of<PhobosLocomotionClass, T>::value, "T must be inherited from PhobosLocomotionClass.");

		return YRComPtr<T>(rclsid, nullptr,
			CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER);
	}

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	virtual ULONG __stdcall AddRef() R0;
	virtual ULONG __stdcall Release() R0;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall IsDirty() R0;
	virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) R0;

	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) R0;

	virtual ~PhobosLocomotionClass() RX;
	virtual int Size() R0;

	// ILocomotion
	// virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	// virtual ULONG __stdcall AddRef() R0;
	// virtual ULONG __stdcall Release() R0;
	virtual HRESULT __stdcall Link_To_Object(void* pointer) R0;
	virtual bool __stdcall Is_Moving() R0;
	virtual CoordStruct* __stdcall Destination(CoordStruct* pcoord) R0;
	virtual CoordStruct* __stdcall Head_To_Coord(CoordStruct* pcoord) R0;
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) RT(Move);
	virtual bool __stdcall Is_To_Have_Shadow() R0;
	virtual Matrix3D __stdcall Draw_Matrix(int* facing) RT(Matrix3D);
	virtual Matrix3D __stdcall Shadow_Matrix(int* facing) RT(Matrix3D);
	virtual Point2D* __stdcall Draw_Point(Point2D* pPoint) R0;
	virtual Point2D* __stdcall Shadow_Point(Point2D* pPoint) R0;
	virtual VisualType __stdcall Visual_Character(VARIANT_BOOL unused) RT(VisualType);
	virtual int __stdcall Z_Adjust() R0;
	virtual ZGradient __stdcall Z_Gradient() RT(ZGradient);
	virtual bool __stdcall Process() R0;
	virtual void __stdcall Move_To(CoordStruct to) RX;
	virtual void __stdcall Stop_Moving() RX;
	virtual void __stdcall Do_Turn(DirStruct coord) RX;
	virtual void __stdcall Unlimbo() RX;
	virtual void __stdcall Tilt_Pitch_AI() RX;
	virtual bool __stdcall Power_On() R0;
	virtual bool __stdcall Power_Off() R0;
	virtual bool __stdcall Is_Powered() R0;
	virtual bool __stdcall Is_Ion_Sensitive() R0;
	virtual bool __stdcall Push(DirStruct dir) R0;
	virtual bool __stdcall Shove(DirStruct dir) R0;
	virtual void __stdcall Force_Track(int track, CoordStruct coord) RX;
	virtual Layer __stdcall In_Which_Layer() RT(Layer);
	virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) RX;
	virtual void __stdcall Force_New_Slope(int ramp) RX;
	virtual bool __stdcall Is_Moving_Now() R0;
	virtual int __stdcall Apparent_Speed() R0;
	virtual int __stdcall Drawing_Code() R0;
	virtual FireError __stdcall Can_Fire() RT(FireError);
	virtual int __stdcall Get_Status() R0;
	virtual void __stdcall Acquire_Hunter_Seeker_Target() RX;
	virtual bool __stdcall Is_Surfacing() R0;
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) RX;
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) R0;
	virtual bool __stdcall Will_Jump_Tracks() R0;
	virtual bool __stdcall Is_Really_Moving_Now() R0;
	virtual void __stdcall Stop_Movement_Animation() RX;
	virtual void __stdcall Lock() RX;
	virtual void __stdcall Unlock() RX;
	virtual void __stdcall ILocomotion_B8() RX;
	virtual int __stdcall Get_Track_Number() R0;
	virtual int __stdcall Get_Track_Index() R0;
	virtual int __stdcall Get_Speed_Accum() R0;

	PhobosLocomotionClass() : LocomotionClass(noinit_t())
	{

	}
};

class PhobosLocomotorFactory : public IClassFactory
{
protected:
	ULONG	RefCount;
public:
	using Locomotion = PhobosLocomotionClass;

	PhobosLocomotorFactory();
	~PhobosLocomotorFactory();

	//IUnknown
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR*);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IClassFactory members
	virtual STDMETHODIMP		CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR*) = 0;
	STDMETHODIMP		LockServer(BOOL);
};

#define LOCO_FACTORY(name, locomotionname) \
class name : public PhobosLocomotorFactory \
{ \
public: \
	using Locomotion = locomotionname; \
	virtual STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, LPVOID* ppvObj) \
	{ \
		if(!ppvObj) return E_INVALIDARG; \
		*ppvObj = nullptr; \
		if (pUnkOuter) return CLASS_E_NOAGGREGATION; \
		auto pLoco = GameCreate<locomotionname>(); \
		if (pLoco == nullptr) return E_OUTOFMEMORY; \
		HRESULT hr = pLoco->QueryInterface(riid, ppvObj); \
		if(FAILED(hr)) GameDelete(pLoco); \
		return hr; \
	} \
};

template <typename...>
struct DummyTypes { };

struct GetObjectHelper
{
	template <typename T>
	static HRESULT Process(REFCLSID rclsid, REFIID riid, LPVOID* ppvOut)
	{
		return getObject<T>(rclsid, riid, ppvOut);
	}

private:
	template <typename T>
	static HRESULT getObject(REFCLSID rclsid, REFIID riid, LPVOID* ppvOut)
	{
		*ppvOut = nullptr;

		if (IsEqualIID(rclsid, T::Locomotion::ClassID))
		{
			T* pcf = GameCreate<T>();
			return pcf->QueryInterface(riid, ppvOut);
		}

		return CLASS_E_CLASSNOTAVAILABLE;
	}
};

struct CoRegistHelper
{
	template <typename T>
	static HRESULT Process(LPUNKNOWN pUkn, LPDWORD lpdwRegister)
	{
		return coRegist<T>(T::Locomotion::ClassID, pUkn, lpdwRegister);
	}

private:
	template <typename T>
	static HRESULT coRegist(REFCLSID rclsid, LPUNKNOWN pUkn,LPDWORD lpdwRegister)
	{
		CoRegisterClassObject(rclsid, pUkn, 1, 1, lpdwRegister);

		auto ComClasses = (DynamicVectorClass<unsigned long>*)0xB0BC88;
		ComClasses->AddItem(*lpdwRegister);
		if (auto pFactory = GameCreate<T>())
			pUkn = pFactory;
		else
			pUkn = nullptr;

		return NOERROR;
	}
};

// this is a complicated thing that calls methods on classes. add types to the
// instantiation of this type, and the most appropriate method for each type
// will be called with no overhead of virtual functions.
template <typename... Ts>
struct LocomotorCollection
{
public:
	__forceinline HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppvOut)
	{
		return process<GetObjectHelper>(DummyTypes<Ts...>(), rclsid, riid, ppvOut);
	}

	__forceinline void CoRegisterClassObject(LPUNKNOWN pUkn, LPDWORD lpdwRegister)
	{
		process<CoRegistHelper>(DummyTypes<Ts...>(), pUkn, lpdwRegister);
	}

private:
	// T: the method dispatcher class to call with each type
	// TArgs: the arguments to call the method dispatcher's Process() method
	// TType and TTypes: setup for recursion. TType is the first type, the one
	// to handle now. TTypes is the tail that is recursively went into.

	// this is the base case, no more types, nothing to call
	template <typename T, typename... TArgs>
	HRESULT process(DummyTypes<>, TArgs... args)
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	// this is the recursion part: invoke T:Process() for first type, then
	// recurse with the remaining types
	template <typename T, typename... TArgs, typename TType, typename... TTypes>
	__forceinline HRESULT process(DummyTypes<TType, TTypes...>, TArgs... args)
	{
		HRESULT hr = T::Process<TType>(args...);
		if (hr != CLASS_E_CLASSNOTAVAILABLE)
			return hr;

		return process<T>(DummyTypes<TTypes...>(), args...);
	}
};