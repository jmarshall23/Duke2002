#pragma once


class idAnimState {
public:
	bool					idleAnim;
	idStr					state;
	int						animBlendFrames;
	int						lastAnimBlendFrames;		// allows override anims to blend based on the last transition time

public:
	idAnimState();
	~idAnimState();

	void					Save(idSaveGame* savefile) const;
	void					Restore(idRestoreGame* savefile);

	void					Init(idActor* owner, idAnimator* _animator, int animchannel);
	void					Shutdown(void);
	void					SetState(const char* name, int blendFrames);
	void					StopAnim(int frames);
	void					PlayAnim(int anim);
	void					CycleAnim(int anim);
	void					BecomeIdle(void);
	bool					UpdateState(void);
	bool					Disabled(void) const;
	void					Enable(int blendFrames);
	void					Disable(void);
	bool					AnimDone(int blendFrames) const;
	bool					IsIdle(void) const;
	animFlags_t				GetAnimFlags(void) const;

private:
	idActor* self;
	idAnimator* animator;
	idThread* thread;
	int						channel;
	bool					disabled;
};
