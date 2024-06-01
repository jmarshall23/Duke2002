// Weapon_shotgun.h
//

#pragma once

class idWeaponShotgun : public idWeaponBase {
public:
	CLASS_PROTOTYPE(idWeaponShotgun);

	idWeaponShotgun();
	virtual ~idWeaponShotgun() = default;

	virtual void Init(idWeapon* owner) override;

protected:
	void InitStates() override;

	void State_Idle() override;
	void State_Lower() override;
	void State_Raise() override;
	void State_NoAmmo();
	void State_Fire();
	void State_Reload();
	virtual void ExitCinematic() override;
	virtual void StateChangedEvent(void) override;
private:
	float next_attack;
	float spread;
	float side;
	bool isReloadAnimPlaying = false;

	// Additional states for mini state machines
	void State_Raise_WaitForAnim();
	void State_Lower_WaitForAnim();
	void State_Fire_WaitForAnim();
	void State_Reload_Start();
	void State_Reload_Loop();
	void State_Reload_End();
	void State_Reload_WaitForAnim();
};