/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __GAME_WEAPON_H__
#define __GAME_WEAPON_H__

/*
===============================================================================

	Player Weapon
	
===============================================================================
*/

class idWeapon;

#define WEAPON_STATE_FUNC(func) static_cast<idWeaponBase::StateFunction>(&func)

#pragma once

class idWeaponBase : public idClass {
friend class idWeapon;
public:
	ABSTRACT_PROTOTYPE(idWeaponBase);

	idWeaponBase();
	virtual ~idWeaponBase() = default;

	using StateFunction = void (idWeaponBase::*)();
	
	virtual void Init(idWeapon* owner) {
		this->owner = owner;
	}

	void EnterCinematic();
	virtual void ExitCinematic();
	void NetCatchup();
	void WeaponStolen();
	void OwnerDied();
	void UpdateSkin();
	void ToggleFlashLight(void);
	virtual bool WeaponHasFlashLight() { return true; }
	virtual idStr GetFireAnim();

	template<typename T>
	void ChangeState(void (T::* newState)(), int blendFrames);
	void Update();
	void SetState(const char* name, int blendFrames);

	virtual void StateChangedEvent(void) { }
protected:
	idWeapon* owner;

	bool flashLightOn = false;

	StateFunction currentState;
	StateFunction nextState;
	int blendFrames;
	bool WEAPON_ATTACK;
	bool WEAPON_RELOAD;
	bool WEAPON_NETRELOAD;
	bool WEAPON_NETENDRELOAD;
	bool WEAPON_RAISEWEAPON;
	bool WEAPON_LOWERWEAPON;
	bool WEAPON_START_FIRING;

	virtual void InitStates() = 0;

	// Virtual state methods to be implemented by derived classes
	virtual void State_Idle() = 0;
	virtual void State_Lower() = 0;
	virtual void State_Raise() = 0;
	virtual void State_Fire() = 0;
	virtual void State_Reload() = 0;
};

template<typename T>
ID_INLINE void idWeaponBase::ChangeState(void (T::* newState)(), int blendFrames) {
	this->nextState = static_cast<StateFunction>(newState);
	this->blendFrames = blendFrames;
	StateChangedEvent();
}

#include "weapons/Weapon_fists.h"
#include "weapons/Weapon_pistol.h"
#include "weapons/Weapon_shotgun.h"
#include "weapons/Weapon_machinegun.h"

typedef enum {
	WP_READY,
	WP_OUTOFAMMO,
	WP_RELOAD,
	WP_HOLSTERED,
	WP_RISING,
	WP_LOWERING
} weaponStatus_t;

typedef int ammo_t;
static const int AMMO_NUMTYPES = 16;

class idPlayer;

static const int LIGHTID_WORLD_MUZZLE_FLASH = 1;
static const int LIGHTID_VIEW_MUZZLE_FLASH = 100;

class idMoveableItem;

class idWeapon : public idAnimatedEntity {
public:
	friend class idWeaponBase;

	CLASS_PROTOTYPE( idWeapon );

							idWeapon();
	virtual					~idWeapon();

	// Init
	void					Spawn( void );
	void					SetOwner( idPlayer *owner );
	idPlayer*				GetOwner( void );
	virtual bool			ShouldConstructScriptObjectAtSpawn( void ) const;

	static void				CacheWeapon( const char *weaponName );

	// save games
	void					Save( idSaveGame *savefile ) const;					// archives object for save game file
	void					Restore( idRestoreGame *savefile );					// unarchives object from save game file

	// Weapon definition management
	void					Clear( void );
	void					GetWeaponDef( const char *objectname, int ammoinclip );
	void					InitWeapon(int currentWeaponId);
	bool					IsLinked() { return scriptWeapon != nullptr; }
	int						GetCurrentWeaponId() { return currentWeaponId; }
	bool					IsWorldModelReady( void );

	// GUIs
	const char *			Icon( void ) const;
	void					UpdateGUI( void );

