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

#include <ArrayClasses.h>
#include <vector>

template<typename T>
class Iterator
{
private:
	const T* items { nullptr };
	size_t count { 0 };
public:
	Iterator() = default;
	Iterator(const T* first, size_t count) : items(first), count(count) { }
	Iterator(const std::vector<T>& vec) : items(vec.data()), count(vec.size()) { }
	Iterator(const VectorClass<T>& vec) : items(vec.Items), count(static_cast<size_t>(vec.Capacity)) { }
	Iterator(const DynamicVectorClass<T>& vec) : items(vec.Items), count(static_cast<size_t>(vec.Count)) { }

	T at(size_t index) const
	{
		return this->items[index];
	}

	size_t size() const
	{
		return this->count;
	}

	const T* begin() const
	{
		return this->items;
	}

	const T* end() const
	{
		if (!this->valid())
		{
			return nullptr;
		}

		return &this->items[count];
	}

	bool valid() const
	{
		return items != nullptr;
	}

	bool empty() const
	{
		return !this->valid() || !this->count;
	}

	bool contains(T other) const
	{
		return std::find(this->begin(), this->end(), other) != this->end();
	}

	operator bool() const
	{
		return !this->empty();
	}

	bool operator !() const
	{
		return this->empty();
	}

	const T& operator [](size_t index) const
	{
		return this->items[index];
	}

	template<typename Out, typename = std::enable_if_t<std::is_assignable<Out&, T>::value>>
	operator Iterator<Out>() const
	{
		// note: this does only work if pointer-to-derived equals pointer-to-base.
		// if derived has virtual methods and base hasn't, this will just break.
		return Iterator<Out>(reinterpret_cast<const Out*>(this->items), this->count);
	}
};

template <typename T>
Iterator<T> make_iterator_single(const T& value)
{
	return Iterator<T>(&value, 1);
}

template <typename T, size_t Size>
Iterator<T> make_iterator(const T(&arr)[Size])
{
	return Iterator<T>(arr, Size);
}

template <typename T>
Iterator<T> make_iterator(const T* ptr, size_t size)
{
	return Iterator<T>(ptr, size);
}

// vector classes
template <typename T>
Iterator<T> make_iterator(const VectorClass<T>& value)
{
	return Iterator<T>(value);
}

template <typename T>
Iterator<T> make_iterator(const DynamicVectorClass<T>& value)
{
	return Iterator<T>(value);
}

template <typename T>
Iterator<T> make_iterator(const std::vector<T>& value)
{
	return Iterator<T>(value);
}

// iterator does not keep temporary alive, thus rvalues are forbidden.
// use the otherwise wierd const&& to not catch any lvalues
template <typename T>
void make_iterator_single(const T&&) = delete;

template <typename T>
void make_iterator(const T&&) = delete;
