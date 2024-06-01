// dnWorldSpawn.cpp
//

#include "precompiled.h"
#include "dnGame_local.h"

CLASS_DECLARATION(idWorldspawn, dnWorldspawn)
END_CLASS

/*
================
dnWorldspawn::~dnWorldspawn
================
*/
dnWorldspawn::~dnWorldspawn() {

}

/*
================
dnWorldspawn::Spawn
================
*/
void dnWorldspawn::Spawn(void) {
	// Load in the atmosphere defnitions for this level.
	defaultAtmosphere = declManager->FindAtmosphere(spawnArgs.GetString("atmosphere", "doom/default"), false);

	// Set the atmosphere properties for this world. 
	iceWorldAtmosphere_t atmosphere;
	atmosphere.fogColor = defaultAtmosphere->GetFogColor();
	atmosphere.fogDensity = defaultAtmosphere->GetFogDensity();
	atmosphere.ambientLightColor = defaultAtmosphere->GetAmbientLightColor();
	atmosphere.fogStart = defaultAtmosphere->GetFogStart();
	atmosphere.fogEnd = defaultAtmosphere->GetFogEnd();
	gameRenderWorld->SetAtmosphere(atmosphere);
}
