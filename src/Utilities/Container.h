#pragma once

#include <unordered_map>

#include <CCINIClass.h>
#include <SwizzleManagerClass.h>

#include <string_view>
#include "Debug.h"
#include "Stream.h"
#include "Swizzle.h"
#include "Phobos.h"

class AbstractClass;

enum class InitState
{
	Blank = 0x0,	  // CTOR'd
	Constanted = 0x1, // values that can be set without looking at Rules (i.e. country default loadscreen)
	Ruled = 0x2,	  // Rules has been loaded and props set (i.e. country powerplants taken from [General])
	Inited = 0x3,	  // values that need the object's state (i.e. is object a secretlab? -> load default boons)
	Completed = 0x4	  // INI has been read and values set
};

/*
 * ==========================
 *	It's a kind of magic
 * ==========================

 * These two templates are the basis of the new class extension standard.

 * ==========================

 * Extension<T> is the parent class for the data you want to link with this instance of T
	( for example, [Warhead]MindControl.Permanent= should be stored in WarheadClassExt::ExtData
	which itself should be a derivate of Extension<WarheadTypeClass> )

 * ==========================

	Container<TX> is the storage for all the Extension<T> which share the same T,
	where TX is the containing class of the relevant derivate of Extension<T>. // complex, huh?
	( for example, there is Container<WarheadTypeExt>
	which contains all the custom data for all WarheadTypeClass instances,
	and WarheadTypeExt itself contains just statics like the Container itself )

	Requires:
	using base_type = T;
	const DWORD Extension<T>::Canary = (any dword value easily identifiable in a byte stream)
	class TX::ExtData : public Extension<T> { custom_data; }

	Complex? Yes. That's partially why you should be happy these are premade for you.
 *
 */

// the non-template root of every extension, mirroring Vinifera's AbstractClassExtension.
// it owns the back-pointer and the staged init state, so all extensions share a common base.
class AbstractExt
{
	AbstractClass* AttachedToObject;
	InitState Initialized;

public:
	// every extension pointer lives in the unused AbstractClass::unknown_18 field
	static constexpr size_t ExtPointerOffset = 0x18;

	// reads the inline extension slot of any AbstractClass-derived object; returns
	// null for objects of unextended types (the slot is zeroed by the game's ctor)
	static AbstractExt* Fetch(const AbstractClass* pThis)
	{
		return *reinterpret_cast<AbstractExt* const*>(reinterpret_cast<const char*>(pThis) + ExtPointerOffset);
	}

	static AbstractExt* TryFetch(const AbstractClass* pThis)
	{
		return pThis ? Fetch(pThis) : nullptr;
	}

	explicit AbstractExt(AbstractClass* const OwnerObject) : AttachedToObject { OwnerObject }, Initialized { InitState::Blank }
	{ }

	AbstractExt(const AbstractExt& other) = delete;

	void operator=(const AbstractExt& RHS) = delete;

	virtual ~AbstractExt() = default;

	void EnsureConstanted()
	{
		if (this->Initialized < InitState::Constanted)
		{
			this->InitializeConstants();
			this->Initialized = InitState::Constanted;
		}
	}

	void LoadFromINI(CCINIClass* pINI)
	{
		if (!pINI)
			return;

		switch (this->Initialized)
		{
		case InitState::Blank:
			this->EnsureConstanted();
		case InitState::Constanted:
			this->InitializeRuled();
			this->Initialized = InitState::Ruled;
		case InitState::Ruled:
			this->Initialize();
			this->Initialized = InitState::Inited;
		case InitState::Inited:
		case InitState::Completed:
			if (pINI == CCINIClass::INI_Rules)
				this->LoadFromRulesFile(pINI);

			this->LoadFromINIFile(pINI);
			this->Initialized = InitState::Completed;
		}
	}

	virtual inline void SaveToStream(PhobosStreamWriter& Stm)
	{
		Stm.Save(this->Initialized);
	}

	virtual inline void LoadFromStream(PhobosStreamReader& Stm)
	{
		Stm.Load(this->Initialized);
	}

	// on load the extension is constructed with the save-time owner pointer;
	// this queues it for remapping when the swizzle manager resolves pointers.
	void RegisterOwnerForChange()
	{
		PhobosSwizzle::RegisterPointerForChange(this->AttachedToObject);
	}

	// called after loading once all pointers (including the owner) have been remapped
	virtual void PostLoad() { }

protected:
	AbstractClass* GetAttachedObject() const
	{
		return this->AttachedToObject;
	}

