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

#include "Iterator.h"

#include <MouseClass.h>
#include <FootClass.h>

#include "../Misc/Stream.h"

class INI_EX;

/**
 * More fancy templates!
 * This one is just a nicer-looking INI Parser... the fun starts with the next one
 */

template<typename T>
class Valueable {
protected:
	T Value{};
public:
	using value_type = T;
	using base_type = std::remove_pointer_t<T>;

	Valueable() = default;
	explicit Valueable(T value) noexcept(noexcept(T{ std::move(value) })) : Value(std::move(value)) {}
	Valueable(Valueable const& other) = default;
	Valueable(Valueable&& other) = default;

	Valueable& operator = (Valueable const& value) = default;
	Valueable& operator = (Valueable&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<T&, Val&&>::value>>
	Valueable& operator = (Val value) {
		this->Value = std::move(value);
		return *this;
	}

	operator const T& () const noexcept {
		return this->Get();
	}

	// only allow this when explict works, otherwise
	// the always-non-null pointer will be used in conditionals.
	//explicit operator T* () noexept {
	//	return this->GetEx();
	//}

	T operator -> () const {
		return this->Get();
	}

	T* operator & () noexcept {
		return this->GetEx();
	}

	bool operator ! () const {
		return this->Get() == 0;
	}

	const T& Get() const noexcept {
		return this->Value;
	}

	T* GetEx() noexcept {
		return &this->Value;
	}

	const T* GetEx() const noexcept {
		return &this->Value;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate = false);

	inline bool Load(IStream* Stm);

	inline bool Save(IStream* Stm) const;
};

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool operator == (const Valueable<T>& val, const T& other) {
	return val.Get() == other;
}

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool operator == (const T& other, const Valueable<T>& val) {
	return val.Get() == other;
}

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool operator != (const Valueable<T>& val, const T& other) {
	return !(val == other);
}

template <typename T, typename = std::enable_if_t<std::is_enum<T>::value>>
inline bool operator != (const T& other, const Valueable<T>& val) {
	return !(val == other);
}

// more fun
template<typename Lookuper>
class ValueableIdx : public Valueable<int> {
public:
	ValueableIdx() noexcept : Valueable<int>(-1) {}
	explicit ValueableIdx(int value) noexcept : Valueable<int>(value) {}
	ValueableIdx(ValueableIdx const& other) = default;
	ValueableIdx(ValueableIdx&& other) = default;

	ValueableIdx& operator = (ValueableIdx const& value) = default;
	ValueableIdx& operator = (ValueableIdx&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<int&, Val&&>::value>>
	ValueableIdx& operator = (Val value) {
		this->Value = std::move(value);
		return *this;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);
};

template<typename T>
class Nullable : public Valueable<T> {
protected:
	bool HasValue{ false };
public:
	Nullable() = default;
	explicit Nullable(T value) noexcept(noexcept(Valueable<T>{std::move(value)})) : Valueable<T>(std::move(value)), HasValue(true) {}
	Nullable(Nullable const& other) = default;
	Nullable(Nullable&& other) = default;

	Nullable& operator = (Nullable const& value) = default;
	Nullable& operator = (Nullable&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<T&, Val&&>::value>>
	Nullable& operator = (Val value) {
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	bool isset() const noexcept {
		return this->HasValue;
	}

	using Valueable<T>::Get;

	T Get(const T& default) const {
		return this->isset() ? this->Get() : default;
	}

	using Valueable<T>::GetEx;

	T* GetEx(T* default) & noexcept {
		return this->isset() ? this->GetEx() : default;
	}

	const T* GetEx(const T* default) const noexcept {
		return this->isset() ? this->GetEx() : default;
	}

	void Reset() {
		this->Value = T();
		this->HasValue = false;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey, bool Allocate = false);

	inline bool Load(IStream* Stm);

	inline bool Save(IStream* Stm) const;
};

template<typename Lookuper>
class NullableIdx : public Nullable<int> {
public:
	NullableIdx() noexcept : Nullable<int>(-1) { this->HasValue = false; }
	explicit NullableIdx(int value) noexcept : Nullable<int>(value) {}
	NullableIdx(NullableIdx const& other) = default;
	NullableIdx(NullableIdx&& other) = default;

	NullableIdx& operator = (NullableIdx const& value) = default;
	NullableIdx& operator = (NullableIdx&& value) = default;

	template <typename Val, typename = std::enable_if_t<std::is_assignable<int&, Val&&>::value>>
	NullableIdx& operator = (Val value) {
		this->Value = std::move(value);
		this->HasValue = true;
		return *this;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);
};

/*
 * This template is for something that varies depending on a unit's Veterancy Level
 * Promotable<int> PilotChance; // class def
 * PilotChance(); // ctor init-list
 * PilotChance->Read(..., "Base%s"); // load from ini
 * PilotChance->Get(Unit); // usage
 *
 * Use %s format specifier, exactly once. If pSingleFlag is null, pBaseFlag will
 * be used. For the single flag name, a trailing dot (after replacing %s) will
 * be removed. I.e. "Test.%s" will be converted to "Test".
 */
template<typename T>
class Promotable {
public:
	T Rookie{};
	T Veteran{};
	T Elite{};

	Promotable() = default;
	explicit Promotable(T const& all) noexcept(noexcept(T{ all })) : Rookie(all), Veteran(all), Elite(all) {}

	void SetAll(const T& val) {
		this->Elite = this->Veteran = this->Rookie = val;
	}

	inline void Read(INI_EX& parser, const char* pSection, const char* pBaseFlag, const char* pSingleFlag = nullptr);

	const T* GetEx(TechnoClass* pTechno) const noexcept {
		return &this->Get(pTechno);
	}

	const T& Get(TechnoClass* pTechno) const noexcept {
		auto const rank = pTechno->Veterancy.GetRemainingLevel();
		if (rank == Rank::Elite) {
			return this->Elite;
		}
		if (rank == Rank::Veteran) {
			return this->Veteran;
		}
		return this->Rookie;
	}

	inline bool Load(IStream* Stm);

	inline bool Save(IStream* Stm) const;
};


template<class T>
class ValueableVector : public std::vector<T> {
public:
	using value_type = T;
	using base_type = std::remove_pointer_t<T>;

	ValueableVector() noexcept = default;

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);

	bool Contains(const T& other) const {
		return std::find(this->begin(), this->end(), other) != this->end();
	}

	int IndexOf(const T& other) const {
		auto it = std::find(this->begin(), this->end(), other);
		if (it != this->end()) {
			return it - this->begin();
		}
		return -1;
	}

	Iterator<T> GetElements() const noexcept {
		return Iterator<T>(*this);
	}

	inline bool Load(IStream* Stm);

	inline bool Save(IStream* Stm) const;
};

template<class T>
class NullableVector : public ValueableVector<T> {
protected:
	bool hasValue{ false };
public:
	NullableVector() noexcept = default;

	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);

	bool HasValue() const noexcept {
		return this->hasValue;
	}

	using ValueableVector<T>::GetElements;

	Iterator<T> GetElements(Iterator<T> default) const noexcept {
		if (!this->hasValue) {
			return default;
		}

		return this->GetElements();
	}

	inline bool Load(IStream* Stm);

	inline bool Save(IStream* Stm) const;
};

template<typename Lookuper>
class ValueableIdxVector : public ValueableVector<int> {
public:
	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);
};

template<typename Lookuper>
class NullableIdxVector : public NullableVector<int> {
public:
	inline void Read(INI_EX& parser, const char* pSection, const char* pKey);
};
