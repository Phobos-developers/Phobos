/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BLOWPIPE.H
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
#pragma once

#include "Pipes.h"
#include "blowfish.h"


class BlowPipe : public Pipe
{
	public:
		typedef enum CryptControl {
			ENCRYPT,
			DECRYPT
		} CryptControl;

	public:
		BlowPipe(CryptControl control) : BF(nullptr), Counter(0), Control(control) {}
		virtual ~BlowPipe() { delete BF; BF = nullptr; }

		virtual int Flush()override;
		virtual int Put(const void * source, int slen) override;

		void Key(const void * key, int length);

	protected:
		BlowfishEngine * BF;

	private:
		char Buffer[8];
		int Counter;
		CryptControl Control;

	private:
		BlowPipe(BlowPipe &) = delete;
		BlowPipe & operator = (const BlowPipe &) = delete;
};
