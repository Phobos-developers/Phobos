#pragma region Ares Copyrights
/*
 *Copyright (c) 2008+, All Ares Contributors
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are met:
 *1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *3. All advertising materials mentioning features or use of this software
 *   must display the following acknowledgement:
 *   This product includes software developed by the Ares Contributors.
 *4. Neither the name of Ares nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY ITS CONTRIBUTORS ''AS IS'' AND ANY
 *EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE ARES CONTRIBUTORS BE LIABLE FOR ANY
 *DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma endregion

#pragma once

#include <Theater.h>
#include <CCINIClass.h>
#include <GeneralStructures.h>
#include <StringTable.h>
#include <Helpers/String.h>
#include <PCX.h>
#include <Utilities/INIParser.h>
#include <Utilities/GeneralUtils.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

#include <Phobos.h>
#include <Phobos.CRT.h>

class PhobosStreamReader;
class PhobosStreamWriter;
#include "Enum.h"

class ConvertClass;

template <typename T>
using UniqueGamePtr = std::unique_ptr<T, GameDeleter>;

class ArmorType
{
public:
	ArmorType() = default;

	template <typename T>
	ArmorType(T value)
	{
		this->value = (int)std::move(value);
	}

	template <typename T>
	ArmorType& operator = (T value)
	{
		this->value = (int)std::move(value);
		return *this;
	}

	operator Armor() const { return  (Armor)this->value; }
	operator int() const { return this->value; }

private:
	int value { 0 };
};

struct Leptons
{
	Leptons() = default;
	explicit Leptons(int value) noexcept : value(value) { }

	operator int() const
	{
		return this->value;
	}

	int value { 0 };
};

class CustomPalette
{
public:
	enum class PaletteMode : unsigned int
	{
		Default = 0,
		Temperate = 1
	};

	PaletteMode Mode { PaletteMode::Default };
	UniqueGamePtr<ConvertClass> Convert { nullptr };
	UniqueGamePtr<BytePalette> Palette { nullptr };

	CustomPalette() = default;
	explicit CustomPalette(PaletteMode mode) noexcept : Mode(mode) { };

	ConvertClass* GetConvert() const
	{
		return this->Convert.get();
	}

	ConvertClass* GetOrDefaultConvert(ConvertClass* pDefault) const
	{
		return this->Convert.get() ? this->Convert.get() : pDefault;
	}

	bool LoadFromINI(
		CCINIClass* pINI, const char* pSection, const char* pKey,
		const char* pDefault = "");

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	void Clear();
	void CreateConvert();
};

// a poor man's map with contiguous storage
template <typename TKey, typename TValue>
class PhobosMap
{
public:
	TValue& operator[] (const TKey& key)
	{
		if (auto pValue = this->find(key))
		{
			return *pValue;
		}
		return this->insert_unchecked(key, TValue());
	}

	TValue* find(const TKey& key)
	{
		auto pValue = static_cast<const PhobosMap*>(this)->find(key);
		return const_cast<TValue*>(pValue);
	}

	const TValue* find(const TKey& key) const
	{
		auto it = this->get_iterator(key);
		if (it != this->values.end())
		{
			return &it->second;
		}
		return nullptr;
	}

	TValue get_or_default(const TKey& key) const
	{
		if (auto pValue = this->find(key))
		{
			return *pValue;
		}
		return TValue();
	}

	TValue get_or_default(const TKey& key, TValue def) const
	{
		if (auto pValue = this->find(key))
		{
			return *pValue;
		}
		return def;
	}

	bool erase(const TKey& key)
	{
		auto it = this->get_iterator(key);
		if (it != this->values.end())
		{
			this->values.erase(it);
			return true;
		}
		return false;
	}

	bool contains(const TKey& key) const
	{
		return this->get_iterator(key) != this->values.end();
	}

	bool insert(const TKey& key, TValue value)
	{
		if (!this->find(key))
		{
			this->insert_unchecked(key, std::move(value));
			return true;
		}
		return false;
	}

	size_t size() const
	{
		return this->values.size();
	}

	bool empty() const
	{
		return this->values.empty();
	}

	void clear()
	{
		this->values.clear();
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		this->clear();

		size_t size = 0;
		auto ret = Stm.Load(size);

		if (ret && size)
		{
			this->values.resize(size);
			for (size_t i = 0; i < size; ++i)
			{
				if (!Savegame::ReadPhobosStream(Stm, this->values[i].first, RegisterForChange)
					|| !Savegame::ReadPhobosStream(Stm, this->values[i].second, RegisterForChange))
				{
					return false;
				}
			}
		}

		return ret;
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		Stm.Save(this->values.size());

		for (const auto& item : this->values)
		{
			Savegame::WritePhobosStream(Stm, item.first);
			Savegame::WritePhobosStream(Stm, item.second);
		}

		return true;
	}

private:
	using container_t = std::vector<std::pair<TKey, TValue>>;

	typename container_t::const_iterator get_iterator(const TKey& key) const
	{
		return std::find_if(this->values.begin(), this->values.end(), [&](const container_t::value_type& item)
 {
	 return item.first == key;
			});
	}

	TValue& insert_unchecked(const TKey& key, TValue value)
	{
		this->values.emplace_back(key, std::move(value));
		return this->values.back().second;
	}

	container_t values;
};

// pcx filename storage with optional automatic loading
class PhobosPCXFile
{
	static const size_t Capacity = 0x20;
public:
	explicit PhobosPCXFile(bool autoResolve = true) : filename(), resolve(autoResolve), checked(false), exists(false)
	{ }

	PhobosPCXFile(const char* pFilename, bool autoResolve = true) : PhobosPCXFile(autoResolve)
	{
		*this = pFilename;
	}

	PhobosPCXFile& operator = (const char* pFilename);

	const FixedString<Capacity>::data_type& GetFilename() const
	{
		return this->filename.data();
	}

	BSurface* GetSurface(BytePalette* pPalette = nullptr) const;

	bool Exists() const;

	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "");

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	bool Save(PhobosStreamWriter& Stm) const;

private:
	FixedString<Capacity> filename;
	bool resolve;
	mutable bool checked;
	mutable bool exists;
};

// provides storage for a csf label with automatic lookup.
class CSFText
{
public:
	CSFText() noexcept { }
	explicit CSFText(nullptr_t) noexcept { }

	explicit CSFText(const char* label) noexcept
	{
		*this = label;
	}

	CSFText& operator = (CSFText const& rhs) = default;

	const CSFText& operator = (const char* label);

	operator const wchar_t* () const
	{
		return this->Text;
	}

	bool empty() const
	{
		return !this->Text || !*this->Text;
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange);

	bool save(PhobosStreamWriter& Stm) const;

	FixedString<0x20> Label;
	const wchar_t* Text { nullptr };
};

// fixed string with read method
template <size_t Capacity>
class PhobosFixedString : public FixedString<Capacity>
{
public:
	PhobosFixedString() = default;
	explicit PhobosFixedString(nullptr_t) noexcept { };
	explicit PhobosFixedString(const char* value) noexcept : FixedString<Capacity>(value) { }

	using FixedString<Capacity>::operator=;

	// It's not obvious, but pDefault = "" means that by default initial string will not be changed
	bool Read(INIClass* pINI, const char* pSection, const char* pKey, const char* pDefault = "")
	{
		if (pINI->ReadString(pSection, pKey, pDefault, Phobos::readBuffer, FixedString<Capacity>::Size))
		{
			if (!INIClass::IsBlank(Phobos::readBuffer))
			{
				*this = Phobos::readBuffer;
			}
			else
			{
				*this = nullptr;
			}
		}
		return Phobos::readBuffer[0] != 0;
	}
};

// owns a resource. not copyable, but movable.
template <typename T, typename Deleter, T Default = T()>
struct Handle
{
	constexpr Handle() noexcept = default;

	constexpr explicit Handle(T value) noexcept
		: Value(value)
	{ }

	Handle(const Handle&) = delete;

	constexpr Handle(Handle&& other) noexcept
		: Value(other.release())
	{ }

	~Handle() noexcept
	{
		if (this->Value != Default)
		{
			Deleter {}(this->Value);
		}
	}

	Handle& operator = (const Handle&) = delete;

	Handle& operator = (Handle&& other) noexcept
	{
		this->reset(other.release());
		return *this;
	}

	constexpr explicit operator bool() const noexcept
	{
		return this->Value != Default;
	}

	constexpr operator T () const noexcept
	{
		return this->Value;
	}

	constexpr T get() const noexcept
	{
		return this->Value;
	}

	T release() noexcept
	{
		return std::exchange(this->Value, Default);
	}

	void reset(T value) noexcept
	{
		Handle(this->Value);
		this->Value = value;
	}

	void clear() noexcept
	{
		Handle(std::move(*this));
	}

	bool load(PhobosStreamReader& Stm, bool RegisterForChange)
	{
		return Savegame::ReadPhobosStream(Stm, this->Value, RegisterForChange);
	}

	bool save(PhobosStreamWriter& Stm) const
	{
		return Savegame::WritePhobosStream(Stm, this->Value);
	}

private:
	T Value { Default };
};

class TranslucencyLevel
{
public:
	constexpr TranslucencyLevel() noexcept = default;

	TranslucencyLevel(int nInt)
	{
		*this = nInt;
	}

	TranslucencyLevel& operator = (int nInt)
	{
		switch (nInt)
		{
		default:
		case 0:
			this->value = BlitterFlags::None;
			break;
		case 25:
			this->value = BlitterFlags::TransLucent25;
			break;
		case 50:
			this->value = BlitterFlags::TransLucent50;
			break;
		case 75:
			this->value = BlitterFlags::TransLucent75;
			break;
		}

		return *this;
	}

	operator BlitterFlags()
	{
		return this->value;
	}

	BlitterFlags GetBlitterFlags()
	{
		return *this;
	}

	bool Read(INI_EX& parser, const char* pSection, const char* pKey);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	bool Save(PhobosStreamWriter& Stm) const;

private:
	BlitterFlags value { BlitterFlags::None };
};

class TheaterSpecificSHP
{
public:
	constexpr TheaterSpecificSHP() noexcept = default;

	TheaterSpecificSHP(SHPStruct* pSHP)
	{
		*this = pSHP;
	}

	TheaterSpecificSHP& operator = (SHPStruct* pSHP)
	{
		this->value = pSHP;
	}

	operator SHPStruct* ()
	{
		return this->value;
	}

	SHPStruct* GetSHP()
	{
		return *this;
	}

	bool Read(INI_EX& parser, const char* pSection, const char* pKey);

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	bool Save(PhobosStreamWriter& Stm) const;
private:
	SHPStruct* value { nullptr };
};
