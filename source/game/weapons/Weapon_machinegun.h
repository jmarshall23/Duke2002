// Weapon_machinegun.h
//

#pragma once

class idWeaponMachinegun : public idWeaponBase {
public:
	CLASS_PROTOTYPE(idWeaponMachinegun);

	idWeaponMachinegun();
	virtual ~idWeaponMachinegun() = default;

	virtual void Init(idWeapon* owner) override;

protected:
	void InitStates() override;

	void State_Idle() override;
	void State_Lower() override;
	void State_Raise() override;
	void State_Fire();
	void State_Reload();
	void ExitCinematic() override;

private:
	float next_attack;
	float spread;

	// Additional states for mini state machines
	void State_Raise_WaitForAnim();
	void State_Lower_WaitForAnim();
	void State_Fire_WaitForAnim();
	void State_Reload_WaitForAnim();
};