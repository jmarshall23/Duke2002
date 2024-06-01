// AnimState.cpp
//

#include "precompiled.h"

#include "Game_local.h"

/*
=====================
idAnimState::idAnimState
=====================
*/
idAnimState::idAnimState() {
	self = NULL;
	animator = NULL;
	thread = NULL;
	idleAnim = true;
	disabled = true;
	channel = ANIMCHANNEL_ALL;
	animBlendFrames = 0;
	lastAnimBlendFrames = 0;
}

/*
=====================
idAnimState::~idAnimState
=====================
*/
idAnimState::~idAnimState() {
	delete thread;
}

/*
=====================
idAnimState::Save
=====================
*/
void idAnimState::Save(idSaveGame* savefile) const {

	savefile->WriteObject(self);

	// Save the entity owner of the animator
	savefile->WriteObject(animator->GetEntity());

	savefile->WriteObject(thread);

	savefile->WriteString(state);

	savefile->WriteInt(animBlendFrames);
	savefile->WriteInt(lastAnimBlendFrames);
	savefile->WriteInt(channel);
	savefile->WriteBool(idleAnim);
	savefile->WriteBool(disabled);
}

/*
=====================
idAnimState::Restore
=====================
*/
void idAnimState::Restore(idRestoreGame* savefile) {
	savefile->ReadObject(reinterpret_cast<idClass*&>(self));

	idEntity* animowner;
	savefile->ReadObject(reinterpret_cast<idClass*&>(animowner));
	if (animowner) {
		animator = animowner->GetAnimator();
	}

	savefile->ReadObject(reinterpret_cast<idClass*&>(thread));

	savefile->ReadString(state);

	savefile->ReadInt(animBlendFrames);
	savefile->ReadInt(lastAnimBlendFrames);
	savefile->ReadInt(channel);
	savefile->ReadBool(idleAnim);
	savefile->ReadBool(disabled);
}

/*
=====================
idAnimState::Init
=====================
*/
void idAnimState::Init(idActor* owner, idAnimator* _animator, int animchannel) {
	assert(owner);
	assert(_animator);
	self = owner;
	animator = _animator;
	channel = animchannel;

	if (!thread) {
		thread = new idThread();
		thread->ManualDelete();
	}
	thread->EndThread();
	thread->ManualControl();
}

/*
=====================
idAnimState::Shutdown
=====================
*/
void idAnimState::Shutdown(void) {
	delete thread;
	thread = NULL;
}

/*
=====================
idAnimState::SetState
=====================
*/
void idAnimState::SetState(const char* statename, int blendFrames) {
	const function_t* func;

	func = self->scriptObject.GetFunction(statename);
	if (!func) {
		assert(0);
		gameLocal.Error("Can't find function '%s' in object '%s'", statename, self->scriptObject.GetTypeName());
	}

	state = statename;
	disabled = false;
	animBlendFrames = blendFrames;
	lastAnimBlendFrames = blendFrames;
	thread->CallFunction(self, func, true);

	animBlendFrames = blendFrames;
	lastAnimBlendFrames = blendFrames;
	disabled = false;
	idleAnim = false;

	if (ai_debugScript.GetInteger() == self->entityNumber) {
		gameLocal.Printf("%d: %s: Animstate: %s\n", gameLocal.time, self->name.c_str(), state.c_str());
	}
}

/*
=====================
idAnimState::StopAnim
=====================
*/
void idAnimState::StopAnim(int frames) {
	animBlendFrames = 0;
	animator->Clear(channel, gameLocal.time, FRAME2MS(frames));
}

/*
=====================
idAnimState::PlayAnim
=====================
*/
void idAnimState::PlayAnim(int anim) {
	if (anim) {
		animator->PlayAnim(channel, anim, gameLocal.time, FRAME2MS(animBlendFrames));
	}
	animBlendFrames = 0;
}

/*
=====================
idAnimState::CycleAnim
=====================
*/
void idAnimState::CycleAnim(int anim) {
	if (anim) {
		animator->CycleAnim(channel, anim, gameLocal.time, FRAME2MS(animBlendFrames));
	}
	animBlendFrames = 0;
}

/*
=====================
idAnimState::BecomeIdle
=====================
*/
void idAnimState::BecomeIdle(void) {
	idleAnim = true;
}

/*
=====================
idAnimState::Disabled
=====================
*/
bool idAnimState::Disabled(void) const {
	return disabled;
}

/*
=====================
idAnimState::AnimDone
=====================
*/
bool idAnimState::AnimDone(int blendFrames) const {
	int animDoneTime;

	animDoneTime = animator->CurrentAnim(channel)->GetEndTime();
	if (animDoneTime < 0) {
		// playing a cycle
		return false;
	}
	else if (animDoneTime - FRAME2MS(blendFrames) <= gameLocal.time) {
		return true;
	}
	else {
		return false;
	}
}

/*
=====================
idAnimState::IsIdle
=====================
*/
bool idAnimState::IsIdle(void) const {
	return disabled || idleAnim;
}

/*
=====================
idAnimState::GetAnimFlags
=====================
*/
animFlags_t idAnimState::GetAnimFlags(void) const {
	animFlags_t flags;

	memset(&flags, 0, sizeof(flags));
	if (!disabled && !AnimDone(0)) {
		flags = animator->GetAnimFlags(animator->CurrentAnim(channel)->AnimNum());
	}

	return flags;
}

/*
=====================
idAnimState::Enable
=====================
*/
void idAnimState::Enable(int blendFrames) {
	if (disabled) {
		disabled = false;
		animBlendFrames = blendFrames;
		lastAnimBlendFrames = blendFrames;
		if (state.Length()) {
			SetState(state.c_str(), blendFrames);
		}
	}
}

/*
=====================
idAnimState::Disable
=====================
*/
void idAnimState::Disable(void) {
	disabled = true;
	idleAnim = false;
}

/*
=====================
idAnimState::UpdateState
=====================
*/
bool idAnimState::UpdateState(void) {
	if (disabled) {
		return false;
	}

	if (ai_debugScript.GetInteger() == self->entityNumber) {
		thread->EnableDebugInfo();
	}
	else {
		thread->DisableDebugInfo();
	}

	thread->Execute();

	return true;
}
