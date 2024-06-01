#include "precompiled.h"
#pragma hdrstop

#include "Game_local.h"

const idEventDef EV_PostSpawn( "<postspawn>", NULL );
const idEventDef EV_FindTargets( "<findTargets>", NULL );
const idEventDef EV_Touch( "<touch>", "et" );
const idEventDef EV_GetName( "getName", NULL, 's' );
const idEventDef EV_SetName( "setName", "s" );
const idEventDef EV_Activate( "activate", "e" );
const idEventDef EV_ActivateTargets( "activateTargets", "e" );
const idEventDef EV_NumTargets( "numTargets", NULL, 'f' );
const idEventDef EV_GetTarget( "getTarget", "f", 'e' );
const idEventDef EV_RandomTarget( "randomTarget", "s", 'e' );
const idEventDef EV_Bind( "bind", "e" );
const idEventDef EV_BindPosition( "bindPosition", "e" );
const idEventDef EV_BindToJoint( "bindToJoint", "esf" );
const idEventDef EV_Unbind( "unbind", NULL );
const idEventDef EV_RemoveBinds( "removeBinds" );
const idEventDef EV_SpawnBind( "<spawnbind>", NULL );
const idEventDef EV_SetOwner( "setOwner", "e" );
const idEventDef EV_SetModel( "setModel", "s" );
const idEventDef EV_SetSkin( "setSkin", "s" );
const idEventDef EV_GetWorldOrigin( "getWorldOrigin", NULL, 'v' );
const idEventDef EV_SetWorldOrigin( "setWorldOrigin", "v" );
const idEventDef EV_GetOrigin( "getOrigin", NULL, 'v' );
const idEventDef EV_SetOrigin( "setOrigin", "v" );
const idEventDef EV_GetAngles( "getAngles", NULL, 'v' );
const idEventDef EV_SetAngles( "setAngles", "v" );
const idEventDef EV_GetLinearVelocity( "getLinearVelocity", NULL, 'v' );
const idEventDef EV_SetLinearVelocity( "setLinearVelocity", "v" );
const idEventDef EV_GetAngularVelocity( "getAngularVelocity", NULL, 'v' );
const idEventDef EV_SetAngularVelocity( "setAngularVelocity", "v" );
const idEventDef EV_GetSize( "getSize", NULL, 'v' );
const idEventDef EV_SetSize( "setSize", "vv" );
const idEventDef EV_GetMins( "getMins", NULL, 'v' );
const idEventDef EV_GetMaxs( "getMaxs", NULL, 'v' );
const idEventDef EV_IsHidden( "isHidden", NULL, 'd' );
const idEventDef EV_Hide( "hide", NULL );
const idEventDef EV_Show( "show", NULL );
const idEventDef EV_Touches( "touches", "E", 'd' );
const idEventDef EV_ClearSignal( "clearSignal", "d" );
const idEventDef EV_GetShaderParm( "getShaderParm", "d", 'f' );
const idEventDef EV_SetShaderParm( "setShaderParm", "df" );
const idEventDef EV_SetShaderParms( "setShaderParms", "ffff" );
const idEventDef EV_SetColor( "setColor", "fff" );
const idEventDef EV_GetColor( "getColor", NULL, 'v' );
const idEventDef EV_CacheSoundShader( "cacheSoundShader", "s" );
const idEventDef EV_StartSoundShader( "startSoundShader", "sd", 'f' );
const idEventDef EV_StartSound( "startSound", "sdd", 'f' );
const idEventDef EV_StopSound( "stopSound", "dd" );
const idEventDef EV_FadeSound( "fadeSound", "dff" );
const idEventDef EV_SetGuiParm( "setGuiParm", "ss" );
const idEventDef EV_SetGuiFloat( "setGuiFloat", "sf" );
const idEventDef EV_GetNextKey( "getNextKey", "ss", 's' );
const idEventDef EV_SetKey( "setKey", "ss" );
const idEventDef EV_GetKey( "getKey", "s", 's' );
const idEventDef EV_GetIntKey( "getIntKey", "s", 'f' );
const idEventDef EV_GetFloatKey( "getFloatKey", "s", 'f' );
const idEventDef EV_GetVectorKey( "getVectorKey", "s", 'v' );
const idEventDef EV_GetEntityKey( "getEntityKey", "s", 'e' );
const idEventDef EV_RestorePosition( "restorePosition" );
const idEventDef EV_UpdateCameraTarget( "<updateCameraTarget>", NULL );
const idEventDef EV_DistanceTo( "distanceTo", "E", 'f' );
const idEventDef EV_DistanceToPoint( "distanceToPoint", "v", 'f' );
const idEventDef EV_StartFx( "startFx", "s" );
const idEventDef EV_HasFunction( "hasFunction", "s", 'd' );
const idEventDef EV_CallFunction( "callFunction", "s" );
const idEventDef EV_SetNeverDormant( "setNeverDormant", "d" );

