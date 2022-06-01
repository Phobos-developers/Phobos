#pragma once

#include <memory>
#include <type_traits>
#include <vector>
#include <set>
#include <map>

struct IStream;

class PhobosByteStream
{
public:
	using data_t = unsigned char;

protected:
	std::vector<data_t> Data;
	size_t CurrentOffset;

public:
	PhobosByteStream(size_t Reserve = 0x1000);
	~PhobosByteStream();

	size_t Size() const
	{
		return this->Data.size();
	}

	size_t Offset() const
	{
		return this->CurrentOffset;
	}

	/**
	* reads {Length} bytes from {pStm} into its storage
	*/
	bool ReadFromStream(IStream* pStm, const size_t Length);

	/**
	* writes all internal storage to {pStm}
	*/
	bool WriteToStream(IStream* pStm) const;

	/**
	* reads the next block of bytes from {pStm} into its storage,
	* the block size is prepended to the block
	*/
	size_t ReadBlockFromStream(IStream* pStm);

	/**
	* writes all internal storage to {pStm}, prefixed with its length
	*/
	bool WriteBlockToStream(IStream* pStm) const;


	// primitive save/load - should not be specialized

	/**
	* if it has {Size} bytes left, assigns the first {Size} unread bytes to {Value}
	* moves the internal position forward
	*/
	bool Read(data_t* Value, size_t Size);

	/**
	* ensures there are at least {Size} bytes left in the internal storage, and assigns {Value} casted to byte to that buffer
	* moves the internal position forward
	*/
	void Write(const data_t* Value, size_t Size);


	/**
	* attempts to read the data from internal storage into {Value}
	*/
	template<typename T>
	bool Load(T& Value)
	{
		// get address regardless of overloaded & operator
		auto Bytes = &reinterpret_cast<data_t&>(Value);
		return this->Read(Bytes, sizeof(T));
	}

	/**
	* writes the data from {Value} into internal storage
	*/
	template<typename T>
	void Save(const T& Value)
	{
		// get address regardless of overloaded & operator
		auto Bytes = &reinterpret_cast<const data_t&>(Value);
		this->Write(Bytes, sizeof(T));
	};
};

class PhobosStreamWorkerBase
{
public:
	explicit PhobosStreamWorkerBase(PhobosByteStream& Stream) :
		stream(&Stream),
		success(true)
	{ }

	PhobosStreamWorkerBase(const PhobosStreamWorkerBase&) = delete;

	PhobosStreamWorkerBase& operator = (const PhobosStreamWorkerBase&) = delete;

	bool Success() const
	{
		return this->success;
	}

protected:
	// set to false_type or true_type to disable or enable debugging checks
	using stream_debugging_t = std::false_type;

	bool IsValid(std::true_type) const
	{
		return this->success;
	}

	bool IsValid(std::false_type) const
	{
		return true;
	}

	PhobosByteStream* stream;
	bool success;
};

class PhobosStreamReader : public PhobosStreamWorkerBase
{
public:
	explicit PhobosStreamReader(PhobosByteStream& Stream) : PhobosStreamWorkerBase(Stream) { }
	PhobosStreamReader(const PhobosStreamReader&) = delete;

	PhobosStreamReader& operator = (const PhobosStreamReader&) = delete;

