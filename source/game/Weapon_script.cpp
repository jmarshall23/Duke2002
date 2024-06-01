// Weapon_Script.cpp
//

#include "precompiled.h"

#include "Game_local.h"


//
// event defs
//
const idEventDef EV_Weapon_Clear("<clear>");
const idEventDef EV_Weapon_GetOwner("getOwner", NULL, 'e');
const idEventDef EV_Weapon_Next("nextWeapon");
const idEventDef EV_Weapon_State("weaponState", "sd");
const idEventDef EV_Weapon_UseAmmo("useAmmo", "d");
const idEventDef EV_Weapon_AddToClip("addToClip", "d");
const idEventDef EV_Weapon_AmmoInClip("ammoInClip", NULL, 'f');
const idEventDef EV_Weapon_AmmoAvailable("ammoAvailable", NULL, 'f');
const idEventDef EV_Weapon_TotalAmmoCount("totalAmmoCount", NULL, 'f');
const idEventDef EV_Weapon_ClipSize("clipSize", NULL, 'f');
const idEventDef EV_Weapon_WeaponOutOfAmmo("weaponOutOfAmmo");
const idEventDef EV_Weapon_WeaponReady("weaponReady");
const idEventDef EV_Weapon_WeaponReloading("weaponReloading");
const idEventDef EV_Weapon_WeaponHolstered("weaponHolstered");
const idEventDef EV_Weapon_WeaponRising("weaponRising");
const idEventDef EV_Weapon_WeaponLowering("weaponLowering");
const idEventDef EV_Weapon_Flashlight("flashlight", "d");
const idEventDef EV_Weapon_LaunchProjectiles("launchProjectiles", "dffff");
const idEventDef EV_Weapon_CreateProjectile("createProjectile", NULL, 'e');
const idEventDef EV_Weapon_EjectBrass("ejectBrass");
const idEventDef EV_Weapon_Melee("melee", NULL, 'd');
const idEventDef EV_Weapon_GetWorldModel("getWorldModel", NULL, 'e');
const idEventDef EV_Weapon_AllowDrop("allowDrop", "d");
const idEventDef EV_Weapon_AutoReload("autoReload", NULL, 'f');
const idEventDef EV_Weapon_NetReload("netReload");
const idEventDef EV_Weapon_IsInvisible("isInvisible", NULL, 'f');
const idEventDef EV_Weapon_NetEndReload("netEndReload");

//
// class def
//
CLASS_DECLARATION(idAnimatedEntity, idWeapon)
	EVENT(EV_Weapon_Clear, idWeapon::Event_Clear)
	EVENT(EV_Weapon_GetOwner, idWeapon::Event_GetOwner)
	EVENT(EV_Weapon_State, idWeapon::Event_WeaponState)
	EVENT(EV_Weapon_WeaponReady, idWeapon::Event_WeaponReady)
	EVENT(EV_Weapon_WeaponOutOfAmmo, idWeapon::Event_WeaponOutOfAmmo)
	EVENT(EV_Weapon_WeaponReloading, idWeapon::Event_WeaponReloading)
	EVENT(EV_Weapon_WeaponHolstered, idWeapon::Event_WeaponHolstered)
	EVENT(EV_Weapon_WeaponRising, idWeapon::Event_WeaponRising)
	EVENT(EV_Weapon_WeaponLowering, idWeapon::Event_WeaponLowering)
	EVENT(EV_Weapon_UseAmmo, idWeapon::Event_UseAmmo)
	EVENT(EV_Weapon_AddToClip, idWeapon::Event_AddToClip)
	EVENT(EV_Weapon_AmmoInClip, idWeapon::Event_AmmoInClip)
	EVENT(EV_Weapon_AmmoAvailable, idWeapon::Event_AmmoAvailable)
	EVENT(EV_Weapon_TotalAmmoCount, idWeapon::Event_TotalAmmoCount)
	EVENT(EV_Weapon_ClipSize, idWeapon::Event_ClipSize)
	EVENT(AI_PlayAnim, idWeapon::Event_PlayAnim)
	EVENT(AI_PlayCycle, idWeapon::Event_PlayCycle)
	EVENT(AI_SetBlendFrames, idWeapon::Event_SetBlendFrames)
	EVENT(AI_GetBlendFrames, idWeapon::Event_GetBlendFrames)
	EVENT(AI_AnimDone, idWeapon::Event_AnimDone)
	EVENT(EV_Weapon_Next, idWeapon::Event_Next)
	EVENT(EV_SetSkin, idWeapon::Event_SetSkin)
	EVENT(EV_Weapon_Flashlight, idWeapon::Event_Flashlight)
	EVENT(EV_Light_GetLightParm, idWeapon::Event_GetLightParm)
	EVENT(EV_Light_SetLightParm, idWeapon::Event_SetLightParm)
	EVENT(EV_Light_SetLightParms, idWeapon::Event_SetLightParms)
	EVENT(EV_Weapon_LaunchProjectiles, idWeapon::Event_LaunchProjectiles)
	EVENT(EV_Weapon_CreateProjectile, idWeapon::Event_CreateProjectile)
	EVENT(EV_Weapon_EjectBrass, idWeapon::Event_EjectBrass)
	EVENT(EV_Weapon_Melee, idWeapon::Event_Melee)
	EVENT(EV_Weapon_GetWorldModel, idWeapon::Event_GetWorldModel)
	EVENT(EV_Weapon_AllowDrop, idWeapon::Event_AllowDrop)
	EVENT(EV_Weapon_AutoReload, idWeapon::Event_AutoReload)
	EVENT(EV_Weapon_NetReload, idWeapon::Event_NetReload)
	EVENT(EV_Weapon_IsInvisible, idWeapon::Event_IsInvisible)
	EVENT(EV_Weapon_NetEndReload, idWeapon::Event_NetEndReload)
