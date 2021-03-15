/*
 *  Every one loves templates.
 *  Make Save & Load easier and not too messy.
 */

#pragma once

#include <type_traits>
#include <ObjIdl.h>
#include <ArrayClasses.h>
#include <SwizzleManagerClass.h>

class PhobosStreamReader
{
public:
    template<typename T>
    static bool Process(IStream* Stm, T& value)
    {
        return Process(Stm, value, sizeof T);
    }

    template<typename T>
    static bool Process(IStream* Stm, T& value, size_t nSize)
    {
        if (SUCCEEDED(Stm->Read(&value, nSize, 0)))
            return true;
        Debug::FatalErrorAndExit(Debug::ExitCode::SLFail, "[PhobosStreamReader] Failed to read value!\n");
        return false;
    }

    // Swizzle :
    // pass in the *address* of the pointer you want to have changed
    // Here_I_Am :
    // the original game objects all save their `this` pointer to the save stream
    // that way they know what ptr they used and call this function with that old ptr and `this` as the new ptr
    // -- from Ares

    // Pointers need extra process.
    template<typename T>
    static bool ProcessPointer(IStream* Stm, T& value, bool bRegisterForChange = false)
    {
        bool bResult = Process(Stm, value, sizeof(T*));
        if( bRegisterForChange )
            bResult &= SUCCEEDED(SwizzleManagerClass::Instance.Swizzle((void**)&value));
        
        if(!bResult)
            Debug::FatalErrorAndExit(Debug::ExitCode::SLFail, "[PhobosStreamReader] Failed to read pointer!\n");
        return bResult;
    }

    template<typename T>
    static T* ProcessObject(IStream* Stm, bool bRegisterForChange = false)
    {
        T* ptrOld = nullptr;
        if (Process(Stm, ptrOld, bRegisterForChange))
        {
            std::unique_ptr<T> ptrNew = std::make_unique<T>();
            if (Process(Stm, *ptrNew, bRegisterForChange))
            {
                SwizzleManagerClass::Instance.Here_I_Am(ptrOld, ptrNew.get());
                return ptrNew.release();
            }
        }
        return nullptr;
    }
};

class PhobosStreamWriter
{
public:
    template<typename T>
    static bool Process(IStream* Stm, T&& value)
    {
        return Process(Stm, value, sizeof (T));
    }

    template<typename T>
    static bool Process(IStream* Stm, T&& value, size_t nSize)
    {
        if (SUCCEEDED(Stm->Write(&value, nSize, 0)))
            return true;
        Debug::FatalErrorAndExit(Debug::ExitCode::SLFail, "[PhobosStreamReader] Failed to save value!\n");
        return false;
    }
    
    template<typename T>
    static bool ProcessObject(IStream* Stm, const T* pValue)
    {
        if (!Process(Stm, pValue))
            return false;
        if (pValue)
            return Process(Stm, *pValue);
    }
    
};