ABSTRACT_DECLARATION( idClass, idEntity )
	EVENT( EV_GetName,				idEntity::Event_GetName )
	EVENT( EV_SetName,				idEntity::Event_SetName )
	EVENT( EV_FindTargets,			idEntity::Event_FindTargets )
	EVENT( EV_ActivateTargets,		idEntity::Event_ActivateTargets )
	EVENT( EV_NumTargets,			idEntity::Event_NumTargets )
	EVENT( EV_GetTarget,			idEntity::Event_GetTarget )
	EVENT( EV_RandomTarget,			idEntity::Event_RandomTarget )
	EVENT( EV_BindToJoint,			idEntity::Event_BindToJoint )
	EVENT( EV_RemoveBinds,			idEntity::Event_RemoveBinds )
	EVENT( EV_Bind,					idEntity::Event_Bind )
	EVENT( EV_BindPosition,			idEntity::Event_BindPosition )
	EVENT( EV_Unbind,				idEntity::Event_Unbind )
	EVENT( EV_SpawnBind,			idEntity::Event_SpawnBind )
	EVENT( EV_SetOwner,				idEntity::Event_SetOwner )
	EVENT( EV_SetModel,				idEntity::Event_SetModel )
	EVENT( EV_SetSkin,				idEntity::Event_SetSkin )
	EVENT( EV_GetShaderParm,		idEntity::Event_GetShaderParm )
	EVENT( EV_SetShaderParm,		idEntity::Event_SetShaderParm )
	EVENT( EV_SetShaderParms,		idEntity::Event_SetShaderParms )
	EVENT( EV_SetColor,				idEntity::Event_SetColor )
	EVENT( EV_GetColor,				idEntity::Event_GetColor )
	EVENT( EV_IsHidden,				idEntity::Event_IsHidden )
	EVENT( EV_Hide,					idEntity::Event_Hide )
	EVENT( EV_Show,					idEntity::Event_Show )
	EVENT( EV_CacheSoundShader,		idEntity::Event_CacheSoundShader )
	EVENT( EV_StartSoundShader,		idEntity::Event_StartSoundShader )
	EVENT( EV_StartSound,			idEntity::Event_StartSound )
	EVENT( EV_StopSound,			idEntity::Event_StopSound )
	EVENT( EV_FadeSound,			idEntity::Event_FadeSound )
	EVENT( EV_GetWorldOrigin,		idEntity::Event_GetWorldOrigin )
	EVENT( EV_SetWorldOrigin,		idEntity::Event_SetWorldOrigin )
	EVENT( EV_GetOrigin,			idEntity::Event_GetOrigin )
	EVENT( EV_SetOrigin,			idEntity::Event_SetOrigin )
	EVENT( EV_GetAngles,			idEntity::Event_GetAngles )
	EVENT( EV_SetAngles,			idEntity::Event_SetAngles )
	EVENT( EV_GetLinearVelocity,	idEntity::Event_GetLinearVelocity )
	EVENT( EV_SetLinearVelocity,	idEntity::Event_SetLinearVelocity )
	EVENT( EV_GetAngularVelocity,	idEntity::Event_GetAngularVelocity )
	EVENT( EV_SetAngularVelocity,	idEntity::Event_SetAngularVelocity )
	EVENT( EV_GetSize,				idEntity::Event_GetSize )
	EVENT( EV_SetSize,				idEntity::Event_SetSize )
	EVENT( EV_GetMins,				idEntity::Event_GetMins)
	EVENT( EV_GetMaxs,				idEntity::Event_GetMaxs )
	EVENT( EV_Touches,				idEntity::Event_Touches )
	EVENT( EV_SetGuiParm, 			idEntity::Event_SetGuiParm )
	EVENT( EV_SetGuiFloat, 			idEntity::Event_SetGuiFloat )
	EVENT( EV_GetNextKey,			idEntity::Event_GetNextKey )
	EVENT( EV_SetKey,				idEntity::Event_SetKey )
	EVENT( EV_GetKey,				idEntity::Event_GetKey )
	EVENT( EV_GetIntKey,			idEntity::Event_GetIntKey )
	EVENT( EV_GetFloatKey,			idEntity::Event_GetFloatKey )
	EVENT( EV_GetVectorKey,			idEntity::Event_GetVectorKey )
	EVENT( EV_GetEntityKey,			idEntity::Event_GetEntityKey )
	EVENT( EV_RestorePosition,		idEntity::Event_RestorePosition )
	EVENT( EV_UpdateCameraTarget,	idEntity::Event_UpdateCameraTarget )
	EVENT( EV_DistanceTo,			idEntity::Event_DistanceTo )
	EVENT( EV_DistanceToPoint,		idEntity::Event_DistanceToPoint )
	EVENT( EV_StartFx,				idEntity::Event_StartFx )
	EVENT( EV_Thread_WaitFrame,		idEntity::Event_WaitFrame )
	EVENT( EV_Thread_Wait,			idEntity::Event_Wait )
	EVENT( EV_HasFunction,			idEntity::Event_HasFunction )
	EVENT( EV_CallFunction,			idEntity::Event_CallFunction )
	EVENT( EV_SetNeverDormant,		idEntity::Event_SetNeverDormant )
