#include <Helpers/Macro.h>

DEFINE_HOOK(0x527B0A, INIClass__Get_UUID, 0x8)
{
	GET(wchar_t*, buffer, ECX);

	if (buffer[0] != L'{') {

		if (_wcsicmp(buffer, L"Drive") == 0) {
			wcscpy(buffer, L"{4A582741-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if (_wcsicmp(buffer, L"Jumpjet") == 0) {
		   wcscpy(buffer, L"{92612C46-F71F-11d1-AC9F-006008055BB5}");
			return 0;
		}

		if (_wcsicmp(buffer, L"Hover") == 0) {
			wcscpy(buffer, L"{4A582742-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if (_wcsicmp(buffer, L"Rocket") == 0) {
			wcscpy(buffer, L"{B7B49766-E576-11d3-9BD9-00104B972FE8}");
			return 0;
		}

		if (_wcsicmp(buffer, L"Tunnel") == 0) {
			wcscpy(buffer, L"{4A582743-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if (_wcsicmp(buffer, L"Walk") == 0) {
			wcscpy(buffer, L"{4A582744-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if (_wcsicmp(buffer, L"DropPod") == 0) {
			wcscpy(buffer, L"{4A582745-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if (_wcsicmp(buffer, L"Fly") == 0) {
			wcscpy(buffer, L"{4A582746-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if (_wcsicmp(buffer, L"Teleport") == 0) {
			wcscpy(buffer, L"{4A582747-9839-11d1-B709-00A024DDAFD1}");
			return 0;
		}

		if (_wcsicmp(buffer, L"Mech") == 0) {
			wcscpy(buffer, L"{55D141B8-DB94-11d1-AC98-006008055BB5}");
			return 0;
		}

		if (_wcsicmp(buffer, L"Ship") == 0) {
			wcscpy(buffer, L"{2BEA74E1-7CCA-11d3-BE14-00104B62A16C}");
			return 0;
		}
	}

	return 0;
}
