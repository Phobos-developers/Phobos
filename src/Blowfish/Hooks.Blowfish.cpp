#include <Utilities/Macro.h>
#include "blowfish.h"
#include "blowpipe.h"
#include "blowstraw.h"
#include "Utilities/Debug.h"

/**
 *  Remove the requirement for BLOWFISH.DLL (Blowfish encryption) and now
 *  handle the encryption/decryption internally.
 *
 *  @author: ZivDero
 */
class FakeBlowfishEngine
{
public:
	BlowfishEngine* CTOR_Proxy() { return new (reinterpret_cast<BlowfishEngine*>(this)) BlowfishEngine; }
};

DEFINE_JUMP(LJMP, 0x6BC33A, 0x6BC425);
DEFINE_JUMP(LJMP, 0x6BD6CA, 0x6BD71D);

DEFINE_FUNCTION_JUMP(LJMP, 0x438210, BlowStraw::Get);
DEFINE_FUNCTION_JUMP(LJMP, 0x438300, BlowStraw::Key);
DEFINE_FUNCTION_JUMP(LJMP, 0x438060, BlowPipe::Flush);
DEFINE_FUNCTION_JUMP(LJMP, 0x4380A0, BlowPipe::Put);
DEFINE_FUNCTION_JUMP(LJMP, 0x4381D0, BlowPipe::Key);

DEFINE_FUNCTION_JUMP(LJMP, 0x437F50, FakeBlowfishEngine::CTOR_Proxy);
DEFINE_FUNCTION_JUMP(LJMP, 0x437FC0, BlowfishEngine::~BlowfishEngine);
DEFINE_FUNCTION_JUMP(LJMP, 0x437FD0, BlowfishEngine::Submit_Key);
DEFINE_FUNCTION_JUMP(LJMP, 0x438000, BlowfishEngine::Encrypt);
DEFINE_FUNCTION_JUMP(LJMP, 0x438030, BlowfishEngine::Decrypt);