END_CLASS

void idEntity::Event_GetName() {
	idThread::ReturnString(NativeEvent_GetName());
}

void idEntity::Event_SetName(const char* name) {
	NativeEvent_SetName(name);
}

void idEntity::Event_FindTargets() {
	NativeEvent_FindTargets();
}

void idEntity::Event_ActivateTargets(idEntity* activator) {
	NativeEvent_ActivateTargets(activator);
}

void idEntity::Event_NumTargets() {
	idThread::ReturnInt(NativeEvent_NumTargets());
}

void idEntity::Event_GetTarget(float index) {
	idThread::ReturnEntity(NativeEvent_GetTarget(index));
}

void idEntity::Event_RandomTarget(const char* ignore) {
	idThread::ReturnEntity(NativeEvent_RandomTarget(ignore));
}

void idEntity::Event_BindToJoint(idEntity* master, const char* jointname, float orientated) {
	NativeEvent_BindToJoint(master, jointname, orientated);
}

void idEntity::Event_RemoveBinds() {
	NativeEvent_RemoveBinds();
}

void idEntity::Event_Bind(idEntity* master) {
	NativeEvent_Bind(master);
}

void idEntity::Event_BindPosition(idEntity* master) {
	NativeEvent_BindPosition(master);
}

void idEntity::Event_Unbind() {
	NativeEvent_Unbind();
}

void idEntity::Event_SpawnBind() {
	NativeEvent_SpawnBind();
}

void idEntity::Event_SetOwner(idEntity* owner) {
	NativeEvent_SetOwner(owner);
}

void idEntity::Event_SetModel(const char* modelname) {
	NativeEvent_SetModel(modelname);
}

void idEntity::Event_SetSkin(const char* skinname) {
	NativeEvent_SetSkin(skinname);
}

void idEntity::Event_GetShaderParm(int parmnum) {
	idThread::ReturnFloat(NativeEvent_GetShaderParm(parmnum));
}

void idEntity::Event_SetShaderParm(int parmnum, float value) {
	NativeEvent_SetShaderParm(parmnum, value);
}

void idEntity::Event_SetShaderParms(float parm0, float parm1, float parm2, float parm3) {
	NativeEvent_SetShaderParms(parm0, parm1, parm2, parm3);
}

void idEntity::Event_SetColor(float red, float green, float blue) {
	NativeEvent_SetColor(red, green, blue);
}

void idEntity::Event_GetColor() {
	idThread::ReturnVector(NativeEvent_GetColor());
}

void idEntity::Event_IsHidden() {
	idThread::ReturnInt(NativeEvent_IsHidden());
}

void idEntity::Event_Hide() {
	NativeEvent_Hide();
}

void idEntity::Event_Show() {
	NativeEvent_Show();
}

void idEntity::Event_CacheSoundShader(const char* soundName) {
	NativeEvent_CacheSoundShader(soundName);
}

void idEntity::Event_StartSoundShader(const char* shaderName, int channel) {
	idThread::ReturnInt(NativeEvent_StartSoundShader(shaderName, channel));
}

void idEntity::Event_StartSound(const char* soundName, int channel, int netSync) {
	idThread::ReturnInt(NativeEvent_StartSound(soundName, channel, netSync));
}

void idEntity::Event_StopSound(int channel, int netSync) {
	NativeEvent_StopSound(channel, netSync);
}

void idEntity::Event_FadeSound(int channel, float to, float over) {
	NativeEvent_FadeSound(channel, to, over);
}

void idEntity::Event_GetWorldOrigin() {
	idThread::ReturnVector(NativeEvent_GetWorldOrigin());
}

