#include "Body.h"

#include <Ext/Rules/Body.h>

DEFINE_HOOK(0x480EA8, CellClass_DamageWall_AdjacentWallDamage, 0x7)
{
	enum{ SkipGameCode = 0x480EB4 };
	GET(CellClass*, pThis, EAX);
	pThis->DamageWall(RulesExt::Global()->AdjacentWallDamage);
	return SkipGameCode;
}

// CellClass::FindInfantrySubposition (0x481180) resolves the coordinate an infantry
// should occupy inside a cell. The per-cell sub-position offsets live in the table
// at 0x89E9F0, which holds five triples, and entry 0 is (0x80, 0x80, 0) - the exact
// cell centre.
//
// The engine only ever derives index 0 when the *requested* coordinate lies within
// 0x3C leptons of the centre (0x4811EC). Otherwise the derivation at
// 0x4811EC..0x481212 produces {2, 3, 4}: it sets bit 0 for X > 0x80 and bit 1 for
// Y > 0x80, then adds 1, which both shifts the pair into 2..4 and skips index 1 (the
// reserved "Top" slot, CellClass occupation bit 0x2). Index 1 is therefore
// unreachable and index 0 means "centre was requested".
//
// Either way index 0 can never survive the selection logic:
//   * 0x481316 `TEST EDI,EDI / JZ 0x48138A` sends it to a random roll over the
//     corner tables at 0x81CC84 / 0x81CC98, and those only contain indices 1..4, and
//   * the per-infantry scan loop at 0x48136A explicitly skips indices 0 and 1
//     (`TEST ECX,ECX / JZ`, `CMP ECX,1 / JZ`).
// Infantry therefore only ever stand on the three corner/lower sub-positions.
//
// The shared tail at 0x481437 indexes the offset table with EDI before it reuses the
// register, so entering it with EDI == 0 produces the centre entry. This hook adds
// that route for infantry that are deliberately walking to the centre
// (CellExt::InfantryCenterAllowed).
//
// The flag is tested *before* dispatching on EDI, and the centre is forced for every
// EDI when it is set. That ordering matters: InfantryClass::UpdatePosition (0x51B967)
// calls this function again on every frame of the walk, re-deriving the index from
// the infantry's live position, which is still off-centre during the approach and so
// yields 2/3/4. Diverting only EDI == 0 would let the vanilla scan snap the infantry
// back onto a corner each frame and it could never reach the centre at all.
//
// The hook replaces the whole index-selection sequence (TEST/JZ/CMP/JZ, 9 bytes) and
// returns an explicit address on every path, so no branch ever falls through the
// patched bytes and no flag state has to be reconstructed.
DEFINE_HOOK(0x481316, CellClass_FindInfantrySubposition_AllowCenter, 0x9)
{
	enum
	{
		// Vanilla EDI == 0: roll a random corner out of the 0x81CC98 table.
		RandomSubposition = 0x48138A,
		// Vanilla EDI == 1 (unreachable per the derivation above, kept for fidelity).
		SubpositionOne = 0x481356,
		// Vanilla EDI >= 2: scan the 0x81CC84 table for a free sub-position.
		ScanSubposition = 0x48131F,
		// Shared tail; consumes EDI as the offset-table index.
		ApplySubposition = 0x481437,
	};

	GET(CellClass*, pThis, ESI);

	auto const pExt = CellExt::TryFetch(pThis);
	if (pExt && pExt->InfantryCenterAllowed)
	{
		// Force the centre entry, whatever index the engine derived.
		R->EDI(0);
		return ApplySubposition;
	}

	// Not centring: reproduce the instructions this hook replaced.
	if (R->EDI() == 0)
		return RandomSubposition;

	return R->EDI() == 1 ? SubpositionOne : ScanSubposition;
}
