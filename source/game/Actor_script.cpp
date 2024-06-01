// Actor_script.cpp
//


#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"


const idEventDef AI_EnableEyeFocus( "enableEyeFocus" );
const idEventDef AI_DisableEyeFocus( "disableEyeFocus" );
const idEventDef EV_Footstep( "footstep" );
const idEventDef EV_FootstepLeft( "leftFoot" );
const idEventDef EV_FootstepRight( "rightFoot" );
const idEventDef EV_EnableWalkIK( "EnableWalkIK" );
const idEventDef EV_DisableWalkIK( "DisableWalkIK" );
const idEventDef EV_EnableLegIK( "EnableLegIK", "d" );
const idEventDef EV_DisableLegIK( "DisableLegIK", "d" );
const idEventDef AI_StopAnim( "stopAnim", "dd" );
const idEventDef AI_PlayAnim( "playAnim", "ds", 'd' );
const idEventDef AI_PlayCycle( "playCycle", "ds", 'd' );
const idEventDef AI_IdleAnim( "idleAnim", "ds", 'd' );
const idEventDef AI_SetSyncedAnimWeight( "setSyncedAnimWeight", "ddf" );
const idEventDef AI_SetBlendFrames( "setBlendFrames", "dd" );
const idEventDef AI_GetBlendFrames( "getBlendFrames", "d", 'd' );
const idEventDef AI_AnimState( "animState", "dsd" );
const idEventDef AI_GetAnimState( "getAnimState", "d", 's' );
const idEventDef AI_InAnimState( "inAnimState", "ds", 'd' );
const idEventDef AI_FinishAction( "finishAction", "s" );
const idEventDef AI_AnimDone( "animDone", "dd", 'd' );
const idEventDef AI_OverrideAnim( "overrideAnim", "d" );
const idEventDef AI_EnableAnim( "enableAnim", "dd" );
const idEventDef AI_PreventPain( "preventPain", "f" );
const idEventDef AI_DisablePain( "disablePain" );
const idEventDef AI_EnablePain( "enablePain" );
const idEventDef AI_GetPainAnim( "getPainAnim", NULL, 's' );
const idEventDef AI_SetAnimPrefix( "setAnimPrefix", "s" );
const idEventDef AI_HasAnim( "hasAnim", "ds", 'f' );
const idEventDef AI_CheckAnim( "checkAnim", "ds" );
const idEventDef AI_ChooseAnim( "chooseAnim", "ds", 's' );
const idEventDef AI_AnimLength( "animLength", "ds", 'f' );
const idEventDef AI_AnimDistance( "animDistance", "ds", 'f' );
const idEventDef AI_HasEnemies( "hasEnemies", NULL, 'd' );
const idEventDef AI_NextEnemy( "nextEnemy", "E", 'e' );
const idEventDef AI_ClosestEnemyToPoint( "closestEnemyToPoint", "v", 'e' );
const idEventDef AI_SetNextState( "setNextState", "s" );
const idEventDef AI_SetState( "setState", "s" );
const idEventDef AI_GetState( "getState", NULL, 's' );
const idEventDef AI_GetHead( "getHead", NULL, 'e' );

CLASS_DECLARATION( idAFEntity_Gibbable, idActor )
	EVENT( AI_EnableEyeFocus,			idActor::Event_EnableEyeFocus )
	EVENT( AI_DisableEyeFocus,			idActor::Event_DisableEyeFocus )
	EVENT( EV_Footstep,					idActor::Event_Footstep )
	EVENT( EV_FootstepLeft,				idActor::Event_Footstep )
	EVENT( EV_FootstepRight,			idActor::Event_Footstep )
	EVENT( EV_EnableWalkIK,				idActor::Event_EnableWalkIK )
	EVENT( EV_DisableWalkIK,			idActor::Event_DisableWalkIK )
	EVENT( EV_EnableLegIK,				idActor::Event_EnableLegIK )
	EVENT( EV_DisableLegIK,				idActor::Event_DisableLegIK )
	EVENT( AI_PreventPain,				idActor::Event_PreventPain )
	EVENT( AI_DisablePain,				idActor::Event_DisablePain )
	EVENT( AI_EnablePain,				idActor::Event_EnablePain )
	EVENT( AI_GetPainAnim,				idActor::Event_GetPainAnim )
	EVENT( AI_SetAnimPrefix,			idActor::Event_SetAnimPrefix )
	EVENT( AI_StopAnim,					idActor::Event_StopAnim )
	EVENT( AI_PlayAnim,					idActor::Event_PlayAnim )
	EVENT( AI_PlayCycle,				idActor::Event_PlayCycle )
	EVENT( AI_IdleAnim,					idActor::Event_IdleAnim )
	EVENT( AI_SetSyncedAnimWeight,		idActor::Event_SetSyncedAnimWeight )
	EVENT( AI_SetBlendFrames,			idActor::Event_SetBlendFrames )
	EVENT( AI_GetBlendFrames,			idActor::Event_GetBlendFrames )
	EVENT( AI_AnimState,				idActor::Event_AnimState )
	EVENT( AI_GetAnimState,				idActor::Event_GetAnimState )
	EVENT( AI_InAnimState,				idActor::Event_InAnimState )
	EVENT( AI_FinishAction,				idActor::Event_FinishAction )
	EVENT( AI_AnimDone,					idActor::Event_AnimDone )
	EVENT( AI_OverrideAnim,				idActor::Event_OverrideAnim )
	EVENT( AI_EnableAnim,				idActor::Event_EnableAnim )
	EVENT( AI_HasAnim,					idActor::Event_HasAnim )
	EVENT( AI_CheckAnim,				idActor::Event_CheckAnim )
	EVENT( AI_ChooseAnim,				idActor::Event_ChooseAnim )
	EVENT( AI_AnimLength,				idActor::Event_AnimLength )
	EVENT( AI_AnimDistance,				idActor::Event_AnimDistance )
	EVENT( AI_HasEnemies,				idActor::Event_HasEnemies )
	EVENT( AI_NextEnemy,				idActor::Event_NextEnemy )
	EVENT( AI_ClosestEnemyToPoint,		idActor::Event_ClosestEnemyToPoint )
	EVENT( EV_StopSound,				idActor::Event_StopSound )
	EVENT( AI_SetNextState,				idActor::Event_SetNextState )
	EVENT( AI_SetState,					idActor::Event_SetState )
	EVENT( AI_GetState,					idActor::Event_GetState )
	EVENT( AI_GetHead,					idActor::Event_GetHead )