	virtual void			SetModel( const char *modelname );
	bool					GetGlobalJointTransform( bool viewModel, const jointHandle_t jointHandle, idVec3 &offset, idMat3 &axis );
	void					SetPushVelocity( const idVec3 &pushVelocity );
	bool					UpdateSkin( void );

	// State control/player interface
	void					Think( void );
	void					Raise( void );
	void					PutAway( void );
	void					Reload( void );
	void					LowerWeapon( void );
	void					RaiseWeapon( void );
	void					HideWeapon( void );
	void					ShowWeapon( void );
	void					HideWorldModel( void );
	void					ShowWorldModel( void );
	void					OwnerDied( void );
	void					BeginAttack( void );
	void					EndAttack( void );
	bool					IsReady( void ) const;
	bool					IsReloading( void ) const;
	bool					IsHolstered( void ) const;
	bool					ShowCrosshair( void ) const;
	idEntity *				DropItem( const idVec3 &velocity, int activateDelay, int removeDelay, bool died );
	bool					CanDrop( void ) const;
	void					WeaponStolen( void );	
	void					SetState( const char *statename, int blendFrames );
	void					EnterCinematic( void );
	void					ExitCinematic( void );
	void					NetCatchup( void );

	// Visual presentation
	void					PresentWeapon( bool showViewModel );
	int						GetZoomFov( void );
	void					GetWeaponAngleOffsets( int *average, float *scale, float *max );
	void					GetWeaponTimeOffsets( float *time, float *scale );
	bool					BloodSplat( float size );

	// Ammo
	static ammo_t			GetAmmoNumForName( const char *ammoname );
	static const char		*GetAmmoNameForNum( ammo_t ammonum );
	static const char		*GetAmmoPickupNameForNum( ammo_t ammonum );
	ammo_t					GetAmmoType( void ) const;
	int						AmmoAvailable( void ) const;
	int						AmmoInClip( void ) const;
	void					ResetAmmoClip( void );
	int						ClipSize( void ) const;
	int						LowAmmo( void ) const;
	int						AmmoRequired( void ) const;

	void					ToggleFlashLight(void);

	virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );

	enum {
		EVENT_RELOAD = idEntity::EVENT_MAXEVENTS,
		EVENT_ENDRELOAD,
		EVENT_CHANGESKIN,
		EVENT_MAXEVENTS
	};
	virtual bool			ClientReceiveEvent( int event, int time, const idBitMsg &msg );

	virtual void			ClientPredictionThink( void );

	const idDeclEntityDef*  CurrentWeaponDef() { return weaponDef; }
	void NativeEvent_Clear();
	idEntity* NativeEvent_GetOwner();
	void NativeEvent_WeaponState(const char* statename, int blendFrames);
	void NativeEvent_UseAmmo(int amount);
	void NativeEvent_AddToClip(int amount);
	int NativeEvent_AmmoInClip();
	int NativeEvent_AmmoAvailable();
	int NativeEvent_TotalAmmoCount();
	int NativeEvent_ClipSize();
	float NativeEvent_AutoReload();
	void NativeEvent_NetReload();
	void NativeEvent_NetEndReload();
	void NativeEvent_PlayAnim(int channel, const char* animname);
	void NativeEvent_PlayCycle(int channel, const char* animname);
	bool NativeEvent_AnimDone(int channel, int blendFrames);
	void NativeEvent_SetBlendFrames(int blendFrames);
	int NativeEvent_GetBlendFrames();
	void NativeEvent_Next();
	void NativeEvent_SetSkin(const char* skinname);
	void NativeEvent_Flashlight(int enable);
	float NativeEvent_GetLightParm(int parmnum);
	void NativeEvent_SetLightParm(int parmnum, float value);
	void NativeEvent_SetLightParms(float parm0, float parm1, float parm2, float parm3);
	idEntity* NativeEvent_CreateProjectile();
	void NativeEvent_LaunchProjectiles(int num_projectiles, float spread, float fuseOffset, float launchPower, float dmgPower);
	bool NativeEvent_Melee();
	idEntity* NativeEvent_GetWorldModel();
	void NativeEvent_AllowDrop(int allow);
	void NativeEvent_EjectBrass();
	float NativeEvent_IsInvisible();
	void NativeEvent_WeaponReady();
	void NativeEvent_WeaponOutOfAmmo();
	void NativeEvent_WeaponReloading();
	void NativeEvent_WeaponHolstered();
	void NativeEvent_WeaponRising();
	void NativeEvent_WeaponLowering();
