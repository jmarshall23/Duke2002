// DeclAtmosphere.h
//

#pragma once

class iceDeclAtmosphere : public idDecl {
public:
	iceDeclAtmosphere();
	~iceDeclAtmosphere() override;

	bool Parse(const char* text, const int textLength) override;
	void FreeData() override;

	virtual const idVec3& GetFogColor() const;
	virtual float GetFogDensity() const;
	virtual float GetFogStart() const;
	virtual float GetFogEnd() const;
	virtual const idVec3& GetAmbientLightColor() const;

private:
	idVec3 fogColor;
	float fogStart;
	float fogEnd;
	float fogDensity;
	idVec3 ambientLightColor;
};