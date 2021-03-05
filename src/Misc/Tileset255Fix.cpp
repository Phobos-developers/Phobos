
/*  Allow usage of TileSet of 255 and above without making NE-SW broken bridges unrepairable
   
    When TileSet number crosses 255 in theater INI files, the NE-SW broken bridges
    become non-repairable. The reason is that the NonMarbleMadnessTile array of size 256
    overflows when filled and affects the variables like BridgeTopRight1 and BridgeBottomLeft2
    that come after it. This patch removes the filling of the unused NonMarbleMadnessTile array
    and its references.
*/

#include <Helpers/Macro.h>

DEFINE_HOOK(545CE2, Tileset255Fix_RemoveNonMMArrayFill, 0)
{
    return 0x545CE9;
}

DEFINE_HOOK(546C23, Tileset255Fix_RefNonMMArray, 0)
{
    return 0x546C8B;
}
