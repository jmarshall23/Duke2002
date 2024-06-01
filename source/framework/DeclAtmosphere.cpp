// DeclAtmosphere.cpp
//

#include "precompiled.h"

/*
========================
iceDeclAtmosphere::iceDeclAtmosphere
========================
*/
iceDeclAtmosphere::iceDeclAtmosphere()
	: fogColor(0, 0, 0), fogDensity(0.0f), ambientLightColor(0, 0, 0) {
}

/*
========================
iceDeclAtmosphere::~iceDeclAtmosphere
========================
*/
iceDeclAtmosphere::~iceDeclAtmosphere() {
	FreeData();
}

/*
=================
iceDeclAtmosphere::Parse
=================
*/
bool iceDeclAtmosphere::Parse(const char* text, const int textLength) {
	idLexer src;
	idToken token;

	src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
	src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");

	while (src.ReadToken(&token)) {
		if (token == "}") {
			break;
		}
		else if (token == "fogColor") {
			fogColor.x = src.ParseFloat();
			fogColor.y = src.ParseFloat();
			fogColor.z = src.ParseFloat();
		}
		else if (token == "fogDensity") {
			fogDensity = src.ParseFloat();
		} else if (token == "fogStart") {
            fogStart = src.ParseFloat();
		}
		else if (token == "fogEnd") {
			fogEnd = src.ParseFloat();
		} 
		else if (token == "ambientLightColor") {
			ambientLightColor.x = src.ParseFloat();
			ambientLightColor.y = src.ParseFloat();
			ambientLightColor.z = src.ParseFloat();
		}
		else {
			src.Warning("Unknown token: %s", token.c_str());
			return false;
		}
	}

	return true;
}

/*
=================
iceDeclAtmosphere::FreeData
=================
*/
void iceDeclAtmosphere::FreeData() {
	fogColor.Zero();
	fogDensity = 0.0f;
	ambientLightColor.Zero();
}

/*
=================
iceDeclAtmosphere::GetFogColor
=================
*/
const idVec3& iceDeclAtmosphere::GetFogColor() const {
	return fogColor;
}

/*
=================
iceDeclAtmosphere::GetFogDensity
=================
*/
float iceDeclAtmosphere::GetFogDensity() const {
	return fogDensity;
}

/*
=================
iceDeclAtmosphere::GetFogStart
=================
*/
float iceDeclAtmosphere::GetFogStart() const {
	return fogStart;
}

/*
=================
iceDeclAtmosphere::GetFogEnd
=================
*/
float iceDeclAtmosphere::GetFogEnd() const {
	return fogEnd;
}

/*
=================
iceDeclAtmosphere::GetAmbientLightColor
=================
*/
const idVec3& iceDeclAtmosphere::GetAmbientLightColor() const {
	return ambientLightColor;
}