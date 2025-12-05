#include "blowfish.h"
#include <Utilities/Macro.h>
#include <Memory.h>

/**
 *  Remove the requirement for BLOWFISH.DLL (Blowfish encryption) and now
 *  handle the encryption/decryption internally.
 *
 *  @author: ZivDero
 */

/**
 *  Skip loading BLOWFISH.DLL
 */
DEFINE_JUMP(LJMP, 0x6BC33A, 0x6BC425);
DEFINE_JUMP(LJMP, 0x6BD6CA, 0x6BD71D);

/**
 *  Replace BlowfishEngine functions.
 */
DEFINE_FUNCTION_JUMP(LJMP, 0x437FC0, BlowfishEngine::~BlowfishEngine);
DEFINE_FUNCTION_JUMP(LJMP, 0x437FD0, BlowfishEngine::Submit_Key);
DEFINE_FUNCTION_JUMP(LJMP, 0x438000, BlowfishEngine::Encrypt);
DEFINE_FUNCTION_JUMP(LJMP, 0x438030, BlowfishEngine::Decrypt);

/**
 *  gamemd.exe contains a COM proxy for the actual BlowfishEngine class.
 *  As a consequence, we need to replace the memory allocations in BlowPipe and
 *  BlowStraw, because by default they only allocate 4 bytes.
 */
DEFINE_HOOK(0x4381DA, BlowPipe_Key_BlowfishEngine_Allocation, 0x0)
{
	BlowfishEngine* bf = GameCreate<BlowfishEngine>();
	R->EAX(bf);
	return 0x4381F3;
}

DEFINE_HOOK(0x43830A, BlowStraw_Key_BlowfishEngine_Allocation, 0x0)
{
	BlowfishEngine* bf = GameCreate<BlowfishEngine>();
	R->EAX(bf);
	return 0x438323;
}
