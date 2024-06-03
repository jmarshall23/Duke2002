// dnWeaponPistol.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "../game/Game_local.h"

CLASS_DECLARATION(idWeaponBase, dnWeaponPistol)
END_CLASS

#define PISTOL_FIRERATE         0.4f
#define PISTOL_LOWAMMO          4
#define PISTOL_NUMPROJECTILES   1

// blend times
#define PISTOL_IDLE_TO_LOWER    2
#define PISTOL_IDLE_TO_FIRE     1
#define PISTOL_IDLE_TO_RELOAD   3
#define PISTOL_RAISE_TO_IDLE    3
#define PISTOL_FIRE_TO_IDLE     4
#define PISTOL_RELOAD_TO_IDLE   4

/*
=====================
dnWeaponPistol::dnWeaponPistol
=====================
*/
dnWeaponPistol::dnWeaponPistol() :
	next_attack(0), spread(0.0f) {
}

/*
=====================
dnWeaponPistol::Init
=====================
*/
void dnWeaponPistol::Init(idWeapon* owner) {
	idWeaponBase::Init(owner);

	next_attack = 0;
	spread = owner->CurrentWeaponDef()->dict.GetFloat("spread");
	ChangeState(&dnWeaponPistol::State_Raise, 0);
}

/*
=====================
dnWeaponPistol::InitStates
=====================
*/
void dnWeaponPistol::InitStates() {
	// Initialize state transitions
}

/*
=====================
dnWeaponPistol::State_Idle
=====================
*/
void dnWeaponPistol::State_Idle() {
	float currentTime;
	float ammoClip;
	float clip_size;

	clip_size = owner->ClipSize();

	owner->NativeEvent_WeaponReady();
	if (!owner->AmmoInClip()) {
		owner->NativeEvent_PlayCycle(ANIMCHANNEL_ALL, "idle_empty");
	}
	else {
		owner->NativeEvent_PlayCycle(ANIMCHANNEL_ALL, "idle");
	}

	if (WEAPON_LOWERWEAPON) {
		ChangeState(&dnWeaponPistol::State_Lower, PISTOL_IDLE_TO_LOWER);
		return;
	}

	currentTime = gameLocal.time;
	ammoClip = owner->AmmoInClip();
	if ((currentTime >= next_attack) && WEAPON_ATTACK) {
		if (ammoClip > 0) {
			ChangeState(&dnWeaponPistol::State_Fire, PISTOL_IDLE_TO_FIRE);
		}
		else if (owner->AmmoAvailable() > 0) {
			if (owner->NativeEvent_AutoReload()) {
				owner->NativeEvent_NetReload();
				ChangeState(&dnWeaponPistol::State_Reload, PISTOL_IDLE_TO_RELOAD);
			}
		}
		return;
	}

	if (WEAPON_RELOAD && (owner->AmmoAvailable() > ammoClip) && (ammoClip < clip_size)) {
		owner->NativeEvent_NetReload();
		ChangeState(&dnWeaponPistol::State_Reload, PISTOL_IDLE_TO_RELOAD);
		return;
	}

	if (WEAPON_NETRELOAD) {
		WEAPON_NETRELOAD = false;
		ChangeState(&dnWeaponPistol::State_Reload, PISTOL_IDLE_TO_RELOAD);
		return;
	}
}

/*
=====================
dnWeaponPistol::State_Lower
=====================
*/
void dnWeaponPistol::State_Lower() {
	owner->NativeEvent_WeaponLowering();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "putaway");
	ChangeState(&dnWeaponPistol::State_Lower_WaitForAnim, 0);
}

/*
=====================
dnWeaponPistol::State_Lower_WaitForAnim
=====================
*/
void dnWeaponPistol::State_Lower_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, 0)) {
		owner->NativeEvent_WeaponHolstered();
		if (WEAPON_RAISEWEAPON) {
			ChangeState(&dnWeaponPistol::State_Raise, 0);
		}
	}
}

/*
=====================
dnWeaponPistol::State_Raise
=====================
*/
void dnWeaponPistol::State_Raise() {
	owner->NativeEvent_WeaponRising();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "raise");
	ChangeState(&dnWeaponPistol::State_Raise_WaitForAnim, 0);
}

/*
=====================
dnWeaponPistol::State_Raise_WaitForAnim
=====================
*/
void dnWeaponPistol::State_Raise_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, PISTOL_RAISE_TO_IDLE)) {
		ChangeState(&dnWeaponPistol::State_Idle, PISTOL_RAISE_TO_IDLE);
	}
}

/*
=====================
dnWeaponPistol::State_Fire
=====================
*/
void dnWeaponPistol::State_Fire() {
	float ammoClip;

	next_attack = gameLocal.time + PISTOL_FIRERATE;

	ammoClip = owner->NativeEvent_AmmoInClip();
	if (ammoClip == PISTOL_LOWAMMO) {
		owner->NativeEvent_StartSound("snd_lowammo", SND_CHANNEL_ITEM, 0);
	}

	owner->NativeEvent_LaunchProjectiles(PISTOL_NUMPROJECTILES, spread, 0, 1.0f, 1.0f);
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "fire");
	ChangeState(&dnWeaponPistol::State_Fire_WaitForAnim, 0);
}

/*
=====================
dnWeaponPistol::State_Fire_WaitForAnim
=====================
*/
void dnWeaponPistol::State_Fire_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, PISTOL_FIRE_TO_IDLE)) {
		ChangeState(&dnWeaponPistol::State_Idle, PISTOL_FIRE_TO_IDLE);
	}
}

/*
=====================
dnWeaponPistol::State_Reload
=====================
*/
void dnWeaponPistol::State_Reload() {
	owner->NativeEvent_WeaponReloading();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "reload");
	ChangeState(&dnWeaponPistol::State_Reload_WaitForAnim, 0);
}

/*
=====================
dnWeaponPistol::State_Reload_WaitForAnim
=====================
*/
void dnWeaponPistol::State_Reload_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, PISTOL_RELOAD_TO_IDLE)) {
		owner->NativeEvent_AddToClip(owner->NativeEvent_ClipSize());
		ChangeState(&dnWeaponPistol::State_Idle, PISTOL_RELOAD_TO_IDLE);
	}
}