	// right after construction. only basic initialization tasks possible;
	// owner object is only partially constructed! do not use global state!
	virtual void InitializeConstants() { }

	virtual void InitializeRuled() { }

	// called before the first ini file is read
	virtual void Initialize() { }

	// for things that only logically work in rules - countries, sides, etc
	virtual void LoadFromRulesFile(CCINIClass* pINI) { }

	// load any ini file: rules, game mode, scenario or map
	virtual void LoadFromINIFile(CCINIClass* pINI) { }
};

// legacy standalone base for extensions whose owners are not AbstractClass-derived
// (the Rules/Scenario/Sidebar singletons and EBolt); AbstractClass-derived owners use
// the AbstractExt hierarchy instead.
template <typename T>
class Extension
{
	T* AttachedToObject;
	InitState Initialized;

public:

	explicit Extension(T* const OwnerObject) : AttachedToObject { OwnerObject }, Initialized { InitState::Blank }
	{ }

	Extension(const Extension& other) = delete;

	void operator=(const Extension& RHS) = delete;

	virtual ~Extension() = default;

	// the object this Extension expands
	T* OwnerObject() const
	{
		return this->AttachedToObject;
	}

	void EnsureConstanted()
	{
		if (this->Initialized < InitState::Constanted)
		{
			this->InitializeConstants();
			this->Initialized = InitState::Constanted;
		}
	}

	void LoadFromINI(CCINIClass* pINI)
	{
		if (!pINI)
			return;

		switch (this->Initialized)
		{
		case InitState::Blank:
			this->EnsureConstanted();
		case InitState::Constanted:
			this->InitializeRuled();
			this->Initialized = InitState::Ruled;
		case InitState::Ruled:
			this->Initialize();
			this->Initialized = InitState::Inited;
		case InitState::Inited:
		case InitState::Completed:
			if (pINI == CCINIClass::INI_Rules)
				this->LoadFromRulesFile(pINI);

			this->LoadFromINIFile(pINI);
			this->Initialized = InitState::Completed;
		}
	}

	virtual inline void SaveToStream(PhobosStreamWriter& Stm)
	{
		Stm.Save(this->Initialized);
	}

	virtual inline void LoadFromStream(PhobosStreamReader& Stm)
	{
		Stm.Load(this->Initialized);
	}

protected:
	// right after construction. only basic initialization tasks possible;
	// owner object is only partially constructed! do not use global state!
	virtual void InitializeConstants() { }

	virtual void InitializeRuled() { }

	// called before the first ini file is read
	virtual void Initialize() { }

	// for things that only logically work in rules - countries, sides, etc
	virtual void LoadFromRulesFile(CCINIClass* pINI) { }

	// load any ini file: rules, game mode, scenario or map
	virtual void LoadFromINIFile(CCINIClass* pINI) { }
};

// a non-virtual base class for a pointer to pointer map.
// pointers are not owned by this map, so be cautious.
class ContainerMapBase final
{
public:
	using key_type = void*;
	using const_key_type = const void*;
	using value_type = void*;
	using map_type = std::unordered_map<const_key_type, value_type>;
	using const_iterator = map_type::const_iterator;
	using iterator = const_iterator;

	ContainerMapBase() = default;
	ContainerMapBase(ContainerMapBase const&) = delete;
	~ContainerMapBase() = default;

	ContainerMapBase& operator=(ContainerMapBase const&) = delete;
	ContainerMapBase& operator=(ContainerMapBase&&) = delete;

	value_type find(const_key_type key) const
	{
		auto const it = this->Items.find(key);
		if (it != this->Items.end())
			return it->second;

		return nullptr;
	}

	void insert(const_key_type key, value_type value)
	{
		this->Items.emplace(key, value);
	}

	value_type remove(const_key_type key)
	{
		auto const it = this->Items.find(key);
		if (it != this->Items.cend())
		{
			auto const value = it->second;
			this->Items.erase(it);

			return value;
		}

		return nullptr;
	}

	void clear()
	{
		// this leaks all objects inside. this case is logged.
		this->Items.clear();
	}

	size_t size() const
	{
		return this->Items.size();
	}

	const_iterator begin() const
	{
		return this->Items.cbegin();
	}

	const_iterator end() const
	{
		return this->Items.cend();
	}

private:
	map_type Items;
};

// looks like a typed map, but is really a thin wrapper around the untyped map
// pointers are not owned here either, see that each pointer is deleted
template <typename Key, typename Value>
class ContainerMap final
{
public:
	using key_type = Key*;
	using const_key_type = const Key*;
	using value_type = Value*;
	using iterator = typename std::unordered_map<key_type, value_type>::const_iterator;

