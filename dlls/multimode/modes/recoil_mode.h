#ifndef MULTIMODE_RECOIL_MODE_H
#define MULTIMODE_RECOIL_MODE_H
#include <vector>
#include "multimode/basemode.h"

/**
 * High Recoil
 *
 * All available weapons have huge knockback (like charged tau cannon).
 * They also knock back players that were hit by the guns.
 *
 * Changes to Vector CBaseEntity::FireBulletsPlayer (combat.cpp):
 * - Added code to apply velocity to the shooter.
 * - Added code to apply velocity to the victim.
 */
class CRecoilMode : public CBaseMode
{
public:
	enum WeaponType
	{
		WPNTYPE_GLOCK,
		WPNTYPE_PYTHON,
		WPNTYPE_SHOTGUN,
		WPNTYPE_MP5,
		WPNTYPE_COUNT
	};

	static constexpr ModeID MODE_ID = ModeID::Recoil;
	static constexpr char MODE_NAME[] = "recoil";

	CRecoilMode();

	virtual ModeID GetModeID();
	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual const char *GetDescription();
	virtual void ValidateConfig(const nlohmann::json &json);
	virtual void ApplyConfig(const nlohmann::json &json);

	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual bool ShouldRespawnWeapon(const char *classname);

	float GetWeaponKnockback(Bullet bullet);
	float GetVictimKnockback(Bullet bullet);

	virtual void PlayerSpawn(CBasePlayer *pPlayer);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	void OnMoveWalk(CBasePlayer *pPlayer, int onground, int waterlevel);

	virtual int GetCritDamage(CBasePlayer *pAttacker, CBaseEntity *pVictim, int iOrigDmg, int iWeapon);

private:
	struct WeaponInfo
	{
		float shooter = 0;
		float victim = 0;
	};

	struct Wpn
	{
		std::string ent;
		float rnd;
	};

	struct Ammo
	{
		std::string type;
		int count;
	};

	struct PlayerInfo
	{
		bool bCritsActive = false;
		float flLastWalkTime = 0;
		float flNextMsg = 0;
	};

	hudtextparms_t m_CritTextParams;
	WeaponInfo m_WeaponInfo[WPNTYPE_COUNT] = {};
	PlayerInfo m_PlayerInfo[MAX_PLAYERS + 1] = {};

	std::vector<Wpn> m_RandomWeapons;
	std::vector<std::string> m_SpawnWeapons;
	std::vector<Ammo> m_SpawnAmmo;
	std::vector<std::string> m_AllowedWeapons;
};

#endif
