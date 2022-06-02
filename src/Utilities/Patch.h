#pragma once

struct patch_decl;

class Patch {
public:
	static void Apply();
	static void Apply(const patch_decl* pItem);
};
