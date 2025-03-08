/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BLOWSTRAW.CPP
 *
 *  @author        Joe L. Bostic (see notes below)
 *
 *  @contributors  CCHyper
 *
 *  @brief         Blowfish driven straw.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 * 
 *  @note          This file contains heavily modified code from the source code
 *                 released by Electronic Arts for the C&C Remastered Collection
 *                 under the GPL3 license. Source:
 *                 https://github.com/ElectronicArts/CnC_Remastered_Collection
 *
 ******************************************************************************/
#include "blowstraw.h"
#include <cassert>


int BlowStraw::Get(void * source, int slen)
{
	if (source == nullptr || slen <= 0) {
		return 0;
	}

	if (BF == nullptr) {
		return Straw::Get(source, slen);
	}

	int total = 0;

	while (slen > 0) {

		if (Counter > 0) {
			int sublen = (slen < Counter) ? slen : Counter;
			std::memmove(source, &Buffer[sizeof(Buffer)-Counter], sublen);
			Counter -= sublen;
			source = ((char *)source) + sublen;
			slen -= sublen;
			total += sublen;
		}
		if (slen == 0) break;

		int incount = Straw::Get(Buffer, sizeof(Buffer));
		if (incount == 0) break;

		if (incount == sizeof(Buffer)) {
			if (Control == DECRYPT) {
				BF->Decrypt(Buffer, incount, Buffer);
			} else {
				BF->Encrypt(Buffer, incount, Buffer);
			}
		} else {
			std::memmove(&Buffer[sizeof(Buffer)-incount], Buffer, incount);
		}
		Counter = incount;
	}

	return total;
}


void BlowStraw::Key(void const * key, int length)
{
	if (BF == nullptr) {
		BF = GameCreate<BlowfishEngine>();
	}

	assert(BF != nullptr);

	if (BF != nullptr) {
		BF->Submit_Key(key, length);
	}
}
