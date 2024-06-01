// Game_locascreen.cpp
//

#include "precompiled.h"

#include "Game_local.h"

/*
===================
idGameLocal::LoadLoadingGui
===================
*/
void idGameLocal::LoadLoadingGui(const char* mapName) {
	// load / program a gui to stay up on the screen while loading
	idStr stripped = mapName;
	stripped.StripFileExtension();
	stripped.StripPath();

	char guiMap[MAX_STRING_CHARS];
	strncpy(guiMap, va("guis/map/%s.gui", stripped.c_str()), MAX_STRING_CHARS);

	if (uiManager->CheckGui(guiMap)) {
		guiLoading = uiManager->FindGui(guiMap, true, false, true);
	}
	else {
		guiLoading = uiManager->FindGui("guis/map/loading.gui", true, false, true);
	}
	guiLoading->SetStateFloat("map_loading", 0.0f);
}

/*
===================
idGameLocal::LoadScreenUpdate
===================
*/
void idGameLocal::LoadScreenUpdate(int time, int bytesNeededForMapLoad) {
	if (guiLoading && bytesNeededForMapLoad) {
		float n = fileSystem->GetReadCount();
		float pct = (n / bytesNeededForMapLoad);
		// pct = idMath::ClampFloat( 0.0f, 100.0f, pct );
		guiLoading->SetStateFloat("map_loading", pct);
		guiLoading->StateChanged(time);
	}
}

/*
===================
idGameLocal::LoadScreenRedraw
===================
*/
void idGameLocal::LoadScreenRedraw(int time) {
	if (guiLoading) {
		guiLoading->Redraw(time);
	}
}