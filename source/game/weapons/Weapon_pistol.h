#pragma once

class idWeaponPistol : public idWeaponBase {
public:
	CLASS_PROTOTYPE(idWeaponPistol);

	idWeaponPistol();
	virtual ~idWeaponPistol() = default;

	virtual void Init(idWeapon *owner) override;

protected:
	void InitStates() override;

	void State_Idle() override;
	void State_Lower() override;
	void State_Raise() override;
	void State_Fire() override;
	void State_Reload() override;

private:
	float next_attack;
	float spread;

	// Additional states for mini state machines
	void State_Raise_WaitForAnim();
	void State_Lower_WaitForAnim();
	void State_Fire_WaitForAnim();
	void State_Reload_WaitForAnim();
};