void idEntity::Event_SetWorldOrigin(const idVec3& pos) {
	NativeEvent_SetWorldOrigin(pos);
}

void idEntity::Event_GetOrigin() {
	idThread::ReturnVector(NativeEvent_GetOrigin());
}

void idEntity::Event_SetOrigin(const idVec3& pos) {
	NativeEvent_SetOrigin(pos);
}

void idEntity::Event_GetAngles() {
	idAngles ang = GetPhysics()->GetAxis().ToAngles();
	idThread::ReturnVector(idVec3(ang[0], ang[1], ang[2]));
}

void idEntity::Event_SetAngles(const idAngles& ang) {
	NativeEvent_SetAngles(ang);
}

void idEntity::Event_GetLinearVelocity() {
	idThread::ReturnVector(NativeEvent_GetLinearVelocity());
}

void idEntity::Event_SetLinearVelocity(const idVec3& velocity) {
	NativeEvent_SetLinearVelocity(velocity);
}

void idEntity::Event_GetAngularVelocity() {
	idThread::ReturnVector(NativeEvent_GetAngularVelocity());
}

void idEntity::Event_SetAngularVelocity(const idVec3& velocity) {
	NativeEvent_SetAngularVelocity(velocity);
}

void idEntity::Event_GetSize() {
	idBounds bounds;

	bounds = GetPhysics()->GetBounds();
	idThread::ReturnVector(bounds[1] - bounds[0]);
}

void idEntity::Event_SetSize(const idVec3& mins, const idVec3& maxs) {
	NativeEvent_SetSize(mins, maxs);
}

void idEntity::Event_GetMins() {
	idThread::ReturnVector(NativeEvent_GetMins());
}

void idEntity::Event_GetMaxs() {
	idThread::ReturnVector(NativeEvent_GetMaxs());
}

void idEntity::Event_Touches(idEntity* ent) {
	idThread::ReturnInt(NativeEvent_Touches(ent));
}

void idEntity::Event_SetGuiParm(const char* key, const char* val) {
	NativeEvent_SetGuiParm(key, val);
}

void idEntity::Event_SetGuiFloat(const char* key, float val) {
	NativeEvent_SetGuiFloat(key, val);
}

void idEntity::Event_GetNextKey(const char* prefix, const char* lastMatch) {
	idThread::ReturnString(NativeEvent_GetNextKey(prefix, lastMatch));
}

void idEntity::Event_SetKey(const char* key, const char* value) {
	NativeEvent_SetKey(key, value);
}

void idEntity::Event_GetKey(const char* key) {
	idThread::ReturnString(NativeEvent_GetKey(key));
}

void idEntity::Event_GetIntKey(const char* key) {
	idThread::ReturnInt(NativeEvent_GetIntKey(key));
}

void idEntity::Event_GetFloatKey(const char* key) {
	idThread::ReturnFloat(NativeEvent_GetFloatKey(key));
}

void idEntity::Event_GetVectorKey(const char* key) {
	idThread::ReturnVector(NativeEvent_GetVectorKey(key));
}

void idEntity::Event_GetEntityKey(const char* key) {
	idThread::ReturnEntity(NativeEvent_GetEntityKey(key));
}

void idEntity::Event_RestorePosition() {
	NativeEvent_RestorePosition();
}

void idEntity::Event_UpdateCameraTarget() {
	NativeEvent_UpdateCameraTarget();
}

void idEntity::Event_DistanceTo(idEntity* ent) {
	idThread::ReturnFloat(NativeEvent_DistanceTo(ent));
}

void idEntity::Event_DistanceToPoint(const idVec3& point) {
	idThread::ReturnFloat(NativeEvent_DistanceToPoint(point));
}

void idEntity::Event_StartFx(const char* fxName) {
	NativeEvent_StartFx(fxName);
}

void idEntity::Event_WaitFrame() {
	idThread* thread;

	thread = idThread::CurrentThread();
	if (thread) {
		thread->WaitFrame();
	}
}

void idEntity::Event_Wait(float time) {
	idThread* thread = idThread::CurrentThread();

	if (!thread) {
		gameLocal.Error("Event 'wait' called from outside thread");
	}

	thread->WaitSec(time);
}

void idEntity::Event_HasFunction(const char* name) {
	idThread::ReturnInt(NativeEvent_HasFunction(name));
}

void idEntity::Event_CallFunction(const char* name) {
	NativeEvent_CallFunction(name);
}

void idEntity::Event_SetNeverDormant(int enable) {
	NativeEvent_SetNeverDormant(enable);
}
