#include <Helpers/Macro.h>
#include <stdint.h>

// Half of an RGB range in 16 bit
// (equivalent of (127, 127, 127))
#define HALF_RANGE_MASK 0x7BEFu

// Blending mask for 16-bit pixels
// https://medium.com/@luc.trudeau/fast-averaging-of-high-color-16-bit-pixels-cb4ac7fd1488

#define BLENDING_MASK 0xF7DEu

inline uint16_t  Blit50TranslucencyFix(uint16_t  dst, uint16_t  src)
{
	return (((src ^ dst) & BLENDING_MASK) >> 1) + (src & dst);
}

inline uint16_t  Blit75TranslucencyFix(uint16_t  dst, uint16_t  src)
{
	uint16_t  div = Blit50TranslucencyFix(dst, src);
	return (div >> 1 & HALF_RANGE_MASK) + (dst >> 1 & HALF_RANGE_MASK);
}

//same as 75, just reversed order of args
inline uint16_t  Blit25TranslucencyFix(uint16_t  dst, uint16_t  src)
{
	return Blit75TranslucencyFix(src, dst);
}

#undef HALF_RANGE_MASK
#undef BLENDING_MASK

// =============================
// container hooks

DEFINE_HOOK(492866, BlitTransLucent50_Fix, 0)
{
	GET(uint16_t , color, EAX);
	GET(uint16_t *, dest, EDI);

	*dest = Blit50TranslucencyFix(*dest, color);

	return 0x492878;
}

DEFINE_HOOK(492956, BlitTransLucent25_Fix, 0)
{
	GET(uint16_t , color, EAX);
	GET(uint16_t *, dest, ESI);

	*dest = Blit25TranslucencyFix(*dest, color);

	return 0x49296D;
}

DEFINE_HOOK(492776, BlitTransLucent75_Fix, 0)
{
	GET(uint16_t , color, EBP);
	GET(uint16_t *, dest, ESI);

	*dest = Blit75TranslucencyFix(*dest, color);

	return 0x49278D;
}