END_CLASS

void idActor::Event_EnableEyeFocus() {
	NativeEvent_EnableEyeFocus();
}

void idActor::Event_DisableEyeFocus() {
	NativeEvent_DisableEyeFocus();
}

void idActor::Event_Footstep() {
	NativeEvent_Footstep();
}

void idActor::Event_EnableWalkIK() {
	NativeEvent_EnableWalkIK();
}

void idActor::Event_DisableWalkIK() {
	NativeEvent_DisableWalkIK();
}

void idActor::Event_EnableLegIK(int num) {
	NativeEvent_EnableLegIK(num);
}

void idActor::Event_DisableLegIK(int num) {
	NativeEvent_DisableLegIK(num);
}

void idActor::Event_PreventPain(float duration) {
	NativeEvent_PreventPain(duration);
}

void idActor::Event_DisablePain() {
	NativeEvent_DisablePain();
}

void idActor::Event_EnablePain() {
	NativeEvent_EnablePain();
}

void idActor::Event_GetPainAnim() {
	idThread::ReturnString(NativeEvent_GetPainAnim());
}

void idActor::Event_SetAnimPrefix(const char* prefix) {
	NativeEvent_SetAnimPrefix(prefix);
}

void idActor::Event_StopAnim(int channel, int frames) {
	NativeEvent_StopAnim(channel, frames);
}

void idActor::Event_PlayAnim(int channel, const char* animname) {
	idThread::ReturnInt(NativeEvent_PlayAnim(channel, animname));
}

void idActor::Event_PlayCycle(int channel, const char* animname) {
	idThread::ReturnInt(NativeEvent_PlayCycle(channel, animname));
}

void idActor::Event_IdleAnim(int channel, const char* animname) {
	idThread::ReturnInt(NativeEvent_IdleAnim(channel, animname));
}

void idActor::Event_SetSyncedAnimWeight(int channel, int anim, float weight) {
	NativeEvent_SetSyncedAnimWeight(channel, anim, weight);
}

void idActor::Event_OverrideAnim(int channel) {
	NativeEvent_OverrideAnim(channel);
}

void idActor::Event_EnableAnim(int channel, int blendFrames) {
	NativeEvent_EnableAnim(channel, blendFrames);
}

void idActor::Event_SetBlendFrames(int channel, int blendFrames) {
	NativeEvent_SetBlendFrames(channel, blendFrames);
}

void idActor::Event_GetBlendFrames(int channel) {
	idThread::ReturnInt(NativeEvent_GetBlendFrames(channel));
}

void idActor::Event_AnimState(int channel, const char* statename, int blendFrames) {
	NativeEvent_AnimState(channel, statename, blendFrames);
}

void idActor::Event_GetAnimState(int channel) {
	idThread::ReturnString(NativeEvent_GetAnimState(channel));
}

void idActor::Event_InAnimState(int channel, const char* statename) {
	idThread::ReturnInt(NativeEvent_InAnimState(channel, statename));
}

void idActor::Event_FinishAction(const char* actionname) {
	NativeEvent_FinishAction(actionname);
}

void idActor::Event_AnimDone(int channel, int blendFrames) {
	idThread::ReturnInt(NativeEvent_AnimDone(channel, blendFrames));
}

void idActor::Event_HasAnim(int channel, const char* animname) {
	idThread::ReturnFloat(NativeEvent_HasAnim(channel, animname));
}

void idActor::Event_CheckAnim(int channel, const char* animname) {
	NativeEvent_CheckAnim(channel, animname);
}

void idActor::Event_ChooseAnim(int channel, const char* animname) {
	idThread::ReturnString(NativeEvent_ChooseAnim(channel, animname));
}

void idActor::Event_AnimLength(int channel, const char* animname) {
	idThread::ReturnFloat(NativeEvent_AnimLength(channel, animname));
}

void idActor::Event_AnimDistance(int channel, const char* animname) {
	idThread::ReturnFloat(NativeEvent_AnimDistance(channel, animname));
}

void idActor::Event_HasEnemies() {
	idThread::ReturnInt(NativeEvent_HasEnemies());
}

void idActor::Event_NextEnemy(idEntity* ent) {
	idThread::ReturnEntity(NativeEvent_NextEnemy(ent));
}

void idActor::Event_ClosestEnemyToPoint(const idVec3& pos) {
	idThread::ReturnEntity(NativeEvent_ClosestEnemyToPoint(pos));
}

void idActor::Event_StopSound(int channel, int netSync) {
	NativeEvent_StopSound(channel, netSync);
}

void idActor::Event_SetNextState(const char* name) {
	NativeEvent_SetNextState(name);
}

void idActor::Event_SetState(const char* name) {
	NativeEvent_SetState(name);
}

void idActor::Event_GetState() {
	idThread::ReturnString(NativeEvent_GetState());
}

void idActor::Event_GetHead() {
	idThread::ReturnEntity(NativeEvent_GetHead());
}