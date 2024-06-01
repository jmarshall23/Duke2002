#pragma once


/*
===============================================================================

	Animated entity base class.

===============================================================================
*/

typedef struct damageEffect_s {
	jointHandle_t			jointNum;
	idVec3					localOrigin;
	idVec3					localNormal;
	int						time;
	const idDeclParticle* type;
	struct damageEffect_s* next;
} damageEffect_t;

class idAnimatedEntity : public idEntity {
public:
	CLASS_PROTOTYPE(idAnimatedEntity);

	idAnimatedEntity();
	~idAnimatedEntity();

	void					Save(idSaveGame* savefile) const;
	void					Restore(idRestoreGame* savefile);

	virtual void			ClientPredictionThink(void);
	virtual void			Think(void);

	void					UpdateAnimation(void);

	virtual idAnimator* GetAnimator(void);
	virtual void			SetModel(const char* modelname);

	bool					GetJointWorldTransform(jointHandle_t jointHandle, int currentTime, idVec3& offset, idMat3& axis);
	bool					GetJointTransformForAnim(jointHandle_t jointHandle, int animNum, int currentTime, idVec3& offset, idMat3& axis) const;

	virtual int				GetDefaultSurfaceType(void) const;
	virtual void			AddDamageEffect(const trace_t& collision, const idVec3& velocity, const char* damageDefName);
	void					AddLocalDamageEffect(jointHandle_t jointNum, const idVec3& localPoint, const idVec3& localNormal, const idVec3& localDir, const idDeclEntityDef* def, const idMaterial* collisionMaterial);
	void					UpdateDamageEffects(void);

	virtual bool			ClientReceiveEvent(int event, int time, const idBitMsg& msg);

	enum {
		EVENT_ADD_DAMAGE_EFFECT = idEntity::EVENT_MAXEVENTS,
		EVENT_MAXEVENTS
	};

protected:
	idAnimator				animator;
	damageEffect_t* damageEffects;

private:
	void					Event_GetJointHandle(const char* jointname);
	void 					Event_ClearAllJoints(void);
	void 					Event_ClearJoint(jointHandle_t jointnum);
	void 					Event_SetJointPos(jointHandle_t jointnum, jointModTransform_t transform_type, const idVec3& pos);
	void 					Event_SetJointAngle(jointHandle_t jointnum, jointModTransform_t transform_type, const idAngles& angles);
	void 					Event_GetJointPos(jointHandle_t jointnum);
	void 					Event_GetJointAngle(jointHandle_t jointnum);
};