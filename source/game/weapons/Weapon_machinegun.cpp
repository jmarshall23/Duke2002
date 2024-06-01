// Weapon_Machinegun.cpp
//

#include "precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

CLASS_DECLARATION(idWeaponBase, idWeaponMachinegun)
END_CLASS

#define MACHINEGUN_FIRERATE         0.1     // ~10 per second
#define MACHINEGUN_LOWAMMO          10
#define MACHINEGUN_NUMPROJECTILES   1

// blend times
#define MACHINEGUN_IDLE_TO_LOWER    4
#define MACHINEGUN_IDLE_TO_FIRE     0
#define MACHINEGUN_IDLE_TO_RELOAD   4
#define MACHINEGUN_RAISE_TO_IDLE    4
#define MACHINEGUN_FIRE_TO_IDLE     0
#define MACHINEGUN_RELOAD_TO_IDLE   4

/*
=====================
idWeaponPistol::idWeaponMachinegun
=====================
*/
idWeaponMachinegun::idWeaponMachinegun() : next_attack(0), spread(0) {
}

/*
=====================
idWeaponPistol::Init
=====================
*/
void idWeaponMachinegun::Init(idWeapon* owner) {
	idWeaponBase::Init(owner);

	next_attack = 0;
	spread = owner->CurrentWeaponDef()->dict.GetFloat("spread");
	ChangeState(&idWeaponMachinegun::State_Raise, 0);
}

/*
=====================
idWeaponPistol::InitStates
=====================
*/
void idWeaponMachinegun::InitStates() {
	// Initialize state transitions
}

/*
=====================
idWeaponPistol::State_Idle
=====================
*/
void idWeaponMachinegun::State_Idle() {
	if (WEAPON_LOWERWEAPON) {
		ChangeState(&idWeaponMachinegun::State_Lower, MACHINEGUN_IDLE_TO_LOWER);
		return;
	}

	float currentTime = gameLocal.time;
	float ammoClip = owner->AmmoInClip();
	float clip_size = owner->ClipSize();

	if ((currentTime >= next_attack) && WEAPON_ATTACK) {
		if (ammoClip > 0) {
			ChangeState(&idWeaponMachinegun::State_Fire, MACHINEGUN_IDLE_TO_FIRE);
		}
		else if (owner->AmmoAvailable() > 0) {
			if (owner->NativeEvent_AutoReload()) {
				owner->NativeEvent_NetReload();
				ChangeState(&idWeaponMachinegun::State_Reload, MACHINEGUN_IDLE_TO_RELOAD);
			}
		}
		return;
	}

	if (WEAPON_RELOAD && (owner->AmmoAvailable() > ammoClip) && (ammoClip < clip_size)) {
		owner->NativeEvent_NetReload();
		ChangeState(&idWeaponMachinegun::State_Reload, MACHINEGUN_IDLE_TO_RELOAD);
		return;
	}

	if (WEAPON_NETRELOAD) {
		WEAPON_NETRELOAD = false;
		ChangeState(&idWeaponMachinegun::State_Reload, MACHINEGUN_IDLE_TO_RELOAD);
		return;
	}

	// Default idle behavior
	if (!owner->NativeEvent_AmmoInClip()) {
		owner->NativeEvent_WeaponOutOfAmmo();
	}
	else {
		owner->NativeEvent_WeaponReady();
	}

	owner->NativeEvent_PlayCycle(ANIMCHANNEL_ALL, "idle");
}

/*
=====================
idWeaponPistol::State_Lower
=====================
*/
void idWeaponMachinegun::State_Lower() {
	owner->NativeEvent_WeaponLowering();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "putaway");
	ChangeState(&idWeaponMachinegun::State_Lower_WaitForAnim, 0);
}

/*
=====================
idWeaponPistol::State_Lower_WaitForAnim
=====================
*/
void idWeaponMachinegun::State_Lower_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, 0)) {
		owner->NativeEvent_WeaponHolstered();
		if (WEAPON_RAISEWEAPON) {
			ChangeState(&idWeaponMachinegun::State_Raise, 0);
		}
	}
}

/*
=====================
idWeaponPistol::State_Raise
=====================
*/
void idWeaponMachinegun::State_Raise() {
	owner->NativeEvent_WeaponRising();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "raise");
	ChangeState(&idWeaponMachinegun::State_Raise_WaitForAnim, 0);
}

/*
=====================
idWeaponPistol::State_Raise_WaitForAnim
=====================
*/
void idWeaponMachinegun::State_Raise_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, MACHINEGUN_RAISE_TO_IDLE)) {
		ChangeState(&idWeaponMachinegun::State_Idle, MACHINEGUN_RAISE_TO_IDLE);
	}
}

/*
=====================
idWeaponPistol::State_Fire
=====================
*/
void idWeaponMachinegun::State_Fire() {
	next_attack = gameLocal.time + MACHINEGUN_FIRERATE;
	float ammoClip = owner->NativeEvent_AmmoInClip();

	if (ammoClip == MACHINEGUN_LOWAMMO) {
		owner->NativeEvent_StartSound("snd_lowammo", SND_CHANNEL_ITEM, true);
	}

	owner->NativeEvent_LaunchProjectiles(MACHINEGUN_NUMPROJECTILES, spread, 0, 1.0f, 1.0f);
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "fire");
	ChangeState(&idWeaponMachinegun::State_Fire_WaitForAnim, 0);
}

/*
=====================
idWeaponPistol::State_Fire_WaitForAnim
=====================
*/
void idWeaponMachinegun::State_Fire_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, MACHINEGUN_FIRE_TO_IDLE)) {
		ChangeState(&idWeaponMachinegun::State_Idle, MACHINEGUN_FIRE_TO_IDLE);
	}
	else {
		float currentTime = gameLocal.time;
		float ammoClip = owner->NativeEvent_AmmoInClip();

		if ((currentTime >= next_attack) && WEAPON_ATTACK && (ammoClip > 0)) {
			ChangeState(&idWeaponMachinegun::State_Fire, 0);
		}
	}
}

/*
=====================
idWeaponPistol::State_Reload
=====================
*/
void idWeaponMachinegun::State_Reload() {
	owner->NativeEvent_WeaponReloading();
	owner->NativeEvent_PlayAnim(ANIMCHANNEL_ALL, "reload");
	ChangeState(&idWeaponMachinegun::State_Reload_WaitForAnim, 0);
}

/*
=====================
idWeaponPistol::State_Reload_WaitForAnim
=====================
*/
void idWeaponMachinegun::State_Reload_WaitForAnim() {
	if (owner->NativeEvent_AnimDone(ANIMCHANNEL_ALL, MACHINEGUN_RELOAD_TO_IDLE)) {
		owner->NativeEvent_AddToClip(owner->ClipSize());
		ChangeState(&idWeaponMachinegun::State_Idle, MACHINEGUN_RELOAD_TO_IDLE);
	}
}

/*
=====================
idWeaponPistol::ExitCinematic
=====================
*/
void idWeaponMachinegun::ExitCinematic() {
	next_attack = 0;
	ChangeState(&idWeaponMachinegun::State_Idle, 0);
}