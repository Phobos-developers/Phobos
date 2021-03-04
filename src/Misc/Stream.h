/*
 *  Every one loves templates.
 *  Make Save & Load easier and not too messy.
 */

#pragma once

#include <type_traits>
#include <ObjIdl.h>
#include <ArrayClasses.h>
#include <TechnoTypeClass.h>

class PhobosStreamReader
{
public:
    template<typename T>
    static bool Process(IStream* Stm, T&& value)
    {
        if (SUCCEEDED(Stm->Read(&value, sizeof T, 0)))
            return true;
        FatalExit(114514);
        return false;
    }

    template<typename T>
    static bool Process(IStream* Stm, T&& value, size_t nSize)
    {
        if (SUCCEEDED(Stm->Read(&value, nSize, 0)))
            return true;
        FatalExit(114514);
        return false;
    }

    template<typename T>
    static bool ProcessArray(IStream* Stm, const T*& pValues, size_t nCount)
    {
        if (!Process(Stm, nCount))
            return false;
        for (size_t ix = 0; ix < nCount; ++ix)
            if (!Process<T>(Stm, pValues[ix]))
                return false;
        return true;
    }

    template<typename T>
    static bool ProcessTechnoList(IStream* Stm, DynamicVectorClass<T>& array)
    {
        static_assert(std::is_base_of<TechnoTypeClass, T>::value,
            "array must be inherited from TechnoTypeClass!");
        array.Clear();
        size_t nCapacity;
        if (!Process(Stm, nCapacity))
            return false;
        array.SetCapacity(nCapacity);
        for (size_t ix = 0; ix < nCapacity; ++ix)
        {
            if (!Process(Stm, StreamBuffer, sizeof(AbstractTypeClass::ID)))
                return false;
            array.AddItem(T::Find(StreamBuffer));
        }
    }
private:
    static char StreamBuffer[0x400];
};

class PhobosStreamWriter
{
public:
    template<typename T>
    static bool Process(IStream* Stm, T&& value)
    {
        if (SUCCEEDED(Stm->Write(&value, sizeof T, 0)))
            return true;
        FatalExit(1919810);
        return false;
    }

    template<typename T>
    static bool Process(IStream* Stm, T&& value, size_t nSize)
    {
        if (SUCCEEDED(Stm->Write(&value, nSize, 0)))
            return true;
        FatalExit(1919810);
        return false;
    }

    template<typename T>
    static size_t ProcessArray(IStream* Stm, const T*& pValues)
    {
        size_t nCount;
        if (!Process(Stm, nCount))
            return 0;
        for (size_t ix = 0; ix < nCount; ++ix)
            if (!Process<T>(Stm, pValues[ix]))
                return false;
        return true;
    }

    template<typename T>
    static bool ProcessVector(IStream* Stm, DynamicVectorClass<T>& array)
    {
        static_assert(std::is_base_of<TechnoTypeClass, T>::value,
            "array must be inherited from TechnoTypeClass!");

        for (size_t ix = 0; ix < array.Count; ++ix)
        {
            if (!Process(Stm, &array.GetItem(ix)->ID, sizeof(AbstractTypeClass::ID)));
                return false;
        }
    }
private:
    
};