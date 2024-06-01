// Weapon_fists.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

CLASS_DECLARATION(idWeaponBase, idWeaponFists)
END_CLASS

// blend times
#define FISTS_IDLE_TO_LOWER		4
#define FISTS_IDLE_TO_PUNCH		0
#define FISTS_RAISE_TO_IDLE		4
#define FISTS_PUNCH_TO_IDLE		1

/*
=====================
idWeaponFists::idWeaponFists
=====================
*/
idWeaponFists::idWeaponFists() : side(0) {
}

/*
=====================
idWeaponFists::Init
=====================
*/
void idWeaponFists::Init(idWeapon* owner) {
	idWeaponBase::Init(owner);
	ChangeState(&idWeaponFists::State_Raise, 0);
}

/*
=====================
idWeaponFists::InitStates
=====================
*/
void idWeaponFists::InitStates() {
	// Initialize state transitions
}

/*
=====================
idWeaponFists::State_Idle
=====================
*/
void idWeaponFists::State_Idle() {
	owner->NativeEvent_WeaponReady();
	owner->NativeEvent_PlayCycle(ANIMCHANNEL_ALL, "idle");

	if (WEAPON_LOWERWEAPON) {
		ChangeState(&idWeaponFists::State_Lower, FISTS_IDLE_TO_LOWER);
		return;
	}
	if (WEAPON_ATTACK) {
		ChangeState(&idWeaponFists::State_Punch, FISTS_IDLE_TO_PUNCH);
		return;
	}
}

/*
=====================
idWeaponFists::State_Lower
=====================
*/
void idWeaponFists::State_Lower() {
	owner->NativeEvent_WeaponLowering();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "putaway");
	ChangeState(&idWeaponFists::State_Lower_WaitForAnim, 0);
}

/*
=====================
idWeaponFists::State_Lower_WaitForAnim
=====================
*/
void idWeaponFists::State_Lower_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, 0)) {
		owner->NativeEvent_WeaponHolstered();
		if (WEAPON_RAISEWEAPON) {
			ChangeState(&idWeaponFists::State_Raise, 0);
		}
	}
}

/*
=====================
idWeaponFists::State_Raise
=====================
*/
void idWeaponFists::State_Raise() {
	owner->NativeEvent_WeaponRising();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "raise");
	ChangeState(&idWeaponFists::State_Raise_WaitForAnim, 0);
}

/*
=====================
idWeaponFists::State_Raise_WaitForAnim
=====================
*/
void idWeaponFists::State_Raise_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, FISTS_RAISE_TO_IDLE)) {
		ChangeState(&idWeaponFists::State_Idle, FISTS_RAISE_TO_IDLE);
	}
}

/*
=====================
idWeaponFists::State_Punch
=====================
*/
void idWeaponFists::State_Punch() {
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, GetFireAnim());
	ChangeState(&idWeaponFists::State_Punch_WaitForAnim, 0);
}

/*
=====================
idWeaponFists::State_Punch_WaitForAnim
=====================
*/
void idWeaponFists::State_Punch_WaitForAnim() {
	// Assuming `sys.wait(0.1)` is equivalent to some kind of delay or waiting mechanism.
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, FISTS_PUNCH_TO_IDLE)) {
		side = !side;
		ChangeState(&idWeaponFists::State_Idle, FISTS_PUNCH_TO_IDLE);
	}
}

/*
=====================
idWeaponFists::ExitCinematic
=====================
*/
void idWeaponFists::ExitCinematic() {
	ChangeState(&idWeaponFists::State_Idle, 0);
}

/*
=====================
idWeaponFists::GetFireAnim
=====================
*/
idStr idWeaponFists::GetFireAnim() {
	return side ? "punch_left" : "punch_right";
}