protected:
	// script control
	idWeaponBase*			scriptWeapon;
	weaponStatus_t			status;

	int						currentWeaponId;
	int						animBlendFrames;
	int						animDoneTime;

	// precreated projectile
	idEntity				*projectileEnt;

	idPlayer *				owner;
	idEntityPtr<idAnimatedEntity>	worldModel;

	// hiding (for GUIs and NPCs)
	int						hideTime;
	float					hideDistance;
	int						hideStartTime;
	float					hideStart;
	float					hideEnd;
	float					hideOffset;
	bool					hide;
	bool					disabled;

	// berserk
	int						berserk;

	// these are the player render view parms, which include bobbing
	idVec3					playerViewOrigin;
	idMat3					playerViewAxis;

	// the view weapon render entity parms
	idVec3					viewWeaponOrigin;
	idMat3					viewWeaponAxis;
	
	// the muzzle bone's position, used for launching projectiles and trailing smoke
	idVec3					muzzleOrigin;
	idMat3					muzzleAxis;

	idVec3					pushVelocity;

	// weapon definition
	// we maintain local copies of the projectile and brass dictionaries so they
	// do not have to be copied across the DLL boundary when entities are spawned
	const idDeclEntityDef *	weaponDef;
	const idDeclEntityDef *	meleeDef;
	idDict					projectileDict;
	float					meleeDistance;
	idStr					meleeDefName;
	idDict					brassDict;
	int						brassDelay;
	idStr					icon;

	// view weapon gui light
	renderLight_t			guiLight;
	int						guiLightHandle;

	// muzzle flash
	renderLight_t			muzzleFlash;		// positioned on view weapon bone
	int						muzzleFlashHandle;

	renderLight_t			worldMuzzleFlash;	// positioned on world weapon bone
	int						worldMuzzleFlashHandle;

	idVec3					flashColor;
	int						muzzleFlashEnd;
	int						flashTime;
	bool					lightOn;
	bool					silent_fire;
	bool					allowDrop;

	// effects
	bool					hasBloodSplat;

	// weapon kick
	int						kick_endtime;
	int						muzzle_kick_time;
	int						muzzle_kick_maxtime;
	idAngles				muzzle_kick_angles;
	idVec3					muzzle_kick_offset;

	// ammo management
	ammo_t					ammoType;
	int						ammoRequired;		// amount of ammo to use each shot.  0 means weapon doesn't need ammo.
	int						clipSize;			// 0 means no reload
	int						ammoClip;
	int						lowAmmo;			// if ammo in clip hits this threshold, snd_
	bool					powerAmmo;			// true if the clip reduction is a factor of the power setting when
												// a projectile is launched
	// mp client
	bool					isFiring;

	// zoom
    int						zoomFov;			// variable zoom fov per weapon

	// joints from models
	jointHandle_t			barrelJointView;
	jointHandle_t			flashJointView;
	jointHandle_t			ejectJointView;
	jointHandle_t			guiLightJointView;
	jointHandle_t			ventLightJointView;

	jointHandle_t			flashJointWorld;
	jointHandle_t			barrelJointWorld;
	jointHandle_t			ejectJointWorld;

	// sound
	const idSoundShader *	sndHum;

	// new style muzzle smokes
	const idDeclParticle *	weaponSmoke;			// null if it doesn't smoke
	int						weaponSmokeStartTime;	// set to gameLocal.time every weapon fire
	bool					continuousSmoke;		// if smoke is continuous ( chainsaw )
	const idDeclParticle *  strikeSmoke;			// striking something in melee
	int						strikeSmokeStartTime;	// timing	
	idVec3					strikePos;				// position of last melee strike	
	idMat3					strikeAxis;				// axis of last melee strike
	int						nextStrikeFx;			// used for sound and decal ( may use for strike smoke too )

	// nozzle effects
	bool					nozzleFx;			// does this use nozzle effects ( parm5 at rest, parm6 firing )
										// this also assumes a nozzle light atm
	int						nozzleFxFade;		// time it takes to fade between the effects
	int						lastAttack;			// last time an attack occured
	renderLight_t			nozzleGlow;			// nozzle light
	int						nozzleGlowHandle;	// handle for nozzle light

	idVec3					nozzleGlowColor;	// color of the nozzle glow
	const idMaterial *		nozzleGlowShader;	// shader for glow light
	float					nozzleGlowRadius;	// radius of glow light

	// weighting for viewmodel angles
	int						weaponAngleOffsetAverages;
	float					weaponAngleOffsetScale;
	float					weaponAngleOffsetMax;
	float					weaponOffsetTime;
	float					weaponOffsetScale;

	// flashlight
	void					AlertMonsters( void );

	// Visual presentation
	void					InitWorldModel( const idDeclEntityDef *def );
	void					MuzzleFlashLight( void );
	void					MuzzleRise( idVec3 &origin, idMat3 &axis );
	void					UpdateNozzleFx( void );
	void					UpdateFlashPosition( void );

	// script events
	void					Event_Clear( void );
	void					Event_GetOwner( void );
	void					Event_WeaponState( const char *statename, int blendFrames );
	void					Event_SetWeaponStatus( float newStatus );
	void					Event_WeaponReady( void );
	void					Event_WeaponOutOfAmmo( void );
	void					Event_WeaponReloading( void );
	void					Event_WeaponHolstered( void );
	void					Event_WeaponRising( void );
	void					Event_WeaponLowering( void );
	void					Event_UseAmmo( int amount );
	void					Event_AddToClip( int amount );
	void					Event_AmmoInClip( void );
	void					Event_AmmoAvailable( void );
	void					Event_TotalAmmoCount( void );
	void					Event_ClipSize( void );
	void					Event_PlayAnim( int channel, const char *animname );
	void					Event_PlayCycle( int channel, const char *animname );
	void					Event_AnimDone( int channel, int blendFrames );
	void					Event_SetBlendFrames( int channel, int blendFrames );
	void					Event_GetBlendFrames( int channel );
	void					Event_Next( void );
	void					Event_SetSkin( const char *skinname );
	void					Event_Flashlight( int enable );
	void					Event_GetLightParm( int parmnum );
	void					Event_SetLightParm( int parmnum, float value );
	void					Event_SetLightParms( float parm0, float parm1, float parm2, float parm3 );
	void					Event_LaunchProjectiles( int num_projectiles, float spread, float fuseOffset, float launchPower, float dmgPower );
	void					Event_CreateProjectile( void );
	void					Event_EjectBrass( void );
	void					Event_Melee( void );
	void					Event_GetWorldModel( void );
	void					Event_AllowDrop( int allow );
	void					Event_AutoReload( void );
	void					Event_NetReload( void );
	void					Event_IsInvisible( void );
	void					Event_NetEndReload( void );
};

ID_INLINE bool idWeapon::IsWorldModelReady( void ) {
	return ( worldModel.GetEntity() != NULL );
}

ID_INLINE idPlayer* idWeapon::GetOwner( void ) {
	return owner;
}

extern const idEventDef EV_Weapon_Clear;
extern const idEventDef EV_Weapon_EjectBrass;

#endif /* !__GAME_WEAPON_H__ */