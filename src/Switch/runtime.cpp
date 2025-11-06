#include <switch.h>
#include <cstdio>
#include <SDL.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace {

	int s_nxlinkSock = -1;

	void nxlinkInit() {
		s_nxlinkSock = nxlinkStdio();
		if (s_nxlinkSock >= 0)
			printf("nxlink activated...\n");
	}

	void nxlinkExit() {
		if (s_nxlinkSock >= 0) {
			close(s_nxlinkSock);
			s_nxlinkSock = -1;
		}
	}

	void copyMapData() {
		const fs::path appMaps{"sdmc:/switch/DuneLegacy/maps"};
		const fs::path romMaps{"romfs:/data/maps"};

		try {
			if(fs::exists(appMaps))
				return;

	#if 0 // recursive approach does not work with romfs currently
			fs::create_directories(appMaps);
			fs::copy(romMaps, appMaps, fs::copy_options::recursive);
	#else
			auto copyFiles = [] (fs::path from, fs::path to) {
				fs::create_directories(to);
				std::ifstream src;
				std::ofstream dst;
				for (const auto & entry : fs::directory_iterator(from)) {
					src.open(entry.path(), std::ios::in | std::ios::binary);
					dst.open(to / entry.path().filename(), std::ios::out | std::ios::binary);

					dst << src.rdbuf();

					src.close();
					dst.close();
				}
			};

			copyFiles(romMaps / "singleplayer", appMaps / "singleplayer");
			copyFiles(romMaps / "multiplayer", appMaps / "multiplayer");
	#endif
		} catch (const fs::filesystem_error& e) {
			printf("Cannot copy map data: %s\n", e.what());
			return;
		}

		printf("Copied maps.\n");
	}
}

extern "C" {
	void userAppInit() {
		socketInitializeDefault(); // enable network play
		nxlinkInit(); // enable debug log
		romfsInit(); // to find engine data
		copyMapData();

		SDL_SetHint(SDL_HINT_GAMECONTROLLER_USE_BUTTON_LABELS, "1");
		SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1"); // use touch screen as mouse
	}

	void userAppExit() {
		romfsExit();
		nxlinkExit();
		socketExit();
	}
}
