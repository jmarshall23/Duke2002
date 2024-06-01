#include "precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

ABSTRACT_DECLARATION(idClass, idWeaponBase)
END_CLASS

idWeaponBase::idWeaponBase() :
	currentState(nullptr),
	nextState(nullptr),
	blendFrames(0),
	WEAPON_ATTACK(false),
	WEAPON_RELOAD(false),
	WEAPON_NETRELOAD(false),
	WEAPON_NETENDRELOAD(false),
	WEAPON_RAISEWEAPON(false),
	WEAPON_LOWERWEAPON(false),
	WEAPON_START_FIRING(false) {
}

void idWeaponBase::EnterCinematic() {
	// Default implementation
}

void idWeaponBase::ExitCinematic() {
	// Default implementation
}

void idWeaponBase::NetCatchup() {
	ChangeState(&idWeaponBase::State_Idle, 0);
}

void idWeaponBase::WeaponStolen() {
	// Default implementation
}

void idWeaponBase::OwnerDied() {
	// Default implementation
}

void idWeaponBase::UpdateSkin() {
	//if (IsInvisible()) {
	//	SetSkin(GetKey("skin_invisible"));
	//}
	//else {
	//	SetSkin("");
	//}
}

idStr idWeaponBase::GetFireAnim() {
	return "fire";
}

void idWeaponBase::Update() {
	if (nextState) {
		currentState = nextState;
		nextState = nullptr;
	}
	if (currentState) {
		(this->*currentState)();
	}
}

void idWeaponBase::SetState(const char* name, int blendFrames) {
	idStr newStateName = name;

	if (newStateName == "Lower") {
		ChangeState(&idWeaponBase::State_Lower, blendFrames);
	}
	else if (newStateName == "Idle") {
		ChangeState(&idWeaponBase::State_Idle, blendFrames);
	}
	else if (newStateName == "Raise") {
		ChangeState(&idWeaponBase::State_Raise, blendFrames);
	}
	else if (newStateName == "Fire") {
		ChangeState(&idWeaponBase::State_Fire, blendFrames);
	}
	else if (newStateName == "Fire") {
		ChangeState(&idWeaponBase::State_Reload, blendFrames);
	}
	else if (newStateName == "EnterCinematic") {
		ChangeState(&idWeaponBase::EnterCinematic, blendFrames);
	}
	else if (newStateName == "ExitCinematic") {
		ChangeState(&idWeaponBase::ExitCinematic, blendFrames);
	}
	else {
		common->FatalError("idWeaponBase::SetState: Unknown state name!");
	}

	StateChangedEvent();
}

void idWeaponBase::ToggleFlashLight(void) {
	if (!WeaponHasFlashLight()) {
		return;
	}

	//flashLightOn = !flashLightOn;
	//owner->NativeEvent_Flashlight(flashLightOn);
}