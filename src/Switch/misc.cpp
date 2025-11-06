
#include <switch.h>
#include <cstring>

namespace Switch {
	void getUserName(char *playername, int namelen) {
		Result rc = accountInitialize(AccountServiceType_Application);
		if (R_SUCCEEDED(rc)) {
			AccountUid userID = {0};

			rc = accountGetPreselectedUser(&userID);
			if (R_SUCCEEDED(rc)) {
				AccountProfile profile;
				AccountProfileBase profilebase = {0};

				rc = accountGetProfile(&profile, userID);
				if (R_SUCCEEDED(rc))
					rc = accountProfileGet(&profile, nullptr, &profilebase);

				if (R_SUCCEEDED(rc)) {
					strncpy(playername, profilebase.nickname, namelen);
					playername[namelen-1] = '\0';
				}

				accountProfileClose(&profile);
			}

			accountExit();
		}
	}

	const char *getUserLanguage() {
		static char buf[sizeof(u64) + 1] = {0};

		//Get system language.
		Result rc = setInitialize();
		if (R_SUCCEEDED(rc)) {
			u64 LanguageCode = 0;
			rc = setGetSystemLanguage(&LanguageCode);

			if (R_SUCCEEDED(rc)) {
				strncpy(buf, (char*)&LanguageCode, sizeof(u64));
				buf[sizeof(u64)] = '\0';
			}

			setExit();
		}

		return buf;
	}
}
