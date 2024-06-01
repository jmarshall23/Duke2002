// Weapon_shotgun.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

CLASS_DECLARATION(idWeaponBase, idWeaponShotgun)
END_CLASS

#define SHOTGUN_FIRERATE        1.333
#define SHOTGUN_LOWAMMO         2
#define SHOTGUN_RELOADRATE      2
#define SHOTGUN_NUMPROJECTILES  13

// blend times
#define SHOTGUN_IDLE_TO_IDLE    0
#define SHOTGUN_IDLE_TO_LOWER   4
#define SHOTGUN_IDLE_TO_FIRE    1
#define SHOTGUN_IDLE_TO_RELOAD  4
#define SHOTGUN_IDLE_TO_NOAMMO  4
#define SHOTGUN_NOAMMO_TO_RELOAD 4
#define SHOTGUN_NOAMMO_TO_IDLE  4
#define SHOTGUN_RAISE_TO_IDLE   1
#define SHOTGUN_FIRE_TO_IDLE    4
#define SHOTGUN_RELOAD_TO_IDLE  4
#define SHOTGUN_RELOAD_TO_FIRE  4
#define SHOTGUN_RELOAD_TO_LOWER 2

/*
=====================
idWeaponShotgun::idWeaponShotgun
=====================
*/
idWeaponShotgun::idWeaponShotgun() : next_attack(0), spread(0), side(0) {
}

/*
=====================
idWeaponShotgun::Init
=====================
*/
void idWeaponShotgun::Init(idWeapon* owner) {
	idWeaponBase::Init(owner);

	next_attack = 0;
	spread = owner->CurrentWeaponDef()->dict.GetFloat("spread");
	ChangeState(&idWeaponShotgun::State_Raise, 0);
}

/*
=====================
idWeaponShotgun::InitStates
=====================
*/
void idWeaponShotgun::InitStates() {
	// Initialize state transitions
}

/*
=====================
idWeaponShotgun::State_Idle
=====================
*/
void idWeaponShotgun::State_Idle() {
	if (WEAPON_LOWERWEAPON) {
		ChangeState(&idWeaponShotgun::State_Lower, SHOTGUN_IDLE_TO_LOWER);
		return;
	}

	float currentTime = gameLocal.time;
	float ammoClip = owner->AmmoInClip();
	float clip_size = owner->ClipSize();

	if ((currentTime >= next_attack) && WEAPON_ATTACK) {
		if (ammoClip > 0) {
			ChangeState(&idWeaponShotgun::State_Fire, SHOTGUN_IDLE_TO_FIRE);
		}
		else if (owner->AmmoAvailable() > 0) {
			if (owner->NativeEvent_AutoReload()) {
				owner->NativeEvent_NetReload();
				ChangeState(&idWeaponShotgun::State_Reload, SHOTGUN_IDLE_TO_RELOAD);
			}
			else {
				ChangeState(&idWeaponShotgun::State_NoAmmo, SHOTGUN_IDLE_TO_NOAMMO);
			}
		}
		else {
			ChangeState(&idWeaponShotgun::State_NoAmmo, SHOTGUN_IDLE_TO_NOAMMO);
		}
		return;
	}

	if (WEAPON_RELOAD && (owner->AmmoAvailable() > ammoClip) && (ammoClip < clip_size)) {
		owner->NativeEvent_NetReload();
		ChangeState(&idWeaponShotgun::State_Reload, SHOTGUN_IDLE_TO_RELOAD);
		return;
	}

	if (WEAPON_NETRELOAD) {
		WEAPON_NETRELOAD = false;
		ChangeState(&idWeaponShotgun::State_Reload, SHOTGUN_IDLE_TO_RELOAD);
		return;
	}

	// Default idle behavior
	if (!owner->AmmoInClip()) {
		owner->NativeEvent_WeaponOutOfAmmo();
	}
	else {
		owner->NativeEvent_WeaponReady();
	}

	owner->NativeEvent_PlayCycle(ANIMCHANNEL_ALL, "idle");
}

/*
=====================
idWeaponShotgun::State_Lower
=====================
*/
void idWeaponShotgun::State_Lower() {
	owner->NativeEvent_WeaponLowering();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "putaway");
	ChangeState(&idWeaponShotgun::State_Lower_WaitForAnim, 0);
}

/*
=====================
idWeaponShotgun::State_Lower_WaitForAnim
=====================
*/
void idWeaponShotgun::State_Lower_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, 0)) {
		owner->NativeEvent_WeaponHolstered();
		if (WEAPON_RAISEWEAPON) {
			ChangeState(&idWeaponShotgun::State_Raise, 0);
		}
	}
}

/*
=====================
idWeaponShotgun::State_Raise
=====================
*/
void idWeaponShotgun::State_Raise() {
	owner->NativeEvent_WeaponRising();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "raise");
	ChangeState(&idWeaponShotgun::State_Raise_WaitForAnim, 0);
}