	ContainerMap() = default;
	ContainerMap(ContainerMap const&) = delete;

	ContainerMap& operator=(ContainerMap const&) = delete;
	ContainerMap& operator=(ContainerMap&&) = delete;

	value_type find(const_key_type key) const
	{
		return static_cast<value_type>(this->Items.find(key));
	}

	value_type insert(const_key_type key, value_type value)
	{
		this->Items.insert(key, value);
		return value;
	}

	value_type remove(const_key_type key)
	{
		return static_cast<value_type>(this->Items.remove(key));
	}

	void clear()
	{
		this->Items.clear();
	}

	size_t size() const
	{
		return this->Items.size();
	}

	iterator begin() const
	{
		auto ret = this->Items.begin();
		return reinterpret_cast<iterator&>(ret);
	}

	iterator end() const
	{
		auto ret = this->Items.end();
		return reinterpret_cast<iterator&>(ret);
	}

private:
	ContainerMapBase Items;
};

template <class T>
concept HasOffset = requires(T) { T::ExtPointerOffset; };

template <typename T>
class Container
{
private:
	using base_type = typename T::base_type;
	using extension_type = typename T::ExtData;
	using base_type_ptr = base_type*;
	using const_base_type_ptr = const base_type*;
	using extension_type_ptr = extension_type*;
	using map_type = ContainerMap<base_type, extension_type>;

	map_type MappedItems;
	std::vector<extension_type_ptr> Items;

	const char* Name;

public:
	explicit Container(const char* pName) :
		MappedItems(),
		Items(),
		Name(pName)
	{ }

	virtual ~Container() = default;

private:
	extension_type_ptr GetExtensionPointer(const_base_type_ptr key) const
	{
		return (extension_type_ptr)(*(uintptr_t*)((char*)key + T::ExtPointerOffset));
	}

	void SetExtensionPointer(base_type_ptr key, extension_type_ptr value)
	{
		(*(uintptr_t*)((char*)key + T::ExtPointerOffset)) = (uintptr_t)value;
	}

	void ResetExtensionPointer(base_type_ptr key)
	{
		(*(uintptr_t*)((char*)key + T::ExtPointerOffset)) = 0;
	}

public:
	extension_type_ptr Allocate(base_type_ptr key)
	{
		// during savegame load extensions are restored from the stream instead
		if (Phobos::IsLoadingSaveGame)
			return nullptr;

		if constexpr (HasOffset<T>)
			ResetExtensionPointer(key);

		auto const val = new extension_type(key);

		val->EnsureConstanted();

		if constexpr (HasOffset<T>)
			SetExtensionPointer(key, val);
		else
			this->MappedItems.insert(key, val);

		Items.emplace_back(val);

		return val;
	}


	extension_type_ptr TryAllocate(base_type_ptr key, bool bCond, const std::string_view& nMessage)
	{
		if (!key || (!bCond && !nMessage.empty()))
		{
			Debug::Log("%s \n", nMessage.data());
			return nullptr;
		}

		return Allocate(key);
	}

	extension_type_ptr TryAllocate(base_type_ptr key)
	{
		if (!key)
		{
			Debug::Log("Attempted to allocate %s from nullptr!\n", typeid(extension_type).name());
			return nullptr;
		}

		return Allocate(key);
	}

	extension_type_ptr TryFind(const_base_type_ptr key) const
	{
		if (!key)
			return nullptr;

		if constexpr (HasOffset<T>)
			return GetExtensionPointer(key);
		else
			return this->MappedItems.find(key);
	}

	extension_type_ptr Find(const_base_type_ptr key) const
	{
		if constexpr (HasOffset<T>)
			return GetExtensionPointer(key);
		else
			return this->MappedItems.find(key);
	}

	void Remove(base_type_ptr key)
	{
		if (auto Item = Find(key))
		{
			if constexpr (HasOffset<T>)
				ResetExtensionPointer(key);
			else
				this->MappedItems.remove(key);

			auto& vec = this->Items;
			auto it = std::find(vec.begin(), vec.end(), Item);

			if (it != vec.end())
			{
				*it = vec.back();
				vec.pop_back();
			}

			delete Item;
		}
	}

	void Clear()
	{
		if (this->Items.size())
		{
			Debug::Log("Cleared %u items from %s.\n", this->Items.size(), this->Name);

			if constexpr (HasOffset<T>)
			{
				for (const auto& item : this->Items)
				{
					ResetExtensionPointer(item->OwnerObject());
				}
			}
			else
			{
				this->MappedItems.clear();
			}

			this->Items.clear();
		}
	}

