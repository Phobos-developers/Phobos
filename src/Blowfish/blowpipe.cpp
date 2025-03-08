/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BLOWPIPE.CPP
 *
 *  @author        Joe L. Bostic (see notes below)
 *
 *  @contributors  CCHyper
 *
 *  @brief         Blowfish driven pipe.
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
#include "blowpipe.h"
#include <cassert>


int BlowPipe::Flush()
{
	int total = 0;
	if (Counter > 0 && BF != nullptr) {
		total += Pipe::Put(Buffer, Counter);
	}
	Counter = 0;
	total += Pipe::Flush();
	return total;
}


int BlowPipe::Put(const void * source, int slen)
{
	if (source == nullptr || slen < 1) {
		return Pipe::Put(source, slen);
	}

	if (BF == nullptr) {
		return Pipe::Put(source, slen);
	}

	int total = 0;

	if (Counter) {
		int sublen = ((int)sizeof(Buffer)-Counter < slen) ? (sizeof(Buffer)-Counter) : slen;
		std::memmove(&Buffer[Counter], source, sublen);
		Counter += sublen;
		source = ((char *)source) + sublen;
		slen -= sublen;

		if (Counter == sizeof(Buffer)) {
			if (Control == DECRYPT) {
				BF->Decrypt(Buffer, sizeof(Buffer), Buffer);
			} else {
				BF->Encrypt(Buffer, sizeof(Buffer), Buffer);
			}
			total += Pipe::Put(Buffer, sizeof(Buffer));
			Counter = 0;
		}
	}

	while (slen >= sizeof(Buffer)) {
		if (Control == DECRYPT) {
			BF->Decrypt(source, sizeof(Buffer), Buffer);
		} else {
			BF->Encrypt(source, sizeof(Buffer), Buffer);
		}
		total += Pipe::Put(Buffer, sizeof(Buffer));
		source = ((char *)source) + sizeof(Buffer);
		slen -= sizeof(Buffer);
	}

	if (slen > 0) {
		std::memmove(Buffer, source, slen);
		Counter = slen;
	}

	return total;
}


void BlowPipe::Key(const void * key, int length)
{
	if (BF == nullptr) {
		BF = GameCreate<BlowfishEngine>();
	}

	assert(BF != nullptr);

	if (BF != nullptr) {
		BF->Submit_Key(key, length);
	}
}
