#include "ExceptionHandler.h"
#include "ExceptionHandler.Resource.h"

#include <Phobos.h>

#include <Unsorted.h>
#include <Surface.h>
#include <MouseClass.h>
#include <WWMouseClass.h>
#include <YRDDraw.h>

#include <Utilities/Debug.h>

namespace
{
	// The game state is untrustworthy after a crash, so the whole sequence
	// runs under a guard; a fault just leaves the cursor visible.
	void FreeMouseBody()
	{
		Game::StreamerThreadFlush();

		MouseClass::Instance.UpdateCursor(MouseCursorType::Default, false);

		if (WWMouseClass::Instance != nullptr)
			WWMouseClass::Instance->ReleaseMouse();

		ShowCursor(TRUE);

		// Black out the surfaces so the dialog is visible over fullscreen
		// DirectDraw instead of a frozen game frame.
		if (DSurface::Alternate != nullptr)
			DSurface::Alternate->Fill(0);
		if (DSurface::Composite != nullptr)
			DSurface::Composite->Fill(0);
		if (DSurface::Hidden != nullptr)
			DSurface::Hidden->Fill(0);
		if (DSurface::Temp != nullptr)
			DSurface::Temp->Fill(0);
		if (DSurface::Primary != nullptr)
			DSurface::Primary->Fill(0);
		if (DSurface::Sidebar != nullptr)
			DSurface::Sidebar->Fill(0);
		if (DSurface::Tile != nullptr)
			DSurface::Tile->Fill(0);

		// Restore the desktop display mode so the dialog isn't shown on a
		// mode-switched 16-bit screen. Deliberately without dropping the
		// cooperative level first: the engine's own Reset_Video_Mode calls
		// RestoreDisplayMode alone, and a second SetCooperativeLevel makes
		// ts-ddraw re-save its own WndProc as the game's, recursing to a
		// stack overflow on the next window message.
		if (DirectDrawWrap::lpDD != nullptr)
			DirectDrawWrap::lpDD->RestoreDisplayMode();

		ShowCursor(TRUE);
	}

	bool WriteFullDumpGuarded()
	{
		__try
		{
			EXCEPTION_POINTERS* pExs = (ExceptionHandler::SavedPointers.ContextRecord != nullptr)
				? &ExceptionHandler::SavedPointers : nullptr;

			return ExceptionHandler::WriteMinidump(pExs, ExceptionHandler::CrashedThreadId, true,
				ExceptionHandler::FullDumpPath, sizeof(ExceptionHandler::FullDumpPath));
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
	}

	INT_PTR CALLBACK ExceptionDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_EXCEPTION_QUIT:
				EndDialog(hDlg, IDC_EXCEPTION_QUIT);
				return TRUE;

			case IDC_EXCEPTION_DEBUG:
				EndDialog(hDlg, IDC_EXCEPTION_DEBUG);
				return TRUE;

			case IDC_EXCEPTION_FULLDUMP:
			{
				EnableWindow(GetDlgItem(hDlg, IDC_EXCEPTION_FULLDUMP), FALSE);

				HCURSOR waitCursor = LoadCursorA(nullptr, IDC_WAIT);
				SetCursor(waitCursor);

				const bool success = WriteFullDumpGuarded();

				SetCursor(LoadCursorA(nullptr, IDC_ARROW));

				SetDlgItemTextA(hDlg, IDC_EXCEPTION_FULLDUMP, success ? "Dump Saved" : "Dump Failed");
				if (success)
					SetDlgItemTextA(hDlg, IDC_EXCEPTION_GROUPBOX, ExceptionHandler::FullDumpPath);

				SetFocus(GetDlgItem(hDlg, IDC_EXCEPTION_QUIT));
				return TRUE;
			}

			default:
				return FALSE;
			}

		case WM_CLOSE:
			EndDialog(hDlg, IDC_EXCEPTION_QUIT);
			return TRUE;