	void LoadFromINI(const_base_type_ptr key, CCINIClass* pINI)
	{
		if (auto ptr = this->Find(key))
			ptr->LoadFromINI(pINI);
	}

	// Writes every live extension of this container into the savegame stream:
	// a header block (canary + count), then one length-prefixed block per extension
	// carrying its save-time address (for pointer swizzling), its owner's identity
	// and the serialized members.
	bool SaveAllToStream(IStream* pStm)
	{
		if constexpr (!HasOffset<T>)
		{
			return true; // map-mode extensions (EBolt) are not serialized
		}
		else
		{
			PhobosByteStream headerStm(sizeof(DWORD) * 2);
			PhobosStreamWriter headerWriter(headerStm);
			headerWriter.Save(T::Canary);
			headerWriter.Save(this->Items.size());

			if (!headerStm.WriteBlockToStream(pStm))
			{
				Debug::Log("SaveAllToStream - Failed to save header for '%s'.\n", this->Name);
				return false;
			}

			for (const auto& item : this->Items)
			{
				PhobosByteStream saver(sizeof(*item));
				PhobosStreamWriter writer(saver);

				// the extension's own save-time address, so pointers to it can be remapped
				writer.RegisterChange(item);
				// the save-time owner address, remapped to the loaded owner by the swizzle manager
				writer.Save(static_cast<void*>(item->OwnerObject()));

				item->SaveToStream(writer);

				if (!saver.WriteBlockToStream(pStm))
				{
					Debug::Log("SaveAllToStream - Failed to save an item of '%s'.\n", this->Name);
					return false;
				}
			}

			return true;
		}
	}

	// Recreates every extension of this container from the savegame stream. Owners do
	// not exist yet; each extension holds the save-time owner pointer until the swizzle
	// manager remaps it, and the owners' inline pointers are restored by
	// RelinkExtensionPointers afterwards.
	bool LoadAllFromStream(IStream* pStm)
	{
		if constexpr (!HasOffset<T>)
		{
			return true;
		}
		else
		{
			PhobosByteStream headerStm(0);
			if (!headerStm.ReadBlockFromStream(pStm))
			{
				Debug::Log("LoadAllFromStream - Failed to read header for '%s'.\n", this->Name);
				return false;
			}

			PhobosStreamReader headerReader(headerStm);
			size_t count = 0;

			if (!headerReader.Expect(T::Canary) || !headerReader.Load(count) || !headerReader.ExpectEndOfBlock())
			{
				Debug::Log("LoadAllFromStream - Invalid header for '%s'.\n", this->Name);
				return false;
			}

			this->Items.reserve(count);

			for (size_t i = 0; i < count; ++i)
			{
				PhobosByteStream loader(0);
				if (!loader.ReadBlockFromStream(pStm))
				{
					Debug::Log("LoadAllFromStream - Failed to read an item of '%s'.\n", this->Name);
					return false;
				}

				PhobosStreamReader reader(loader);

				void* oldPtr = nullptr;
				void* oldOwner = nullptr;

				if (!reader.Load(oldPtr) || !reader.Load(oldOwner))
				{
					Debug::Log("LoadAllFromStream - Invalid item header in '%s'.\n", this->Name);
					return false;
				}

				auto const buffer = new extension_type(static_cast<base_type_ptr>(oldOwner));
				buffer->RegisterOwnerForChange();
				PhobosSwizzle::RegisterChange(oldPtr, buffer);
				this->Items.emplace_back(buffer);

				buffer->LoadFromStream(reader);

				if (!reader.ExpectEndOfBlock())
					return false;
			}

			return true;
		}
	}

	// After the swizzle manager has remapped all pointers, write each extension back
	// into its owner's inline slot (the owner's loaded bytes still hold the stale
	// save-time value).
	void RelinkExtensionPointers()
	{
		if constexpr (HasOffset<T>)
		{
			for (const auto& item : this->Items)
			{
				auto const key = item->OwnerObject();

				if (!key)
					Debug::FatalErrorAndExit("RelinkExtensionPointers - '%s' extension has no owner!\n", this->Name);

				SetExtensionPointer(key, item);
				item->PostLoad();
			}
		}
	}

	decltype(auto) begin() const = delete;

	decltype(auto) end() const = delete;

	size_t size() const
	{
		return this->Items.size();
	}

private:
	Container(const Container&) = delete;
	Container& operator = (const Container&) = delete;
	Container& operator = (Container&&) = delete;
};
