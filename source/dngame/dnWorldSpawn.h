// dnWorldSpawn.h
//

#pragma once

class dnWorldspawn : public idWorldspawn {
public:
	CLASS_PROTOTYPE(dnWorldspawn);

	~dnWorldspawn();

	virtual void			Spawn(void);

private:	
	const iceDeclAtmosphere* defaultAtmosphere;
};