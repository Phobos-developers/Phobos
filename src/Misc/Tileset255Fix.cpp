
/*  Allow usage of TileSet of 255 and above without making NE-SW broken bridges unrepairable

    When TileSet number crosses 255 in theater INI files (temperat/snow etc.), the NE-SW broken
    bridges become non-repairable. The variable values of BridgeTopRight1 and BridgeBottomLeft2
    that are parsed into, gets corrupted. To fix it, this patch allocates new addresses for those variables.
*/

#include <Helpers/Macro.h>

int BridgeTopRight_1 = -1;
int BridgeBottomLeft_2 = -1;

DEFINE_HOOK(545BBA, Tileset255Fix_AssignmentA, 0)
{
    BridgeTopRight_1 = R->EAX();
    return 0x545BBF;
}

DEFINE_HOOK(56A2E9, Tileset255Fix_ReferenceA1, 0)
{
    if ((signed)R->EBX() == BridgeTopRight_1)
        return 0x56A2F9;
    else
        return 0x56A2F1;
}

DEFINE_HOOK(56A600, Tileset255Fix_ReferenceA2, 0)
{
    if ((signed)R->EAX() == BridgeTopRight_1)
        return 0x56A610;
    else
        return 0x56A608;
}

DEFINE_HOOK(56AAC0, Tileset255Fix_ReferenceA3, 0)
{
    if ((signed)R->EAX() == BridgeTopRight_1)
        return 0x56AAD0;
    else
        return 0x56AAC8;
}

DEFINE_HOOK(56F83B, Tileset255Fix_ReferenceA4, 0)
{
    R->EDX(BridgeTopRight_1);
    return 0x56F841;
}

DEFINE_HOOK(56FD44, Tileset255Fix_ReferenceA5, 0)
{
    R->ECX(BridgeTopRight_1);
    return 0x56FD4A;
}

DEFINE_HOOK(57048C, Tileset255Fix_ReferenceA6, 0)
{
    if ((signed)R->EAX() == BridgeTopRight_1)
        return 0x57049C;
    else
        return 0x570494;
}

DEFINE_HOOK(571315, Tileset255Fix_ReferenceA7, 0)
{
    if ((signed)R->EAX() == BridgeTopRight_1)
        return 0x571325;
    else
        return 0x57131D;
}

DEFINE_HOOK(572D2B, Tileset255Fix_ReferenceA8, 0)
{
    R->EDX(BridgeTopRight_1);
    return 0x572D31;
}

DEFINE_HOOK(573234, Tileset255Fix_ReferenceA9, 0)
{
    R->ECX(BridgeTopRight_1);
    return 0x57323A;
}

DEFINE_HOOK(57398A, Tileset255Fix_ReferenceA10, 0)
{
    if ((signed)R->EAX() == BridgeTopRight_1)
        return 0x57399A;
    else
        return 0x573992;
}

DEFINE_HOOK(5746C1, Tileset255Fix_ReferenceA11, 0)
{
    R->ESI(BridgeTopRight_1);
    return 0x5746C7;
}

DEFINE_HOOK(576A2D, Tileset255Fix_ReferenceA12, 0)
{
    if ((signed)R->EAX() == BridgeTopRight_1)
        return 0x576A3D;
    else
        return 0x576A35;
}

DEFINE_HOOK(545C05, Tileset255Fix_AssignmentB, 0)
{
    BridgeBottomLeft_2 = R->EAX();
    return 0x545C0A;
}

DEFINE_HOOK(5691AB, Tileset255Fix_ReferenceB1, 0)
{
    if ((signed)R->ESI() != BridgeBottomLeft_2)
        return 0x5691C0;
    else
        return 0x5691B3;
}

DEFINE_HOOK(569ACB, Tileset255Fix_ReferenceB2, 0)
{
    if ((signed)R->ESI() != BridgeBottomLeft_2)
        return 0x569AE0;
    else
        return 0x569AD3;
}

DEFINE_HOOK(56F738, Tileset255Fix_ReferenceB3, 0)
{
    if ((signed)R->EAX() == BridgeBottomLeft_2)
        return 0x56F783;
    else
        return 0x56F740;
}

DEFINE_HOOK(56F985, Tileset255Fix_ReferenceB4, 0)
{
    if ((signed)R->EAX() == BridgeBottomLeft_2)
        return 0x56FC5B;
    else
        return 0x56F991;
}

DEFINE_HOOK(570C3A, Tileset255Fix_ReferenceB5, 0)
{
    if ((signed)R->EAX() != BridgeBottomLeft_2)
        return 0x570C4B;
    else
        return 0x570C42;
}

DEFINE_HOOK(572C28, Tileset255Fix_ReferenceB6, 0)
{
    if ((signed)R->EAX() == BridgeBottomLeft_2)
        return 0x572C73;
    else
        return 0x572C30;
}

DEFINE_HOOK(572E75, Tileset255Fix_ReferenceB7, 0)
{
    if ((signed)R->EAX() == BridgeBottomLeft_2)
        return 0x57314B;
    else
        return 0x572E81;
}

DEFINE_HOOK(574670, Tileset255Fix_ReferenceB8, 0)
{
    if ((signed)R->ECX() != BridgeBottomLeft_2)
        return 0x574687;
    else
        return 0x574678;
}

DEFINE_HOOK(57635A, Tileset255Fix_ReferenceB9, 0)
{
    if ((signed)R->EAX() != BridgeBottomLeft_2)
        return 0x57636B;
    else
        return 0x576362;
}

