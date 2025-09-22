// src/Misc/Hooks.RadBeam.cpp
#include <YRPP.h>
#include <Helpers/Macro.h>

#include <MapClass.h>
#include <ScenarioClass.h>
#include <HouseClass.h>
#include <CellClass.h>
#include <RadBeam.h>
#include <Utilities/Debug.h>

#define RADBEAM_FOW_DEBUG 1

// "either-endpoint visible" to match Laser feel; fail closed.
static __forceinline bool EitherEndVisible(const CoordStruct& a, const CoordStruct& b) {
    // No fog → always visible
    if(!(ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar)) {
        return true;
    }
    // Global reveal
    if(HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive) {
        return true;
    }
    // A endpoint
    if(auto* ca = MapClass::Instance.GetCellAt(CellClass::Coord2Cell(a))) {
        if(!ca->IsFogged()) { return true; }
    }
    // B endpoint
    if(auto* cb = MapClass::Instance.GetCellAt(CellClass::Coord2Cell(b))) {
        if(!cb->IsFogged()) { return true; }
    }
    return false;
}

/*
  RadBeam fog gating - DISABLED due to Ares conflicts

  Multiple approaches tried but all crash due to Ares interaction:
  1. Post-prologue hooks (0x6596ED/0x659666) - stack corruption
  2. Call-site hook (0x6592BB) - ECX doesn't contain RadBeam pointer

  RadBeam fog gating remains unimplemented until Ares compatibility resolved.

  Working fog gates: EBolt, LaserDraw, LaserTrail
*/

namespace RadBeamTemp
{
    bool FogHidden = false; // Flag to track if current beam should be hidden due to fog
}

#include <Windows.h>

static __forceinline bool CellVisible(const CellStruct& cs) {
    if(!(ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar)) return true;
    if(HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive) return true;
    if(auto* c = MapClass::Instance.GetCellAt(cs)) return !c->IsFogged();
    return false;
}

// Original draw (whatever Ares/others already point the CALL to)
static void (__thiscall* RadBeam_Draw_Orig)(RadBeam*) = nullptr;

// Called from the *patched CALL* (so we have real CALL/RET semantics).
static void __fastcall RadBeam_Draw_Gate(RadBeam* pThis, void* /*edx*/) {
#if RADBEAM_FOW_DEBUG
    static int dbg = 0;
    if(dbg++ < 8 && pThis) {
        Debug::Log("[RADBEAM_FOW] Gate pThis=%p from=(%d,%d,%d) to=(%d,%d,%d)\n",
            pThis,
            pThis->SourceLocation.X, pThis->SourceLocation.Y, pThis->SourceLocation.Z,
            pThis->TargetLocation.X, pThis->TargetLocation.Y, pThis->TargetLocation.Z);
    }
#endif
    if(!pThis) return;
    if(!(ScenarioClass::Instance && ScenarioClass::Instance->SpecialFlags.FogOfWar)) { RadBeam_Draw_Orig(pThis); return; }
    if(HouseClass::CurrentPlayer && HouseClass::CurrentPlayer->SpySatActive)        { RadBeam_Draw_Orig(pThis); return; }
    if(EitherEndVisible(pThis->SourceLocation, pThis->TargetLocation))               { RadBeam_Draw_Orig(pThis); return; }

#if RADBEAM_FOW_DEBUG
    if(dbg <= 8) Debug::Log("[RADBEAM_FOW] both fogged → skip draw\n");
#endif
    // both fogged → do nothing, return to caller (after CALL)
}

static void* ResolveCallTarget(uintptr_t callSite) {
    if(*reinterpret_cast<uint8_t*>(callSite) != 0xE8) return nullptr; // must be CALL rel32
    int32_t rel = *reinterpret_cast<int32_t*>(callSite + 1);
    return reinterpret_cast<void*>(callSite + 5 + rel);
}

static bool PatchCALL(uintptr_t callSite, void* newTarget) {
    DWORD oldProt;
    if(!VirtualProtect(reinterpret_cast<void*>(callSite), 5, PAGE_EXECUTE_READWRITE, &oldProt)) return false;
    *reinterpret_cast<uint8_t*>(callSite) = 0xE8;
    int32_t rel = static_cast<int32_t>(reinterpret_cast<uintptr_t>(newTarget) - (callSite + 5));
    *reinterpret_cast<int32_t*>(callSite + 1) = rel;
    VirtualProtect(reinterpret_cast<void*>(callSite), 5, oldProt, &oldProt);
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(callSite), 5);
    return true;
}

// Call this once during DLL init, after Ares has done its own patches.
void Install_RadBeamFogGate() {
    // Verified CALL site to RadBeam::Draw inside DrawAll:
    const uintptr_t site = 0x6592C6;   // next instruction is 0x6592CB

    // Must be CALL (0xE8). If a previous JMP hook sits here, bail.
    if(*reinterpret_cast<uint8_t*>(site) != 0xE8) {
#if RADBEAM_FOW_DEBUG
        Debug::Log("[RADBEAM_FOW] ERROR: 0x%08X is not CALL (byte=%02X)\n",
                   site, *reinterpret_cast<uint8_t*>(site));
#endif
        return;
    }

    // Capture original target (this may be Ares' wrapper — that's fine)
    void* target = ResolveCallTarget(site);
    RadBeam_Draw_Orig = reinterpret_cast<decltype(RadBeam_Draw_Orig)>(target);
#if RADBEAM_FOW_DEBUG
    Debug::Log("[RADBEAM_FOW] Captured original draw target = %p from site 0x%08X\n",
               RadBeam_Draw_Orig, site);
#endif
    if(!RadBeam_Draw_Orig) return;

    // Redirect that CALL to our gate
    PatchCALL(site, reinterpret_cast<void*>(&RadBeam_Draw_Gate));
#if RADBEAM_FOW_DEBUG
    Debug::Log("[RADBEAM_FOW] Patched CALL at 0x%08X to gate\n", site);
#endif
}

// Install the patch during initialization - use a different hook point
DEFINE_HOOK(0x685659, Scenario_ClearClasses_RadBeamInit, 0xA)
{
    Install_RadBeamFogGate();
    return 0;
}