#pragma once

class idWeaponFists : public idWeaponBase {
public:
	CLASS_PROTOTYPE(idWeaponFists);

	idWeaponFists();
	virtual ~idWeaponFists() = default;

	virtual void Init(idWeapon* owner) override;

protected:
	void InitStates() override;
	virtual bool WeaponHasFlashLight() override { return false; }
	void State_Idle() override;
	void State_Lower() override;
	void State_Raise() override;
	void State_Punch();
	virtual void ExitCinematic() override;

	virtual void State_Fire() { };
	virtual void State_Reload() { };

	virtual idStr GetFireAnim() override;

private:
	float side;

	// Additional states for mini state machines
	void State_Raise_WaitForAnim();
	void State_Lower_WaitForAnim();
	void State_Punch_WaitForAnim();
};