END_CLASS

void idWeapon::Event_Clear() {
	NativeEvent_Clear();
}

void idWeapon::Event_GetOwner() {
	idThread::ReturnEntity(NativeEvent_GetOwner());
}

void idWeapon::Event_WeaponState(const char* statename, int blendFrames) {
	NativeEvent_WeaponState(statename, blendFrames);
}

void idWeapon::Event_UseAmmo(int amount) {
	NativeEvent_UseAmmo(amount);
}

void idWeapon::Event_AddToClip(int amount) {
	NativeEvent_AddToClip(amount);
}

void idWeapon::Event_AmmoInClip() {
	idThread::ReturnInt(NativeEvent_AmmoInClip());
}

void idWeapon::Event_AmmoAvailable() {
	idThread::ReturnInt(NativeEvent_AmmoAvailable());
}

void idWeapon::Event_TotalAmmoCount() {
	idThread::ReturnInt(NativeEvent_TotalAmmoCount());
}

void idWeapon::Event_ClipSize() {
	idThread::ReturnInt(NativeEvent_ClipSize());
}

void idWeapon::Event_AutoReload() {
	idThread::ReturnFloat(NativeEvent_AutoReload());
}

void idWeapon::Event_NetReload() {
	NativeEvent_NetReload();
}

void idWeapon::Event_NetEndReload() {
	NativeEvent_NetEndReload();
}

void idWeapon::Event_PlayAnim(int channel, const char* animname) {
	NativeEvent_PlayAnim(channel, animname);
}

void idWeapon::Event_PlayCycle(int channel, const char* animname) {
	NativeEvent_PlayCycle(channel, animname);
}

void idWeapon::Event_AnimDone(int channel, int blendFrames) {
	idThread::ReturnInt(NativeEvent_AnimDone(channel, blendFrames));
}

void idWeapon::Event_SetBlendFrames(int channel, int blendFrames) {
	NativeEvent_SetBlendFrames(blendFrames);
}

void idWeapon::Event_GetBlendFrames(int channel) {
	idThread::ReturnInt(NativeEvent_GetBlendFrames());
}

void idWeapon::Event_Next() {
	NativeEvent_Next();
}

void idWeapon::Event_SetSkin(const char* skinname) {
	NativeEvent_SetSkin(skinname);
}

void idWeapon::Event_Flashlight(int enable) {
	NativeEvent_Flashlight(enable);
}

void idWeapon::Event_GetLightParm(int parmnum) {
	idThread::ReturnFloat(NativeEvent_GetLightParm(parmnum));
}

void idWeapon::Event_SetLightParm(int parmnum, float value) {
	NativeEvent_SetLightParm(parmnum, value);
}

void idWeapon::Event_SetLightParms(float parm0, float parm1, float parm2, float parm3) {
	NativeEvent_SetLightParms(parm0, parm1, parm2, parm3);
}

void idWeapon::Event_CreateProjectile() {
	idThread::ReturnEntity(NativeEvent_CreateProjectile());
}

void idWeapon::Event_LaunchProjectiles(int num_projectiles, float spread, float fuseOffset, float launchPower, float dmgPower) {
	NativeEvent_LaunchProjectiles(num_projectiles, spread, fuseOffset, launchPower, dmgPower);
}

void idWeapon::Event_Melee() {
	idThread::ReturnInt(NativeEvent_Melee());
}

void idWeapon::Event_GetWorldModel() {
	idThread::ReturnEntity(NativeEvent_GetWorldModel());
}

void idWeapon::Event_AllowDrop(int allow) {
	NativeEvent_AllowDrop(allow);
}

void idWeapon::Event_EjectBrass() {
	NativeEvent_EjectBrass();
}

void idWeapon::Event_IsInvisible() {
	idThread::ReturnFloat(NativeEvent_IsInvisible());
}

void idWeapon::Event_WeaponReady() {
	NativeEvent_WeaponReady();
}

void idWeapon::Event_WeaponOutOfAmmo() {
	NativeEvent_WeaponOutOfAmmo();
}

void idWeapon::Event_WeaponReloading() {
	NativeEvent_WeaponReloading();
}

void idWeapon::Event_WeaponHolstered() {
	NativeEvent_WeaponHolstered();
}

void idWeapon::Event_WeaponRising() {
	NativeEvent_WeaponRising();
}

void idWeapon::Event_WeaponLowering() {
	NativeEvent_WeaponLowering();
}