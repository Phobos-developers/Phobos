#pragma once

#include <unordered_map>

#include <CCINIClass.h>
#include <SwizzleManagerClass.h>

#include "../Utilities/Debug.h"
#include "../Misc/Stream.h"

enum class InitState {
	Blank = 0x0, // CTOR'd
	Constanted = 0x1, // values that can be set without looking at Rules (i.e. country default loadscreen)
	Ruled = 0x2, // Rules has been loaded and props set (i.e. country powerplants taken from [General])
	Inited = 0x3, // values that need the object's state (i.e. is object a secretlab? -> load default boons)
	Completed = 0x4 // INI has been read and values set
};

/*
 * ==========================
 *    It's a kind of magic
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
     and WarheadTypeExt itself contains just statics like the Container itself

      )

   Requires:
   	using base_type = T;
   	const DWORD Extension<T>::Canary = (any dword value easily identifiable in a byte stream)
   	class TX::ExtData : public Extension<T> { custom_data; }

   Complex? Yes. That's partially why you should be happy these are premade for you.
 *
 */

template<typename T>
class Extension {
	T* AttachedToObject;
	InitState Initialized;
public:

	static const DWORD Canary;

	Extension(T* const OwnerObject) :
		AttachedToObject(OwnerObject),
		Initialized(InitState::Blank)
	{ }

	Extension(const Extension &other) = delete;

	void operator = (const Extension &RHS) = delete;

	virtual ~Extension() = default;

	// the object this Extension expands
	T* const& OwnerObject() const {
		return this->AttachedToObject;
	}

	void EnsureConstanted() {
		if(this->Initialized < InitState::Constanted) {
			this->InitializeConstants();
			this->Initialized = InitState::Constanted;
		}
	}

	void LoadFromINI(CCINIClass* pINI) {
		if(!pINI) {
			return;
		}

		switch(this->Initialized) {
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
			if(pINI == CCINIClass::INI_Rules) {
				this->LoadFromRulesFile(pINI);
			}
			this->LoadFromINIFile(pINI);
			this->Initialized = InitState::Completed;
		}
	}

	virtual void InvalidatePointer(void* ptr, bool bRemoved) = 0;

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
class ContainerMapBase final {
public:
	using key_type = void*;
	using const_key_type = const void*;
	using value_type = void*;
	using map_type = std::unordered_map<const_key_type, value_type>;
	using const_iterator = map_type::const_iterator;
	using iterator = const_iterator;

	ContainerMapBase();
	ContainerMapBase(ContainerMapBase const&) = delete;
	~ContainerMapBase();

	ContainerMapBase& operator =(ContainerMapBase const&) = delete;
	ContainerMapBase& operator =(ContainerMapBase&&) = delete;

	value_type find(const_key_type key) const;
	void insert(const_key_type key, value_type value);
	value_type remove(const_key_type key);
	void clear();

	size_t size() const {
		return this->Items.size();
	}

	const_iterator begin() const {
		return this->Items.cbegin();
	}

	const_iterator end() const {
		return this->Items.cend();
	}

private:
	map_type Items;
};

// looks like a typed map, but is really a thin wrapper around the untyped map
// pointers are not owned here either, see that each pointer is deleted
template<typename Key, typename Value>
class ContainerMap final {
public:
	using key_type = Key*;
	using const_key_type = const Key*;
	using value_type = Value*;
	using iterator = typename std::unordered_map<key_type, value_type>::const_iterator;

	ContainerMap() = default;
	ContainerMap(ContainerMap const&) = delete;

	ContainerMap& operator =(ContainerMap const&) = delete;
	ContainerMap& operator =(ContainerMap&&) = delete;

	value_type find(const_key_type key) const {
		return static_cast<value_type>(this->Items.find(key));
	}

	value_type insert(const_key_type key, value_type value) {
		this->Items.insert(key, value);
		return value;
	}

	value_type remove(const_key_type key) {
		return static_cast<value_type>(this->Items.remove(key));
	}

	void clear() {
		this->Items.clear();
	}

	size_t size() const {
		return this->Items.size();
	}

	iterator begin() const {
		auto ret = this->Items.begin();
		return reinterpret_cast<iterator&>(ret);
	}

	iterator end() const {
		auto ret = this->Items.end();
		return reinterpret_cast<iterator&>(ret);
	}

private:
	ContainerMapBase Items;
};

template<typename T>
class Container {
private:
	using base_type = typename T::base_type;
	using extension_type = typename T::ExtData;
	using key_type = base_type*;
	using const_key_type = const base_type*;
	using value_type = extension_type*;
	using map_type = ContainerMap<base_type, extension_type>;

	map_type Items;
public:
	base_type* SavingObject;
	IStream* SavingStream;
private:
	const char* Name;

public:
	explicit Container(const char* pName) : Items(),
		SavingObject(nullptr),
		SavingStream(nullptr),
		Name(pName)
	{ }

	virtual ~Container() = default;

	void PointerGotInvalid(void *ptr, bool bRemoved) {
		this->InvalidatePointer(ptr, bRemoved);
		if(!this->InvalidateExtDataIgnorable(ptr)) {
			this->InvalidateExtDataPointer(ptr, bRemoved);
		}
	}

protected:
	virtual void InvalidatePointer(void *ptr, bool bRemoved) {
	}

