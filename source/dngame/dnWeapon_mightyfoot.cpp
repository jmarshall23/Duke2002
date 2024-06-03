// Weapon_fists.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "../game/Game_local.h"

CLASS_DECLARATION(idWeaponBase, dnWeaponMightyFoot)
END_CLASS

// blend times
#define FISTS_IDLE_TO_LOWER		4
#define FISTS_IDLE_TO_PUNCH		0
#define FISTS_RAISE_TO_IDLE		4
#define FISTS_PUNCH_TO_IDLE		1

/*
=====================
dnWeaponMightyFoot::dnWeaponMightyFoot
=====================
*/
dnWeaponMightyFoot::dnWeaponMightyFoot() : side(0) {
}

/*
=====================
dnWeaponMightyFoot::Init
=====================
*/
void dnWeaponMightyFoot::Init(idWeapon* owner) {
	idWeaponBase::Init(owner);
	ChangeState(&dnWeaponMightyFoot::State_Raise, 0);
}

/*
=====================
dnWeaponMightyFoot::InitStates
=====================
*/
void dnWeaponMightyFoot::InitStates() {
	// Initialize state transitions
}

/*
=====================
dnWeaponMightyFoot::State_Idle
=====================
*/
void dnWeaponMightyFoot::State_Idle() {
	owner->NativeEvent_WeaponReady();
	owner->NativeEvent_PlayCycle(ANIMCHANNEL_ALL, "idle");

	if (WEAPON_LOWERWEAPON) {
		ChangeState(&dnWeaponMightyFoot::State_Lower, FISTS_IDLE_TO_LOWER);
		return;
	}
	if (WEAPON_ATTACK) {
		ChangeState(&dnWeaponMightyFoot::State_Punch, FISTS_IDLE_TO_PUNCH);
		return;
	}
}

/*
=====================
dnWeaponMightyFoot::State_Lower
=====================
*/
void dnWeaponMightyFoot::State_Lower() {
	owner->NativeEvent_WeaponLowering();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "putaway");
	ChangeState(&dnWeaponMightyFoot::State_Lower_WaitForAnim, 0);
}

/*
=====================
dnWeaponMightyFoot::State_Lower_WaitForAnim
=====================
*/
void dnWeaponMightyFoot::State_Lower_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, 0)) {
		owner->NativeEvent_WeaponHolstered();
		if (WEAPON_RAISEWEAPON) {
			ChangeState(&dnWeaponMightyFoot::State_Raise, 0);
		}
	}
}

/*
=====================
dnWeaponMightyFoot::State_Raise
=====================
*/
void dnWeaponMightyFoot::State_Raise() {
	owner->NativeEvent_WeaponRising();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "raise");
	ChangeState(&dnWeaponMightyFoot::State_Raise_WaitForAnim, 0);
}

/*
=====================
dnWeaponMightyFoot::State_Raise_WaitForAnim
=====================
*/
void dnWeaponMightyFoot::State_Raise_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, FISTS_RAISE_TO_IDLE)) {
		ChangeState(&dnWeaponMightyFoot::State_Idle, FISTS_RAISE_TO_IDLE);
	}
}

/*
=====================
dnWeaponMightyFoot::State_Punch
=====================
*/
void dnWeaponMightyFoot::State_Punch() {
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, GetFireAnim());
	ChangeState(&dnWeaponMightyFoot::State_Punch_WaitForAnim, 0);
}

/*
=====================
dnWeaponMightyFoot::State_Punch_WaitForAnim
=====================
*/
void dnWeaponMightyFoot::State_Punch_WaitForAnim() {
	// Assuming `sys.wait(0.1)` is equivalent to some kind of delay or waiting mechanism.
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, FISTS_PUNCH_TO_IDLE)) {
		side = !side;
		ChangeState(&dnWeaponMightyFoot::State_Idle, FISTS_PUNCH_TO_IDLE);
	}
}

/*
=====================
dnWeaponMightyFoot::ExitCinematic
=====================
*/
void dnWeaponMightyFoot::ExitCinematic() {
	ChangeState(&dnWeaponMightyFoot::State_Idle, 0);
}

/*
=====================
dnWeaponMightyFoot::GetFireAnim
=====================
*/
idStr dnWeaponMightyFoot::GetFireAnim() {
	return "fire";
}