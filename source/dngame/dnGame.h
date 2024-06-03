// Icegame.h
//

#pragma once

#include "dnAnim_vertex.h"

//
// dnGameLocal
//
class dnGameLocal : public idGameLocal {
public:
	virtual void			Init(void) override;
};

extern dnGameLocal gameLocal;