		case WM_INITDIALOG:
		{
			// Render the report in a monospaced font so stack addresses and
			// register columns line up. The font handle is intentionally
			// leaked - the process terminates right after this dialog.
			HDC hdc = GetDC(hDlg);
			const int fontHeight = -MulDiv(9, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			ReleaseDC(hDlg, hdc);

			HFONT font = CreateFontA(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
				FIXED_PITCH | FF_MODERN, "Consolas");

			if (font != nullptr)
				SendDlgItemMessageA(hDlg, IDC_EXCEPTION_LOG, WM_SETFONT, reinterpret_cast<WPARAM>(font), MAKELPARAM(FALSE, 0));

			SetDlgItemTextA(hDlg, IDC_EXCEPTION_LOG, ExceptionHandler::ReportFinished
				? ExceptionHandler::ReportBuffer
				: "The crash report could not be generated. A minidump may still have been written to the debug folder.");

			ShowWindow(hDlg, SW_SHOWNORMAL);
			SetForegroundWindow(hDlg);

			// Focus the Quit button so Enter takes the safe action, and
			// return FALSE to tell the dialog manager we set focus ourselves.
			SetFocus(GetDlgItem(hDlg, IDC_EXCEPTION_QUIT));
			return FALSE;
		}

		default:
			return FALSE;
		}
	}
}

void ExceptionHandler::FreeMouse()
{
	__try
	{
		FreeMouseBody();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		ShowCursor(TRUE);
	}
}

INT_PTR ExceptionHandler::ShowDialog(HWND parent, int recursionCount)
{
	switch (recursionCount)
	{
	case 1:
		Debug::Log("Recursive exception detected!\n");
		MessageBoxA(nullptr, "Recursive exception detected!", "Error!", MB_OK | MB_ICONEXCLAMATION);
		return IDC_EXCEPTION_QUIT;

	case 2:
		return IDC_EXCEPTION_QUIT;

	case 3:
		ExitProcess(EXIT_FAILURE);

	default:
		break;
	}

	SetWindowTextA(Game::hWnd, "Fatal Error - Yuri's Revenge");

	HMODULE hModule = static_cast<HMODULE>(Phobos::hInstance);
	INT_PTR result = -1;

	HRSRC hResource = FindResourceA(hModule, MAKEINTRESOURCEA(IDD_EXCEPTION), RT_DIALOG);
	HGLOBAL hTemplate = hResource != nullptr ? LoadResource(hModule, hResource) : nullptr;

	if (hTemplate != nullptr)
	{
		// Activate the side-by-side manifest embedded in the DLL (resource
		// ID 2) so the dialog binds against comctl32 v6; without it the
		// dialog falls back to classic chrome, which is only cosmetic.
		char modulePath[MAX_PATH] = { };
		GetModuleFileNameA(hModule, modulePath, sizeof(modulePath));

		ACTCTXA actCtx = { };
		actCtx.cbSize = sizeof(actCtx);
		actCtx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
		actCtx.lpSource = modulePath;
		actCtx.lpResourceName = MAKEINTRESOURCEA(2); // ISOLATIONAWARE_MANIFEST_RESOURCE_ID

		HANDLE hActCtx = CreateActCtxA(&actCtx);
		ULONG_PTR cookie = 0;
		bool activated = false;
		if (hActCtx != INVALID_HANDLE_VALUE)
			activated = ActivateActCtx(hActCtx, &cookie) != FALSE;

		result = DialogBoxIndirectParamA(reinterpret_cast<HINSTANCE>(hModule),
			static_cast<LPCDLGTEMPLATEA>(LockResource(hTemplate)), parent, ExceptionDialogProc, 0);

		if (activated)
			DeactivateActCtx(0, cookie);
		if (hActCtx != INVALID_HANDLE_VALUE)
			ReleaseActCtx(hActCtx);
	}
	else
	{
		Debug::Log("Unable to find the exception dialog resource!\n");
	}

	if (result != IDC_EXCEPTION_QUIT && result != IDC_EXCEPTION_DEBUG)
	{
		// The dialog could not be created at all - fall back to a plain
		// message box so the user still learns where the report went.
		MessageBoxA(parent,
			"Yuri's Revenge has encountered a fatal error and is unable to continue.\n\n"
			"A crash report and a minidump have been saved to the game's \"debug\" folder. "
			"Please include them and your debug.log when reporting this crash.",
			"Fatal Error - Yuri's Revenge", MB_OK | MB_ICONERROR);

		result = IDC_EXCEPTION_QUIT;
	}

	return result;
}