/*
=====================
idWeaponShotgun::State_Raise_WaitForAnim
=====================
*/
void idWeaponShotgun::State_Raise_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, SHOTGUN_RAISE_TO_IDLE)) {
		ChangeState(&idWeaponShotgun::State_Idle, SHOTGUN_RAISE_TO_IDLE);
	}
}

/*
=====================
idWeaponShotgun::State_NoAmmo
=====================
*/
void idWeaponShotgun::State_NoAmmo() {
	if (WEAPON_NETRELOAD) {
		WEAPON_NETRELOAD = false;
		ChangeState(&idWeaponShotgun::State_Reload, SHOTGUN_IDLE_TO_RELOAD);
		return;
	}

	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "noammo");
	ChangeState(&idWeaponShotgun::State_Idle, SHOTGUN_NOAMMO_TO_IDLE);
}

/*
=====================
idWeaponShotgun::State_Fire
=====================
*/
void idWeaponShotgun::State_Fire() {
	if (WEAPON_NETRELOAD) {
		WEAPON_NETRELOAD = false;
		ChangeState(&idWeaponShotgun::State_Reload, SHOTGUN_IDLE_TO_RELOAD);
		return;
	}

	next_attack = gameLocal.time + SHOTGUN_FIRERATE;
	float ammoClip = owner->NativeEvent_AmmoInClip();

	if (ammoClip == SHOTGUN_LOWAMMO) {
		owner->NativeEvent_StartSound("snd_lowammo", SND_CHANNEL_ITEM, true);
	}

	owner->NativeEvent_LaunchProjectiles(SHOTGUN_NUMPROJECTILES, spread, 0, 1.0f, 1.0f);
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "fire");
	ChangeState(&idWeaponShotgun::State_Fire_WaitForAnim, 0);
}

/*
=====================
idWeaponShotgun::State_Fire_WaitForAnim
=====================
*/
void idWeaponShotgun::State_Fire_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, SHOTGUN_FIRE_TO_IDLE)) {
		ChangeState(&idWeaponShotgun::State_Idle, SHOTGUN_FIRE_TO_IDLE);
	}
}

/*
=====================
idWeaponShotgun::State_Reload
=====================
*/
void idWeaponShotgun::State_Reload() {
	owner->NativeEvent_WeaponReloading();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "reload_start");
	ChangeState(&idWeaponShotgun::State_Reload_Start, 0);
}

/*
=====================
idWeaponShotgun::State_Reload_Start
=====================
*/
void idWeaponShotgun::State_Reload_Start() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, 0)) {
		ChangeState(&idWeaponShotgun::State_Reload_Loop, 0);
	}
}

/*
=====================
idWeaponShotgun::State_Reload_Loop
=====================
*/
void idWeaponShotgun::State_Reload_Loop() {
	float ammoClip = owner->AmmoInClip();
	float ammoAvail = owner->AmmoAvailable();
	float clip_size = owner->ClipSize();

	if ((ammoClip < clip_size) && (ammoClip < ammoAvail)) {
		owner->NativeEvent_WeaponReloading();

		if (!isReloadAnimPlaying)
		{
			owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "reload_loop");
		}

		isReloadAnimPlaying = true;

		if (WEAPON_ATTACK && (ammoClip > 0)) {
			ChangeState(&idWeaponShotgun::State_Fire, SHOTGUN_RELOAD_TO_FIRE);
			isReloadAnimPlaying = false;
			return;
		}

		if (WEAPON_LOWERWEAPON || WEAPON_NETENDRELOAD) {
			ChangeState(&idWeaponShotgun::State_Lower, SHOTGUN_RELOAD_TO_LOWER);
			isReloadAnimPlaying = false;
			return;
		}

		if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, 0)) {
			owner->NativeEvent_AddToClip(SHOTGUN_RELOADRATE);			
			owner->NativeEvent_WeaponReady();
			isReloadAnimPlaying = false;
		}
	}
	else {
		owner->NativeEvent_NetEndReload();
		owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "reload_end");
		ChangeState(&idWeaponShotgun::State_Reload_End, 0);
		isReloadAnimPlaying = false;
	}
}

/*
=====================
idWeaponShotgun::State_Reload_End
=====================
*/
void idWeaponShotgun::State_Reload_End() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, SHOTGUN_RELOAD_TO_IDLE)) {
		ChangeState(&idWeaponShotgun::State_Idle, SHOTGUN_RELOAD_TO_IDLE);
	}
}

/*
=====================
idWeaponShotgun::ExitCinematic
=====================
*/
void idWeaponShotgun::ExitCinematic() {
	next_attack = 0;
	ChangeState(&idWeaponShotgun::State_Idle, 0);
}

/*
=====================
idWeaponShotgun::StateChangedEvent
=====================
*/
void idWeaponShotgun::StateChangedEvent(void) {
	isReloadAnimPlaying = false;
}