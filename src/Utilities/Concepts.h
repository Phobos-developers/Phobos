/*
 *  C++ 20 standard introduces this, might be useful. 
 *  However, IntelliSense do not recognize them... for now, so do modules.
 *  When it's possible, I might try to make the codes fit with C++ 20 then.
 * 
 *  So this file is for further use.
 *  Author : secsome
 */

#pragma once

namespace Phobos_CXX20
{
    // This is how a concept defines
    template<typename T>
    concept Incrementable = requires(T x) { x++; ++x; };

    // Another example
    template <typename T>
    concept HasSize = requires (T x) {
        {x.size()} -> std::convertible_to<std::size_t>;
    };
}
