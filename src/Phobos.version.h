// SPDX-License-Identifier: GPL-3.0-or-later
// Phobos - Ares-compatible C&C Red Alert 2: Yuri's Revenge engine extension
// Copyright (C) 2020 Phobos developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#ifndef VERSION_H
#define VERSION_H

#define _WSTR(x) _WSTR_(x)
#define _WSTR_(x) L ## #x
#define _STR(x) _STR_(x)
#define _STR_(x) #x

#pragma region Release build version numbering

// Indicates project maturity and completeness
#define VERSION_MAJOR 0

// Indicates major changes and significant additions, like new logics
#define VERSION_MINOR 3

// Indicates minor changes, like vanilla bugfixes, unhardcodings or hacks
#define VERSION_REVISION 0

// Indicates Phobos-related bugfixes only
#define VERSION_PATCH 1

#pragma endregion

// Build number. Incremented on each released build.
#define BUILD_NUMBER 48

// Nightly defines GIT_COMMIT and GIT_BRANCH in GH Actions

#ifdef IS_RELEASE_VER // Release build metadata
	#define SAVEGAME_ID ((VERSION_MAJOR << 24) | (VERSION_MINOR << 16) | (VERSION_REVISION << 8) | VERSION_PATCH)
	#define FILE_DESCRIPTION "Phobos, Ares-compatible YR engine extension"
	#define FILE_VERSION_STR _STR(VERSION_MAJOR) "." _STR(VERSION_MINOR) "." _STR(VERSION_REVISION) "." _STR(VERSION_PATCH)
	#define FILE_VERSION VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_PATCH
	#define PRODUCT_VERSION "Release Build " FILE_VERSION_STR
#elif defined(GIT_COMMIT) // Nightly devbuild metadata
	#define STR_GIT_COMMIT _STR(GIT_COMMIT)
	#define STR_GIT_BRANCH _STR(GIT_BRANCH)

	#define SAVEGAME_ID ((BUILD_NUMBER << 24) | (BUILD_NUMBER << 12) | (BUILD_NUMBER))
	#define FILE_DESCRIPTION "Unstable nightly devbuild of Phobos engine extension"
	#define FILE_VERSION_STR "Commit " STR_GIT_COMMIT
	#define FILE_VERSION 0
	#define PRODUCT_VERSION "Nightly Build " STR_GIT_COMMIT " @ " STR_GIT_BRANCH
#else // Regular devbuild metadata
	#define SAVEGAME_ID ((BUILD_NUMBER << 24) | (BUILD_NUMBER << 12) | (BUILD_NUMBER))
	#define FILE_DESCRIPTION "Development build of Phobos engine extension"
	#define FILE_VERSION_STR "Build #" _STR(BUILD_NUMBER)
	#define FILE_VERSION 0,0,0,BUILD_NUMBER
	#define PRODUCT_VERSION "Development Build #" _STR(BUILD_NUMBER)
#endif

#endif // VERSION_H
