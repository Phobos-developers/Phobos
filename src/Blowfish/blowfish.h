/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BLOWFISH.H
 *
 *  @author        Joe L. Bostic (see notes below)
 *
 *  @contributors  CCHyper
 *
 *  @brief         This implements the Blowfish algorithm.
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
#pragma once

#include <climits>


class BlowfishEngine
{
	public:
		BlowfishEngine();
		~BlowfishEngine();

		void Submit_Key(void const * key, int length);

		int Encrypt(void const * plaintext, int length, void * cyphertext);
		int Decrypt(void const * cyphertext, int length, void * plaintext);

		enum {
			DEF_KEY_LENGTH = 16,
			MIN_KEY_LENGTH = 4,
			MAX_KEY_LENGTH = 56
		};

	private:
		enum {
			ROUNDS = 16,
			BYTES_PER_BLOCK = 8
		};

		void Sub_Key_Encrypt(unsigned long & left, unsigned long & right);
		void Process_Block(void const * plaintext, void * cyphertext, unsigned long const * ptable);

	private:
		static unsigned long const P_Init[(int)ROUNDS+2];
		static unsigned long const S_Init[4][UCHAR_MAX+1];

	private:
		bool IsKeyed;

		unsigned long P_Encrypt[(int)ROUNDS+2];
		unsigned long P_Decrypt[(int)ROUNDS+2];

		unsigned long bf_S[4][UCHAR_MAX+1];
};
