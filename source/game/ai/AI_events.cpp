/*
===========================================================================

IcedTech
(c) 2024 by Justin Marshall - icecolddukde

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

#include "precompiled.h"
#pragma hdrstop

#include "../Game_local.h"

idActor* idAI::FindEnemy(int useFOV) {
	int i;
	idEntity* ent;
	idActor* actor;

	if (gameLocal.InPlayerPVS(this)) {
		for (i = 0; i < gameLocal.numClients; i++) {
			ent = gameLocal.entities[i];

			if (!ent || !ent->IsType(idActor::GetClassType())) {
				continue;
			}

			actor = static_cast<idActor*>(ent);
			if ((actor->health <= 0) || !(ReactionTo(actor) & ATTACK_ON_SIGHT)) {
				continue;
			}

			if (CanSee(actor, useFOV != 0)) {
				return actor;
			}
		}
	}

	return NULL;
}

idActor* idAI::FindEnemyAI(int useFOV) {
	idEntity* ent;
	idActor* actor;
	idActor* bestEnemy;
	float bestDist;
	float dist;
	idVec3 delta;
	pvsHandle_t pvs;

	pvs = gameLocal.pvs.SetupCurrentPVS(GetPVSAreas(), GetNumPVSAreas());

	bestDist = idMath::INFINITY;
	bestEnemy = NULL;
	for (ent = gameLocal.activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next()) {
		if (ent->fl.hidden || ent->fl.isDormant || !ent->IsType(idActor::GetClassType())) {
			continue;
		}

		actor = static_cast<idActor*>(ent);
		if ((actor->health <= 0) || !(ReactionTo(actor) & ATTACK_ON_SIGHT)) {
			continue;
		}

		if (!gameLocal.pvs.InCurrentPVS(pvs, actor->GetPVSAreas(), actor->GetNumPVSAreas())) {
			continue;
		}

		delta = physicsObj.GetOrigin() - actor->GetPhysics()->GetOrigin();
		dist = delta.LengthSqr();
		if ((dist < bestDist) && CanSee(actor, useFOV != 0)) {
			bestDist = dist;
			bestEnemy = actor;
		}
	}

	gameLocal.pvs.FreeCurrentPVS(pvs);
	return bestEnemy;
}

idActor* idAI::FindEnemyInCombatNodes(void) {
	int i, j;
	idCombatNode* node;
	idEntity* ent;
	idEntity* targetEnt;
	idActor* actor;

	if (!gameLocal.InPlayerPVS(this)) {
		return NULL;
	}

	for (i = 0; i < gameLocal.numClients; i++) {
		ent = gameLocal.entities[i];

		if (!ent || !ent->IsType(idActor::GetClassType())) {
			continue;
		}

		actor = static_cast<idActor*>(ent);
		if ((actor->health <= 0) || !(ReactionTo(actor) & ATTACK_ON_SIGHT)) {
			continue;
		}

		for (j = 0; j < targets.Num(); j++) {
			targetEnt = targets[j].GetEntity();
			if (!targetEnt || !targetEnt->IsType(idCombatNode::GetClassType())) {
				continue;
			}

			node = static_cast<idCombatNode*>(targetEnt);
			if (!node->IsDisabled() && node->EntityInView(actor, actor->GetPhysics()->GetOrigin())) {
				return actor;
			}
		}
	}

	return NULL;
}

idActor* idAI::ClosestReachableEnemyOfEntity(idEntity* team_mate) {
	idActor* actor;
	idActor* ent;
	idActor* bestEnt;
	float bestDistSquared;
	float distSquared;
	idVec3 delta;
	int areaNum;
	int enemyAreaNum;
	aasPath_t path;

	if (!team_mate->IsType(idActor::GetClassType())) {
		gameLocal.Error("Entity '%s' is not an AI character or player", team_mate->GetName());
	}

	actor = static_cast<idActor*>(team_mate);

	const idVec3& origin = physicsObj.GetOrigin();
	areaNum = PointReachableAreaNum(origin);

	bestDistSquared = idMath::INFINITY;
	bestEnt = NULL;
	for (ent = actor->enemyList.Next(); ent != NULL; ent = ent->enemyNode.Next()) {
		if (ent->fl.hidden) {
			continue;
		}
		delta = ent->GetPhysics()->GetOrigin() - origin;
		distSquared = delta.LengthSqr();
		if (distSquared < bestDistSquared) {
			const idVec3& enemyPos = ent->GetPhysics()->GetOrigin();
			enemyAreaNum = PointReachableAreaNum(enemyPos);
			if ((areaNum != 0) && PathToGoal(path, areaNum, origin, enemyAreaNum, enemyPos)) {
				bestEnt = ent;
				bestDistSquared = distSquared;
			}
		}
	}

	return bestEnt;
}

idActor* idAI::HeardSound(int ignore_team) {
	idActor* actor = gameLocal.GetAlertEntity();
	if (actor && (!ignore_team || (ReactionTo(actor) & ATTACK_ON_SIGHT)) && gameLocal.InPlayerPVS(this)) {
		idVec3 pos = actor->GetPhysics()->GetOrigin();
		idVec3 org = physicsObj.GetOrigin();
		float dist = (pos - org).LengthSqr();
		if (dist < Square(AI_HEARING_RANGE)) {
			return actor;
		}
	}

	return NULL;
}

void idAI::SetEnemy(idEntity* ent) {
	if (!ent) {
		ClearEnemy();
	}
	else if (!ent->IsType(idActor::GetClassType())) {
		gameLocal.Error("'%s' is not an idActor (player or ai controlled character)", ent->name.c_str());
	}
	else {
		SetEnemy(static_cast<idActor*>(ent));
	}
}

void idAI::MuzzleFlash(const char* jointname) {
	idVec3 muzzle;
	idMat3 axis;

	GetMuzzle(jointname, muzzle, axis);
	TriggerWeaponEffects(muzzle);
}

idEntity* idAI::CreateMissile(const char* jointname) {
	idVec3 muzzle;
	idMat3 axis;

	if (!projectileDef) {
		gameLocal.Warning("%s (%s) doesn't have a projectile specified", name.c_str(), GetEntityDefName());
		return NULL;
	}

	GetMuzzle(jointname, muzzle, axis);
	CreateProjectile(muzzle, viewAxis[0] * physicsObj.GetGravityAxis());
	if (projectile.GetEntity()) {
		if (!jointname || !jointname[0]) {
			projectile.GetEntity()->Bind(this, true);
		}
		else {
			projectile.GetEntity()->BindToJoint(this, jointname, true);
		}
	}
	return projectile.GetEntity();
}

idProjectile* idAI::AttackMissile(const char* jointname) {
	return LaunchProjectile(jointname, enemy.GetEntity(), true);
}

idProjectile* idAI::FireMissileAtTarget(const char* jointname, const char* targetname) {
	idEntity* aent = gameLocal.FindEntity(targetname);
	if (!aent) {
		gameLocal.Warning("Entity '%s' not found for 'fireMissileAtTarget'", targetname);
	}
	return LaunchProjectile(jointname, aent, false);
}


idEntity* idAI::LaunchMissile(const idVec3& org, const idAngles& ang) {
	idVec3 start;
	trace_t tr;
	idBounds projBounds;
	const idClipModel* projClip;
	idMat3 axis;
	float distance;

	if (!projectileDef) {
		gameLocal.Warning("%s (%s) doesn't have a projectile specified", name.c_str(), GetEntityDefName());
		return NULL;
	}

	axis = ang.ToMat3();
	if (!projectile.GetEntity()) {
		CreateProjectile(org, axis[0]);
	}

	const idBounds& ownerBounds = physicsObj.GetAbsBounds();
	projClip = projectile.GetEntity()->GetPhysics()->GetClipModel();
	projBounds = projClip->GetBounds().Rotate(projClip->GetAxis());

	if (((ownerBounds[1][0] - ownerBounds[0][0]) > (projBounds[1][0] - projBounds[0][0])) &&
		((ownerBounds[1][1] - ownerBounds[0][1]) > (projBounds[1][1] - projBounds[0][1])) &&
		((ownerBounds[1][2] - ownerBounds[0][2]) > (projBounds[1][2] - projBounds[0][2]))) {
		if ((ownerBounds - projBounds).RayIntersection(org, viewAxis[0], distance)) {
			start = org + distance * viewAxis[0];
		}
		else {
			start = ownerBounds.GetCenter();
		}
	}
	else {
		start = ownerBounds.GetCenter();
	}

	gameLocal.clip.Translation(tr, start, org, projClip, projClip->GetAxis(), MASK_SHOT_RENDERMODEL, this);

	projectile.GetEntity()->Launch(tr.endpos, axis[0], vec3_origin);
	projectile = NULL;

	TriggerWeaponEffects(tr.endpos);

	lastAttackTime = gameLocal.time;

	return projectile.GetEntity();
}

void idAI::DirectDamage(idEntity* damageTarget, const char* damageDefName) {
	DirectDamage(damageDefName, damageTarget);
}

void idAI::RadiusDamageFromJoint(const char* jointname, const char* damageDefName) {
	jointHandle_t joint;
	idVec3 org;
	idMat3 axis;

	if (!jointname || !jointname[0]) {
		org = physicsObj.GetOrigin();
	}
	else {
		joint = animator.GetJointHandle(jointname);
		if (joint == INVALID_JOINT) {
			gameLocal.Error("Unknown joint '%s' on %s", jointname, GetEntityDefName());
		}
		GetJointWorldTransform(joint, gameLocal.time, org, axis);
	}

	gameLocal.RadiusDamage(org, this, this, this, this, damageDefName);
}

idPathCorner* idAI::RandomPath(void) {
	return idPathCorner::RandomPath(this, NULL);
}

bool idAI::CanBecomeSolid(void) {
	int i;
	int num;
	idEntity* hit;
	idClipModel* cm;
	idClipModel* clipModels[MAX_GENTITIES];

	num = gameLocal.clip.ClipModelsTouchingBounds(physicsObj.GetAbsBounds(), MASK_MONSTERSOLID, clipModels, MAX_GENTITIES);
	for (i = 0; i < num; i++) {
		cm = clipModels[i];

		if (cm->IsRenderModel()) {
			continue;
		}

		hit = cm->GetEntity();
		if ((hit == this) || !hit->fl.takedamage) {
			continue;
		}

		if (physicsObj.ClipContents(cm)) {
			return false;
		}
	}

	return true;
}

void idAI::BecomeSolid(void) {
	physicsObj.EnableClip();
	if (spawnArgs.GetBool("big_monster")) {
		physicsObj.SetContents(0);
	}
	else if (use_combat_bbox) {
		physicsObj.SetContents(CONTENTS_BODY | CONTENTS_SOLID);
	}
	else {
		physicsObj.SetContents(CONTENTS_BODY);
	}
	physicsObj.GetClipModel()->Link(gameLocal.clip);
	fl.takedamage = !spawnArgs.GetBool("noDamage");
}

void idAI::BecomeNonSolid(void) {
	fl.takedamage = false;
	physicsObj.SetContents(0);
	physicsObj.GetClipModel()->Unlink();
}

bool idAI::BecomeRagdoll(void) {
	return StartRagdoll();
}

void idAI::StopRagdoll(void) {
	idActor::StopRagdoll();
	SetPhysics(&physicsObj);
}

void idAI::SetHealth(float newHealth) {
	health = newHealth;
	fl.takedamage = true;
	AI_DEAD = (health <= 0);
}

float idAI::GetHealth(void) {
	return health;
}

void idAI::AllowDamage(void) {
	fl.takedamage = true;
}

void idAI::IgnoreDamage(void) {
	fl.takedamage = false;
}

float idAI::GetCurrentYaw(void) {
	return current_yaw;
}

void idAI::TurnTo(float angle) {
	TurnToward(angle);
}

void idAI::TurnToPos(const idVec3& pos) {
	TurnToward(pos);
}

void idAI::TurnToEntity(idEntity* ent) {
	if (ent) {
		TurnToward(ent->GetPhysics()->GetOrigin());
	}
}

int idAI::MoveStatus(void) {
	return move.moveStatus;
}

void idAI::StopMove(void) {
	StopMove(MOVE_STATUS_DONE);
}

void idAI::MoveToCover(void) {
	idActor* enemyEnt = enemy.GetEntity();
	StopMove(MOVE_STATUS_DEST_NOT_FOUND);
	if (enemyEnt && MoveToCover(enemyEnt, lastVisibleEnemyPos)) {
		return;
	}
}

void idAI::MoveToAttackPosition(idEntity* entity, const char* attack_anim) {
	int anim;
	StopMove(MOVE_STATUS_DEST_NOT_FOUND);

	anim = GetAnim(ANIMCHANNEL_LEGS, attack_anim);
	if (!anim) {
		gameLocal.Error("Unknown anim '%s'", attack_anim);
	}

	MoveToAttackPosition(entity, anim);
}

void idAI::SlideTo(const idVec3& pos, float time) {
	SlideToPosition(pos, time);
}

void idAI::Wander(void) {
	WanderAround();
}

idCombatNode* idAI::GetCombatNode(void) {
	int i;
	float dist;
	idEntity* targetEnt;
	idCombatNode* node;
	float bestDist;
	idCombatNode* bestNode;
	idActor* enemyEnt = enemy.GetEntity();

	if (!targets.Num()) {
		return NULL;
	}

	if (!enemyEnt || !EnemyPositionValid()) {
		return NULL;
	}

	bestNode = NULL;
	const idVec3& myPos = physicsObj.GetOrigin();
	bestDist = (myPos - lastVisibleEnemyPos).LengthSqr();
	for (i = 0; i < targets.Num(); i++) {
		targetEnt = targets[i].GetEntity();
		if (!targetEnt || !targetEnt->IsType(idCombatNode::GetClassType())) {
			continue;
		}

		node = static_cast<idCombatNode*>(targetEnt);
		if (!node->IsDisabled() && node->EntityInView(enemyEnt, lastVisibleEnemyPos)) {
			idVec3 org = node->GetPhysics()->GetOrigin();
			dist = (myPos - org).LengthSqr();
			if (dist < bestDist) {
				bestNode = node;
				bestDist = dist;
			}
		}
	}

	return bestNode;
}

bool idAI::EnemyInCombatCone(idEntity* ent, int use_current_enemy_location) {
	idCombatNode* node;
	bool result;
	idActor* enemyEnt = enemy.GetEntity();

	if (!targets.Num()) {
		return false;
	}

	if (!enemyEnt) {
		return false;
	}

	if (!ent || !ent->IsType(idCombatNode::GetClassType())) {
		return false;
	}

	node = static_cast<idCombatNode*>(ent);
	if (use_current_enemy_location) {
		const idVec3& pos = enemyEnt->GetPhysics()->GetOrigin();
		result = node->EntityInView(enemyEnt, pos);
	}
	else {
		result = node->EntityInView(enemyEnt, lastVisibleEnemyPos);
	}

	return result;
}

idVec3 idAI::GetJumpVelocity(const idVec3& pos, float speed, float max_height) {
	idVec3 start;
	idVec3 end;
	idVec3 dir;
	float dist;
	bool result;
	idEntity* enemyEnt = enemy.GetEntity();

	if (!enemyEnt) {
		return vec3_zero;
	}

	if (speed <= 0.0f) {
		gameLocal.Error("Invalid speed.  speed must be > 0.");
	}

	start = physicsObj.GetOrigin();
	end = pos;
	dir = end - start;
	dist = dir.Normalize();
	if (dist > 16.0f) {
		dist -= 16.0f;
		end -= dir * 16.0f;
	}

	result = PredictTrajectory(start, end, speed, physicsObj.GetGravity(), physicsObj.GetClipModel(), MASK_MONSTERSOLID, max_height, this, enemyEnt, ai_debugMove.GetBool() ? 4000 : 0, dir);
	if (result) {
		return dir * speed;
	}
	else {
		return vec3_zero;
	}
}

bool idAI::EntityInAttackCone(idEntity* ent) {
	float attack_cone;
	idVec3 delta;
	float yaw;
	float relYaw;

	if (!ent) {
		return false;
	}

	delta = ent->GetPhysics()->GetOrigin() - GetEyePosition();

	const idVec3& gravityDir = GetPhysics()->GetGravityNormal();

	delta -= gravityDir * (gravityDir * delta);

	delta.Normalize();
	yaw = delta.ToYaw();

	attack_cone = spawnArgs.GetFloat("attack_cone", "70");
	relYaw = idMath::AngleNormalize180(ideal_yaw - yaw);
	return (idMath::Fabs(relYaw) < (attack_cone * 0.5f));
}

bool idAI::CanSeeEntity(idEntity* ent) {
	return (ent && CanSee(ent, false));
}

void idAI::SetTalkTarget(idEntity* target) {
	if (target && !target->IsType(idActor::GetClassType())) {
		gameLocal.Error("Cannot set talk target to '%s'.  Not a character or player.", target->GetName());
	}
	talkTarget = static_cast<idActor*>(target);
	AI_TALK = (target != NULL);
}

idEntity* idAI::GetTalkTarget(void) {
	return talkTarget.GetEntity();
}

void idAI::SetTalkState(int state) {
	if ((state < 0) || (state >= NUM_TALK_STATES)) {
		gameLocal.Error("Invalid talk state (%d)", state);
	}

	talk_state = static_cast<talkState_t>(state);
}

float idAI::EnemyRange(void) {
	idActor* enemyEnt = enemy.GetEntity();
	if (enemyEnt) {
		return (enemyEnt->GetPhysics()->GetOrigin() - GetPhysics()->GetOrigin()).Length();
	}
	else {
		return idMath::INFINITY;
	}
}

float idAI::EnemyRange2D(void) {
	idActor* enemyEnt = enemy.GetEntity();
	if (enemyEnt) {
		return (enemyEnt->GetPhysics()->GetOrigin().ToVec2() - GetPhysics()->GetOrigin().ToVec2()).Length();
	}
	else {
		return idMath::INFINITY;
	}
}

idEntity* idAI::GetEnemy(void) {
	return enemy.GetEntity();
}

idVec3 idAI::GetEnemyPos(void) {
	return lastVisibleEnemyPos;
}

idVec3 idAI::GetEnemyEyePos(void) {
	return lastVisibleEnemyPos + lastVisibleEnemyEyeOffset;
}

idVec3 idAI::PredictEnemyPos(float time) {
	predictedPath_t path;
	idActor* enemyEnt = enemy.GetEntity();

	if (!enemyEnt) {
		return physicsObj.GetOrigin();
	}

	idAI::PredictPath(enemyEnt, aas, lastVisibleEnemyPos, enemyEnt->GetPhysics()->GetLinearVelocity(), SEC2MS(time), SEC2MS(time), (move.moveType == MOVETYPE_FLY) ? SE_BLOCKED : (SE_BLOCKED | SE_ENTER_LEDGE_AREA), path);

	return path.endPos;
}

bool idAI::CanHitEnemy(void) {
	trace_t tr;
	idEntity* hit;

	idActor* enemyEnt = enemy.GetEntity();
	if (!AI_ENEMY_VISIBLE || !enemyEnt) {
		return false;
	}

	if (gameLocal.time == lastHitCheckTime) {
		return lastHitCheckResult;
	}

	lastHitCheckTime = gameLocal.time;

	idVec3 toPos = enemyEnt->GetEyePosition();
	idVec3 eye = GetEyePosition();
	idVec3 dir;

	dir = toPos - eye;
	dir.Normalize();
	toPos = eye + dir * MAX_WORLD_SIZE;
	gameLocal.clip.TracePoint(tr, eye, toPos, MASK_SHOT_BOUNDINGBOX, this);
	hit = gameLocal.GetTraceEntity(tr);
	if (tr.fraction >= 1.0f || (hit == enemyEnt)) {
		lastHitCheckResult = true;
	}
	else if ((tr.fraction < 1.0f) && (hit->IsType(idAI::GetClassType())) &&
		(static_cast<idAI*>(hit)->team != team)) {
		lastHitCheckResult = true;
	}
	else {
		lastHitCheckResult = false;
	}

	return lastHitCheckResult;
}

bool idAI::CanHitEnemyFromAnim(const char* animname) {
	int anim;
	idVec3 dir;
	idVec3 local_dir;
	idVec3 fromPos;
	idMat3 axis;
	idVec3 start;
	trace_t tr;
	float distance;

	idActor* enemyEnt = enemy.GetEntity();
	if (!AI_ENEMY_VISIBLE || !enemyEnt) {
		return false;
	}

	anim = GetAnim(ANIMCHANNEL_LEGS, animname);
	if (!anim) {
		return false;
	}

	if (enemyEnt->GetPhysics()->GetAbsBounds().IntersectsBounds(physicsObj.GetAbsBounds().Expand(16.0f))) {
		return CanHitEnemy();
	}

	const idVec3& org = physicsObj.GetOrigin();
	dir = lastVisibleEnemyPos - org;
	physicsObj.GetGravityAxis().ProjectVector(dir, local_dir);
	local_dir.z = 0.0f;
	local_dir.ToVec2().Normalize();
	axis = local_dir.ToMat3();
	fromPos = physicsObj.GetOrigin() + missileLaunchOffset[anim] * axis;

	if (projectileClipModel == NULL) {
		CreateProjectileClipModel();
	}

	const idBounds& ownerBounds = physicsObj.GetAbsBounds();
	const idBounds& projBounds = projectileClipModel->GetBounds();
	if (((ownerBounds[1][0] - ownerBounds[0][0]) > (projBounds[1][0] - projBounds[0][0])) &&
		((ownerBounds[1][1] - ownerBounds[0][1]) > (projBounds[1][1] - projBounds[0][1])) &&
		((ownerBounds[1][2] - ownerBounds[0][2]) > (projBounds[1][2] - projBounds[0][2]))) {
		if ((ownerBounds - projBounds).RayIntersection(org, viewAxis[0], distance)) {
			start = org + distance * viewAxis[0];
		}
		else {
			start = ownerBounds.GetCenter();
		}
	}
	else {
		start = ownerBounds.GetCenter();
	}

	gameLocal.clip.Translation(tr, start, fromPos, projectileClipModel, mat3_identity, MASK_SHOT_RENDERMODEL, this);
	fromPos = tr.endpos;

	return GetAimDir(fromPos, enemy.GetEntity(), this, dir);
}


bool idAI::CanHitEnemyFromJoint(const char* jointname) {
	trace_t tr;
	idVec3 muzzle;
	idMat3 axis;
	idVec3 start;
	float distance;

	idActor* enemyEnt = enemy.GetEntity();
	if (!AI_ENEMY_VISIBLE || !enemyEnt) {
		return false;
	}

	if (gameLocal.time == lastHitCheckTime) {
		return lastHitCheckResult;
	}

	lastHitCheckTime = gameLocal.time;

	const idVec3& org = physicsObj.GetOrigin();
	idVec3 toPos = enemyEnt->GetEyePosition();
	jointHandle_t joint = animator.GetJointHandle(jointname);
	if (joint == INVALID_JOINT) {
		gameLocal.Error("Unknown joint '%s' on %s", jointname, GetEntityDefName());
	}
	animator.GetJointTransform(joint, gameLocal.time, muzzle, axis);
	muzzle = org + (muzzle + modelOffset) * viewAxis * physicsObj.GetGravityAxis();

	if (projectileClipModel == NULL) {
		CreateProjectileClipModel();
	}

	const idBounds& ownerBounds = physicsObj.GetAbsBounds();
	const idBounds& projBounds = projectileClipModel->GetBounds();
	if (((ownerBounds[1][0] - ownerBounds[0][0]) > (projBounds[1][0] - projBounds[0][0])) &&
		((ownerBounds[1][1] - ownerBounds[0][1]) > (projBounds[1][1] - projBounds[0][1])) &&
		((ownerBounds[1][2] - ownerBounds[0][2]) > (projBounds[1][2] - projBounds[0][2]))) {
		if ((ownerBounds - projBounds).RayIntersection(org, viewAxis[0], distance)) {
			start = org + distance * viewAxis[0];
		}
		else {
			start = ownerBounds.GetCenter();
		}
	}
	else {
		start = ownerBounds.GetCenter();
	}

	gameLocal.clip.Translation(tr, start, muzzle, projectileClipModel, mat3_identity, MASK_SHOT_BOUNDINGBOX, this);
	muzzle = tr.endpos;

	gameLocal.clip.Translation(tr, muzzle, toPos, projectileClipModel, mat3_identity, MASK_SHOT_BOUNDINGBOX, this);
	if (tr.fraction >= 1.0f || (gameLocal.GetTraceEntity(tr) == enemyEnt)) {
		lastHitCheckResult = true;
	}
	else {
		lastHitCheckResult = false;
	}

	return lastHitCheckResult;
}

void idAI::ChargeAttack(const char* damageDef) {
	idActor* enemyEnt = enemy.GetEntity();

	StopMove(MOVE_STATUS_DEST_NOT_FOUND);
	if (enemyEnt) {
		idVec3 enemyOrg;

		if (move.moveType == MOVETYPE_FLY) {
			enemyOrg = enemyEnt->GetEyePosition();
			enemyOrg -= enemyEnt->GetPhysics()->GetGravityNormal() * fly_offset;
		}
		else {
			enemyOrg = enemyEnt->GetPhysics()->GetOrigin();
		}

		BeginAttack(damageDef);
		DirectMoveToPosition(enemyOrg);
		TurnToward(enemyOrg);
	}
}

float idAI::TestChargeAttack(void) {
	trace_t trace;
	idActor* enemyEnt = enemy.GetEntity();
	predictedPath_t path;
	idVec3 end;

	if (!enemyEnt) {
		return 0.0f;
	}

	if (move.moveType == MOVETYPE_FLY) {
		end = enemyEnt->GetEyePosition();
		end -= enemyEnt->GetPhysics()->GetGravityNormal() * fly_offset;
	}
	else {
		end = enemyEnt->GetPhysics()->GetOrigin();
	}

	idAI::PredictPath(this, aas, physicsObj.GetOrigin(), end - physicsObj.GetOrigin(), 1000, 1000, (move.moveType == MOVETYPE_FLY) ? SE_BLOCKED : (SE_ENTER_OBSTACLE | SE_BLOCKED | SE_ENTER_LEDGE_AREA), path);

	if (ai_debugMove.GetBool()) {
		gameRenderWorld->DebugLine(colorGreen, physicsObj.GetOrigin(), end, gameLocal.msec);
		gameRenderWorld->DebugBounds(path.endEvent == 0 ? colorYellow : colorRed, physicsObj.GetBounds(), end, gameLocal.msec);
	}

	if ((path.endEvent == 0) || (path.blockingEntity == enemyEnt)) {
		idVec3 delta = end - physicsObj.GetOrigin();
		float time = delta.LengthFast();
		return time;
	}
	else {
		return 0.0f;
	}
}

bool idAI::TestAnimMoveTowardEnemy(const char* animname) {
	int anim;
	predictedPath_t path;
	idVec3 moveVec;
	float yaw;
	idVec3 delta;
	idActor* enemyEnt;

	enemyEnt = enemy.GetEntity();
	if (!enemyEnt) {
		return false;
	}

	anim = GetAnim(ANIMCHANNEL_LEGS, animname);
	if (!anim) {
		gameLocal.DWarning("missing '%s' animation on '%s' (%s)", animname, name.c_str(), GetEntityDefName());
		return false;
	}

	delta = enemyEnt->GetPhysics()->GetOrigin() - physicsObj.GetOrigin();
	yaw = delta.ToYaw();

	moveVec = animator.TotalMovementDelta(anim) * idAngles(0.0f, yaw, 0.0f).ToMat3() * physicsObj.GetGravityAxis();
	idAI::PredictPath(this, aas, physicsObj.GetOrigin(), moveVec, 1000, 1000, (move.moveType == MOVETYPE_FLY) ? SE_BLOCKED : (SE_ENTER_OBSTACLE | SE_BLOCKED | SE_ENTER_LEDGE_AREA), path);

	if (ai_debugMove.GetBool()) {
		gameRenderWorld->DebugLine(colorGreen, physicsObj.GetOrigin(), physicsObj.GetOrigin() + moveVec, gameLocal.msec);
		gameRenderWorld->DebugBounds(path.endEvent == 0 ? colorYellow : colorRed, physicsObj.GetBounds(), physicsObj.GetOrigin() + moveVec, gameLocal.msec);
	}

	return (path.endEvent == 0);
}

bool idAI::TestAnimMove(const char* animname) {
	int anim;
	predictedPath_t path;
	idVec3 moveVec;

	anim = GetAnim(ANIMCHANNEL_LEGS, animname);
	if (!anim) {
		gameLocal.DWarning("missing '%s' animation on '%s' (%s)", animname, name.c_str(), GetEntityDefName());
		return false;
	}

	moveVec = animator.TotalMovementDelta(anim) * idAngles(0.0f, ideal_yaw, 0.0f).ToMat3() * physicsObj.GetGravityAxis();
	idAI::PredictPath(this, aas, physicsObj.GetOrigin(), moveVec, 1000, 1000, (move.moveType == MOVETYPE_FLY) ? SE_BLOCKED : (SE_ENTER_OBSTACLE | SE_BLOCKED | SE_ENTER_LEDGE_AREA), path);

	if (ai_debugMove.GetBool()) {
		gameRenderWorld->DebugLine(colorGreen, physicsObj.GetOrigin(), physicsObj.GetOrigin() + moveVec, gameLocal.msec);
		gameRenderWorld->DebugBounds(path.endEvent == 0 ? colorYellow : colorRed, physicsObj.GetBounds(), physicsObj.GetOrigin() + moveVec, gameLocal.msec);
	}

	return (path.endEvent == 0);
}

bool idAI::TestMoveToPosition(const idVec3& position) {
	predictedPath_t path;

	idAI::PredictPath(this, aas, physicsObj.GetOrigin(), position - physicsObj.GetOrigin(), 1000, 1000, (move.moveType == MOVETYPE_FLY) ? SE_BLOCKED : (SE_ENTER_OBSTACLE | SE_BLOCKED | SE_ENTER_LEDGE_AREA), path);

	if (ai_debugMove.GetBool()) {
		gameRenderWorld->DebugLine(colorGreen, physicsObj.GetOrigin(), position, gameLocal.msec);
		gameRenderWorld->DebugBounds(colorYellow, physicsObj.GetBounds(), position, gameLocal.msec);
		if (path.endEvent) {
			gameRenderWorld->DebugBounds(colorRed, physicsObj.GetBounds(), path.endPos, gameLocal.msec);
		}
	}

	return (path.endEvent == 0);
}

bool idAI::TestMeleeAttack(void) {
	return TestMelee();
}

bool idAI::TestAnimAttack(const char* animname) {
	int anim;
	predictedPath_t path;

	anim = GetAnim(ANIMCHANNEL_LEGS, animname);
	if (!anim) {
		gameLocal.DWarning("missing '%s' animation on '%s' (%s)", animname, name.c_str(), GetEntityDefName());
		return false;
	}

	idAI::PredictPath(this, aas, physicsObj.GetOrigin(), animator.TotalMovementDelta(anim), 1000, 1000, (move.moveType == MOVETYPE_FLY) ? SE_BLOCKED : (SE_ENTER_OBSTACLE | SE_BLOCKED | SE_ENTER_LEDGE_AREA), path);

	return (path.blockingEntity && (path.blockingEntity == enemy.GetEntity()));
}

void idAI::PreBurn(void) {
	renderEntity.noShadow = true;
}

void idAI::Burn(void) {
	renderEntity.shaderParms[SHADERPARM_TIME_OF_DEATH] = gameLocal.time * 0.001f;
	SpawnParticles("smoke_burnParticleSystem");
	UpdateVisuals();
}

void idAI::ClearBurn(void) {
	renderEntity.noShadow = spawnArgs.GetBool("noshadows");
	renderEntity.shaderParms[SHADERPARM_TIME_OF_DEATH] = 0.0f;
	UpdateVisuals();
}


void idAI::SetSmokeVisibility(int num, int on) {
	int i;
	int time;

	if (num >= particles.Num()) {
		gameLocal.Warning("Particle #%d out of range (%d particles) on entity '%s'", num, particles.Num(), name.c_str());
		return;
	}

	if (on != 0) {
		time = gameLocal.time;
		BecomeActive(TH_UPDATEPARTICLES);
	}
	else {
		time = 0;
	}

	if (num >= 0) {
		particles[num].time = time;
	}
	else {
		for (i = 0; i < particles.Num(); i++) {
			particles[i].time = time;
		}
	}

	UpdateVisuals();
}

int idAI::NumSmokeEmitters(void) {
	return particles.Num();
}

void idAI::StopThinking(void) {
	BecomeInactive(TH_THINK);
	idThread* thread = idThread::CurrentThread();
	if (thread) {
		thread->DoneProcessing();
	}
}

float idAI::GetTurnDelta(void) {
	if (turnRate) {
		float amount = idMath::AngleNormalize180(ideal_yaw - current_yaw);
		return amount;
	}
	else {
		return 0.0f;
	}
}

int idAI::GetMoveType(void) {
	return move.moveType;
}

void idAI::SetMoveType(int moveType) {
	if ((moveType < 0) || (moveType >= NUM_MOVETYPES)) {
		gameLocal.Error("Invalid movetype %d", moveType);
	}

	move.moveType = static_cast<moveType_t>(moveType);
	if (move.moveType == MOVETYPE_FLY) {
		travelFlags = TFL_WALK | TFL_AIR | TFL_FLY;
	}
	else {
		travelFlags = TFL_WALK | TFL_AIR;
	}
}

void idAI::SaveMove(void) {
	savedMove = move;
}

void idAI::RestoreMove(void) {
	idVec3 goalPos;
	idVec3 dest;

	switch (savedMove.moveCommand) {
	case MOVE_NONE:
		StopMove(savedMove.moveStatus);
		break;

	case MOVE_FACE_ENEMY:
		FaceEnemy();
		break;

	case MOVE_FACE_ENTITY:
		FaceEntity(savedMove.goalEntity.GetEntity());
		break;

	case MOVE_TO_ENEMY:
		MoveToEnemy();
		break;

	case MOVE_TO_ENEMYHEIGHT:
		MoveToEnemyHeight();
		break;

	case MOVE_TO_ENTITY:
		MoveToEntity(savedMove.goalEntity.GetEntity());
		break;

	case MOVE_OUT_OF_RANGE:
		MoveOutOfRange(savedMove.goalEntity.GetEntity(), savedMove.range);
		break;

	case MOVE_TO_ATTACK_POSITION:
		MoveToAttackPosition(savedMove.goalEntity.GetEntity(), savedMove.anim);
		break;

	case MOVE_TO_COVER:
		MoveToCover(savedMove.goalEntity.GetEntity(), lastVisibleEnemyPos);
		break;

	case MOVE_TO_POSITION:
		MoveToPosition(savedMove.moveDest);
		break;

	case MOVE_TO_POSITION_DIRECT:
		DirectMoveToPosition(savedMove.moveDest);
		break;

	case MOVE_SLIDE_TO_POSITION:
		SlideToPosition(savedMove.moveDest, savedMove.duration);
		break;

	case MOVE_WANDER:
		WanderAround();
		break;
	}

	if (GetMovePos(goalPos)) {
		CheckObstacleAvoidance(goalPos, dest);
	}
}

void idAI::AllowMovement(float flag) {
	allowMove = (flag != 0.0f);
}

void idAI::JumpFrame(void) {
	AI_JUMP = true;
}

void idAI::EnableClip(void) {
	physicsObj.SetClipMask(MASK_MONSTERSOLID);
	disableGravity = false;
}

void idAI::DisableClip(void) {
	physicsObj.SetClipMask(0);
	disableGravity = true;
}

void idAI::EnableGravity(void) {
	disableGravity = false;
}

void idAI::DisableGravity(void) {
	disableGravity = true;
}

void idAI::EnableAFPush(void) {
	af_push_moveables = true;
}

void idAI::DisableAFPush(void) {
	af_push_moveables = false;
}

void idAI::SetFlySpeed(float speed) {
	if (move.speed == fly_speed) {
		move.speed = speed;
	}
	fly_speed = speed;
}

void idAI::SetFlyOffset(int offset) {
	fly_offset = offset;
}

void idAI::ClearFlyOffset(void) {
	spawnArgs.GetInt("fly_offset", "0", fly_offset);
}

idEntity* idAI::GetClosestHiddenTarget(const char* type) {
	int i;
	idEntity* ent;
	idEntity* bestEnt;
	float time;
	float bestTime;
	const idVec3& org = physicsObj.GetOrigin();
	idActor* enemyEnt = enemy.GetEntity();

	if (!enemyEnt) {
		return NULL;
	}

	if (targets.Num() == 1) {
		ent = targets[0].GetEntity();
		if (ent && idStr::Cmp(ent->GetEntityDefName(), type) == 0) {
			if (!EntityCanSeePos(enemyEnt, lastVisibleEnemyPos, ent->GetPhysics()->GetOrigin())) {
				return ent;
			}
		}
		return NULL;
	}

	bestEnt = NULL;
	bestTime = idMath::INFINITY;
	for (i = 0; i < targets.Num(); i++) {
		ent = targets[i].GetEntity();
		if (ent && idStr::Cmp(ent->GetEntityDefName(), type) == 0) {
			const idVec3& destOrg = ent->GetPhysics()->GetOrigin();
			time = TravelDistance(org, destOrg);
			if ((time >= 0.0f) && (time < bestTime)) {
				if (!EntityCanSeePos(enemyEnt, lastVisibleEnemyPos, destOrg)) {
					bestEnt = ent;
					bestTime = time;
				}
			}
		}
	}
	return bestEnt;
}

idEntity* idAI::GetRandomTarget(const char* type) {
	int i;
	int num;
	int which;
	idEntity* ent;
	idEntity* ents[MAX_GENTITIES];

	num = 0;
	for (i = 0; i < targets.Num(); i++) {
		ent = targets[i].GetEntity();
		if (ent && idStr::Cmp(ent->GetEntityDefName(), type) == 0) {
			ents[num++] = ent;
			if (num >= MAX_GENTITIES) {
				break;
			}
		}
	}

	if (!num) {
		return NULL;
	}

	which = gameLocal.random.RandomInt(num);
	return ents[which];
}

float idAI::TravelDistanceToPoint(const idVec3& pos) {
	return TravelDistance(physicsObj.GetOrigin(), pos);
}

float idAI::TravelDistanceToEntity(idEntity* ent) {
	return TravelDistance(physicsObj.GetOrigin(), ent->GetPhysics()->GetOrigin());
}

float idAI::TravelDistanceBetweenPoints(const idVec3& source, const idVec3& dest) {
	return TravelDistance(source, dest);
}

float idAI::TravelDistanceBetweenEntities(idEntity* source, idEntity* dest) {
	assert(source);
	assert(dest);
	return TravelDistance(source->GetPhysics()->GetOrigin(), dest->GetPhysics()->GetOrigin());
}

void idAI::LookAtEntity(idEntity* ent, float duration) {
	if (ent == this) {
		ent = NULL;
	}

	if ((ent != focusEntity.GetEntity()) || (focusTime < gameLocal.time)) {
		focusEntity = ent;
		alignHeadTime = gameLocal.time;
		forceAlignHeadTime = gameLocal.time + SEC2MS(1);
		blink_time = 0;
	}

	focusTime = gameLocal.time + SEC2MS(duration);
}

void idAI::LookAtEnemy(float duration) {
	idActor* enemyEnt;

	enemyEnt = enemy.GetEntity();
	if ((enemyEnt != focusEntity.GetEntity()) || (focusTime < gameLocal.time)) {
		focusEntity = enemyEnt;
		alignHeadTime = gameLocal.time;
		forceAlignHeadTime = gameLocal.time + SEC2MS(1);
		blink_time = 0;
	}

	focusTime = gameLocal.time + SEC2MS(duration);
}

void idAI::SetJointMod(int allow) {
	allowJointMod = (allow != 0);
}

void idAI::ThrowMoveable(void) {
	idEntity* ent;
	idEntity* moveable = NULL;

	for (ent = GetNextTeamEntity(); ent != NULL; ent = ent->GetNextTeamEntity()) {
		if (ent->GetBindMaster() == this && ent->IsType(idMoveable::GetClassType())) {
			moveable = ent;
			break;
		}
	}
	if (moveable) {
		moveable->Unbind();
		moveable->PostEventMS(&EV_SetOwner, 200, NULL);
	}
}

void idAI::ThrowAF(void) {
	idEntity* ent;
	idEntity* af = NULL;

	for (ent = GetNextTeamEntity(); ent != NULL; ent = ent->GetNextTeamEntity()) {
		if (ent->GetBindMaster() == this && ent->IsType(idAFEntity_Base::GetClassType())) {
			af = ent;
			break;
		}
	}
	if (af) {
		af->Unbind();
		af->PostEventMS(&EV_SetOwner, 200, NULL);
	}
}

void idAI::SetAngles(idAngles const& ang) {
	current_yaw = ang.yaw;
	viewAxis = idAngles(0, current_yaw, 0).ToMat3();
}

idVec3 idAI::GetAngles(void) {
	return idVec3(0.0f, current_yaw, 0.0f);
}


void idAI::RealKill(void) {
	health = 0;

	if (af.IsLoaded()) {
		af.Rest();
		BecomeActive(TH_PHYSICS);
	}

	Killed(this, this, 0, vec3_zero, INVALID_JOINT);
}

void idAI::Kill(void) {
	PostEventMS(&AI_RealKill, 0);
}

void idAI::WakeOnFlashlight(int enable) {
	wakeOnFlashlight = (enable != 0);
}

void idAI::LocateEnemy(void) {
	idActor* enemyEnt;
	int areaNum;

	enemyEnt = enemy.GetEntity();
	if (!enemyEnt) {
		return;
	}

	enemyEnt->GetAASLocation(aas, lastReachableEnemyPos, areaNum);
	SetEnemyPosition();
	UpdateEnemyPosition();
}

void idAI::KickObstacles(idEntity* kickEnt, float force) {
	idVec3 dir;
	idEntity* obEnt;

	if (kickEnt) {
		obEnt = kickEnt;
	}
	else {
		obEnt = move.obstacle.GetEntity();
	}

	if (obEnt) {
		dir = obEnt->GetPhysics()->GetOrigin() - physicsObj.GetOrigin();
		dir.Normalize();
	}
	else {
		dir = viewAxis[0];
	}
	KickObstacles(dir, force, obEnt);
}

idEntity* idAI::GetObstacle(void) {
	return move.obstacle.GetEntity();
}

idVec3 idAI::PushPointIntoAAS(const idVec3& pos) {
	int areaNum;
	idVec3 newPos;

	areaNum = PointReachableAreaNum(pos);
	if (areaNum) {
		newPos = pos;
		aas->PushPointIntoAreaNum(areaNum, newPos);
		return newPos;
	}
	else {
		return pos;
	}
}

float idAI::GetTurnRate(void) {
	return turnRate;
}

void idAI::SetTurnRate(float rate) {
	turnRate = rate;
}

void idAI::AnimTurn(float angles) {
	turnVel = 0.0f;
	anim_turn_angles = angles;
	if (angles) {
		anim_turn_yaw = current_yaw;
		anim_turn_amount = idMath::Fabs(idMath::AngleNormalize180(current_yaw - ideal_yaw));
		if (anim_turn_amount > anim_turn_angles) {
			anim_turn_amount = anim_turn_angles;
		}
	}
	else {
		anim_turn_amount = 0.0f;
		animator.CurrentAnim(ANIMCHANNEL_LEGS)->SetSyncedAnimWeight(0, 1.0f);
		animator.CurrentAnim(ANIMCHANNEL_LEGS)->SetSyncedAnimWeight(1, 0.0f);
		animator.CurrentAnim(ANIMCHANNEL_TORSO)->SetSyncedAnimWeight(0, 1.0f);
		animator.CurrentAnim(ANIMCHANNEL_TORSO)->SetSyncedAnimWeight(1, 0.0f);
	}
}

void idAI::AllowHiddenMovement(int enable) {
	allowHiddenMovement = (enable != 0);
}

idEntity* idAI::FindActorsInBounds(const idVec3& mins, const idVec3& maxs) {
	idEntity* ent;
	idEntity* entityList[MAX_GENTITIES];
	int numListedEntities;
	int i;

	numListedEntities = gameLocal.clip.EntitiesTouchingBounds(idBounds(mins, maxs), CONTENTS_BODY, entityList, MAX_GENTITIES);
	for (i = 0; i < numListedEntities; i++) {
		ent = entityList[i];
		if (ent != this && !ent->IsHidden() && (ent->health > 0) && ent->IsType(idActor::GetClassType())) {
			return ent;
		}
	}

	return NULL;
}

bool idAI::CanReachPosition(const idVec3& pos) {
	aasPath_t path;
	int toAreaNum;
	int areaNum;

	toAreaNum = PointReachableAreaNum(pos);
	areaNum = PointReachableAreaNum(physicsObj.GetOrigin());
	return (toAreaNum && PathToGoal(path, areaNum, physicsObj.GetOrigin(), toAreaNum, pos));
}

bool idAI::CanReachEntity(idEntity* ent) {
	aasPath_t path;
	int toAreaNum;
	int areaNum;
	idVec3 pos;

	if (!ent) {
		return false;
	}

	if (move.moveType != MOVETYPE_FLY) {
		if (!ent->GetFloorPos(64.0f, pos)) {
			return false;
		}
		if (ent->IsType(idActor::GetClassType()) && static_cast<idActor*>(ent)->OnLadder()) {
			return false;
		}
	}
	else {
		pos = ent->GetPhysics()->GetOrigin();
	}

	toAreaNum = PointReachableAreaNum(pos);
	if (!toAreaNum) {
		return false;
	}

	const idVec3& org = physicsObj.GetOrigin();
	areaNum = PointReachableAreaNum(org);
	return (toAreaNum && PathToGoal(path, areaNum, org, toAreaNum, pos));
}

bool idAI::CanReachEnemy(void) {
	aasPath_t path;
	int toAreaNum;
	int areaNum;
	idVec3 pos;
	idActor* enemyEnt;

	enemyEnt = enemy.GetEntity();
	if (!enemyEnt) {
		return false;
	}

	if (move.moveType != MOVETYPE_FLY) {
		if (enemyEnt->OnLadder()) {
			return false;
		}
		enemyEnt->GetAASLocation(aas, pos, toAreaNum);
	}
	else {
		pos = enemyEnt->GetPhysics()->GetOrigin();
		toAreaNum = PointReachableAreaNum(pos);
	}

	if (!toAreaNum) {
		return false;
	}

	const idVec3& org = physicsObj.GetOrigin();
	areaNum = PointReachableAreaNum(org);
	return PathToGoal(path, areaNum, org, toAreaNum, pos);
}

idVec3 idAI::GetReachableEntityPosition(idEntity* ent) {
	int toAreaNum;
	idVec3 pos;

	if (move.moveType != MOVETYPE_FLY) {
		if (!ent->GetFloorPos(64.0f, pos)) {
			return vec3_zero;
		}
		if (ent->IsType(idActor::GetClassType()) && static_cast<idActor*>(ent)->OnLadder()) {
			return vec3_zero;
		}
	}
	else {
		pos = ent->GetPhysics()->GetOrigin();
	}

	if (aas) {
		toAreaNum = PointReachableAreaNum(pos);
		aas->PushPointIntoAreaNum(toAreaNum, pos);
	}

	return pos;
}

bool idAI::MeleeAttackToJoint(const char* jointname, const char* meleeDefName) {
	jointHandle_t joint;
	idVec3 start;
	idVec3 end;
	idMat3 axis;
	trace_t trace;
	idEntity* hitEnt;

	joint = animator.GetJointHandle(jointname);
	if (joint == INVALID_JOINT) {
		gameLocal.Error("Unknown joint '%s' on %s", jointname, GetEntityDefName());
	}
	animator.GetJointTransform(joint, gameLocal.time, end, axis);
	end = physicsObj.GetOrigin() + (end + modelOffset) * viewAxis * physicsObj.GetGravityAxis();
	start = GetEyePosition();

	if (ai_debugMove.GetBool()) {
		gameRenderWorld->DebugLine(colorYellow, start, end, gameLocal.msec);
	}

	gameLocal.clip.TranslationEntities(trace, start, end, NULL, mat3_identity, MASK_SHOT_BOUNDINGBOX, this);
	if (trace.fraction < 1.0f) {
		hitEnt = gameLocal.GetTraceEntity(trace);
		if (hitEnt && hitEnt->IsType(idActor::GetClassType())) {
			DirectDamage(meleeDefName, hitEnt);
			return true;
		}
	}

	return false;
}

void idAI::Touch(idEntity* other, trace_t* trace) {
	if (!enemy.GetEntity() && !other->fl.notarget && (ReactionTo(other) & ATTACK_ON_ACTIVATE)) {
		Activate(other);
	}
	AI_PUSHED = true;
}