	template <typename _Ty>
	PhobosStreamReader& Process(std::set<_Ty>& s, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			s.clear();
			size_t size;
			this->Process(size, RegisterForChange);
			for (size_t i = 0; i < size; i++)
			{
				_Ty obj;
				this->Process(obj, RegisterForChange);
				s.emplace(obj);
			}
		}
		return *this;
	}

	template <typename _Kty, typename _Ty>
	PhobosStreamReader& Process(std::map<_Kty, _Ty>& m, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			m.clear();
			size_t size;
			this->Process(size, RegisterForChange);
			for (size_t i = 0; i < size; i++)
			{
				_Kty key;
				_Ty value;
				this->Process(key, RegisterForChange);
				this->Process(value, RegisterForChange);
				m.emplace(key, value);
			}
		}
		return *this;
	}

	template <typename _Kty, typename _Ty>
	PhobosStreamReader& Process(std::multimap<_Kty, _Ty>& m, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			m.clear();
			size_t size;
			this->Process(size, RegisterForChange);
			for (size_t i = 0; i < size; i++)
			{
				_Kty key;
				_Ty value;
				this->Process(key, RegisterForChange);
				this->Process(value, RegisterForChange);
				m.emplace(key, value);
			}
		}
		return *this;
	}

	template <typename _Ty1, typename _Ty2>
	PhobosStreamReader& Process(std::pair<_Ty1, _Ty2>& p, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			this->Process(p.first, RegisterForChange);
			this->Process(p.second, RegisterForChange);
		}
		return *this;
	}

	template <typename T>
	PhobosStreamReader& Process(T& value, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
			this->success &= Savegame::ReadPhobosStream(*this, value, RegisterForChange);
		return *this;
	}

	// helpers

	bool ExpectEndOfBlock() const
	{
		if (!this->Success() || this->stream->Size() != this->stream->Offset())
		{
			this->EmitExpectEndOfBlockWarning(stream_debugging_t());
			return false;
		}

		return true;
	}

	template <typename T>
	bool Load(T& buffer)
	{
		if (!this->stream->Load(buffer))
		{
			this->EmitLoadWarning(sizeof(T), stream_debugging_t());
			this->success = false;
			return false;
		}
		return true;
	}

	bool Read(PhobosByteStream::data_t* Value, size_t Size)
	{
		if (!this->stream->Read(Value, Size))
		{
			this->EmitLoadWarning(Size, stream_debugging_t());
			this->success = false;
			return false;
		}
		return true;
	}

	bool Expect(unsigned int value)
	{
		unsigned int buffer = 0;
		if (this->Load(buffer))
		{
			if (buffer == value)
				return true;

			this->EmitExpectWarning(buffer, value, stream_debugging_t());
		}
		return false;
	}

	bool RegisterChange(void* newPtr);

private:
	void EmitExpectEndOfBlockWarning(std::true_type) const;
	void EmitExpectEndOfBlockWarning(std::false_type) const { }

	void EmitLoadWarning(size_t size, std::true_type) const;
	void EmitLoadWarning(size_t size, std::false_type) const { }

	void EmitExpectWarning(unsigned int found, unsigned int expect, std::true_type) const;
	void EmitExpectWarning(unsigned int found, unsigned int expect, std::false_type) const { }

	void EmitSwizzleWarning(long id, void* pointer, std::true_type) const;
	void EmitSwizzleWarning(long id, void* pointer, std::false_type) const { }
};

class PhobosStreamWriter : public PhobosStreamWorkerBase
{
public:
	explicit PhobosStreamWriter(PhobosByteStream& Stream) : PhobosStreamWorkerBase(Stream) { }
	PhobosStreamWriter(const PhobosStreamWriter&) = delete;

	PhobosStreamWriter& operator = (const PhobosStreamWriter&) = delete;

	template <typename _Ty>
	PhobosStreamWriter& Process(std::set<_Ty>& s, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			size_t size = s.size();
			this->Process(size, RegisterForChange);
			for (auto& obj : s)
			{
				this->Process(obj, RegisterForChange);
			}
		}
		return *this;
	}

	template <typename _Kty, typename _Ty>
	PhobosStreamWriter& Process(std::map<_Kty, _Ty>& m, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			size_t size = m.size();
			this->Process(size, RegisterForChange);
			for (std::pair<const _Kty, _Ty>& p : m)
			{
				_Kty key = p.first;
				_Ty value = p.second;
				this->Process(key, RegisterForChange);
				this->Process(value, RegisterForChange);
			}
		}
		return *this;
	}

	template <typename _Kty, typename _Ty>
	PhobosStreamWriter& Process(std::multimap<_Kty, _Ty>& m, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			size_t size = m.size();
			this->Process(size, RegisterForChange);
			for (std::pair<const _Kty, _Ty>& p : m)
			{
				_Kty key = p.first;
				_Ty value = p.second;
				this->Process(key, RegisterForChange);
				this->Process(value, RegisterForChange);
			}
		}
		return *this;
	}

	template <typename _Ty1, typename _Ty2>
	PhobosStreamWriter& Process(std::pair<_Ty1, _Ty2>& p, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
		{
			this->Process(p.first, RegisterForChange);
			this->Process(p.second, RegisterForChange);
		}
		return *this;
	}

	template <typename T>
	PhobosStreamWriter& Process(T& value, bool RegisterForChange = true)
	{
		if (this->IsValid(stream_debugging_t()))
			this->success &= Savegame::WritePhobosStream(*this, value);

		return *this;
	}

	// helpers

	template <typename T>
	void Save(const T& buffer)
	{
		this->stream->Save(buffer);
	}

	void Write(const PhobosByteStream::data_t* Value, size_t Size)
	{
		this->stream->Write(Value, Size);
	}

	bool Expect(unsigned int value)
	{
		this->Save(value);
		return true;
	}

	bool RegisterChange(const void* oldPtr)
	{
		this->Save(oldPtr);
		return true;
	}
};
