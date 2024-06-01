// Icegame.h
//

#pragma once

//
// dnGameLocal
//
class dnGameLocal : public idGameLocal {
public:
	virtual void			Init(void) override;
};

extern dnGameLocal gameLocal;