	virtual bool InvalidateExtDataIgnorable(void* const ptr) const {
		return true;
	}

	void InvalidateExtDataPointer(void *ptr, bool bRemoved) {
		for(const auto& i : this->Items) {
			i.second->InvalidatePointer(ptr, bRemoved);
		}
	}

public:
	value_type FindOrAllocate(key_type key) {
		if(key == nullptr) {
			Debug::Log("CTOR of %s attempted for a NULL pointer! WTF!\n", this->Name);
			return nullptr;
		}
		if(auto const ptr = this->Items.find(key)) {
			return ptr;
		}
		auto val = new extension_type(key);
		val->EnsureConstanted();
		return this->Items.insert(key, val);
	}

	value_type Find(const_key_type key) const {
		return this->Items.find(key);
	}

	void Remove(const_key_type key) {
		delete this->Items.remove(key);
	}

	void Clear() {
		if(this->Items.size()) {
			Debug::Log(Debug::Severity::Fatal, "Cleared %u items from %s.\n",
				this->Items.size(), this->Name);
			this->Items.clear();
		}
	}

	void LoadAllFromINI(CCINIClass *pINI) {
		for(const auto& i : this->Items) {
			i.second->LoadFromINI(pINI);
		}
	}

	void LoadFromINI(const_key_type key, CCINIClass *pINI) {
		if(auto const ptr = this->Items.find(key)) {
			ptr->LoadFromINI(pINI);
		}
	}

	void PrepareStream(key_type key, IStream *pStm) {
		this->SavingObject = key;
		this->SavingStream = pStm;
	}

	void SaveStatic() {
		if (this->SavingObject && this->SavingStream) {
			if (!this->Save(this->SavingObject, this->SavingStream)) {
				Debug::FatalErrorAndExit(Debug::ExitCode::SLFail, "[SaveStatic] Saving failed!\n");
			}
		}
		else {
			Debug::Log("[SaveStatic] Object or Stream not set for '%s': %p, %p\n",
				this->Name, this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

	void LoadStatic() {
		if (this->SavingObject && this->SavingStream) {

			if (!this->Load(this->SavingObject, this->SavingStream)) {
				Debug::FatalErrorAndExit(Debug::ExitCode::SLFail, "[LoadStatic] Loading failed!\n");
			}
		}
		else {
			Debug::Log("[LoadStatic] Object or Stream not set for '%s': %p, %p\n",
				this->Name, this->SavingObject, this->SavingStream);
		}

		this->SavingObject = nullptr;
		this->SavingStream = nullptr;
	}

	protected:
		// override this method to do type-specific stuff
		virtual bool Save(key_type key, IStream* pStm) {
			return this->SaveKey(key, pStm) != nullptr;
		}

		// override this method to do type-specific stuff
		virtual bool Load(key_type key, IStream* pStm) {
			return this->LoadKey(key, pStm) != nullptr;
		}

		value_type SaveKey(key_type key, IStream* pStm) {
			// this really shouldn't happen
			if (!key) {
				Debug::Log("[SaveKey] Attempted for a null pointer! WTF!\n");
				return nullptr;
			}

			// get the value data
			auto buffer = this->Find(key);
			if (!buffer) {
				Debug::Log("[SaveKey] Could not find value.\n");
				return nullptr;
			}

			// write the current pointer, the size of the block, and the canary
			PhobosStreamWriter::Process(pStm, extension_type::Canary);
			PhobosStreamWriter::Process(pStm, buffer);
			// save the data
			buffer->SaveToStream(pStm);

			// save the block
			PhobosStreamWriter::Process(pStm, sizeof(*buffer));
			if (PhobosStreamWriter::Process(pStm,*buffer)) {
				Debug::Log("[SaveKey] Failed to save data.\n");
				return nullptr;
			}

			// done
			return buffer;
		}

		value_type LoadKey(key_type key, IStream* pStm) {
			// this really shouldn't happen
			if (!key) {
				Debug::Log("[LoadKey] Attempted for a null pointer! WTF!\n");
				return nullptr;
			}

			// get the value data
			auto buffer = this->FindOrAllocate(key);
			if (!buffer) {
				Debug::Log("[LoadKey] Could not find or allocate value.\n");
				return nullptr;
			}

			size_t size;
			PhobosStreamReader::Process(pStm, size); // extension_type::Canary
			if (size == extension_type::Canary)
			{
				PhobosStreamReader::Process(pStm, buffer);
				buffer->LoadFromStream(pStm);
				PhobosStreamReader::Process(pStm, size);
				if (size == sizeof(*buffer))
					PhobosStreamReader::Process(pStm, *buffer);
				else
					Debug::FatalErrorAndExit(Debug::ExitCode::SLFail, 
						"[LoadKey] Size isn't correct as I excepted.");
			}
			else
				Debug::FatalErrorAndExit(Debug::ExitCode::SLFail, 
					"[LoadKey] %s's Canary isn't correct as I excepted.", this->Name);
			return nullptr;
		}
};
