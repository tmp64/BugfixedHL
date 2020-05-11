#ifndef MULTIMODE_BIOHAZARD_MODE_H
#define MULTIMODE_DM_MODE_H
#include "multimode/basemode.h"

class CBiohazardMode : public CBaseMode
{
public:
	static constexpr ModeID MODE_ID = ModeID::Biohazard;
	static constexpr char MODE_NAME[] = "biohazard";

	CBiohazardMode();

	virtual ModeID GetModeID();
	virtual const char *GetModeName();
	virtual const char *GetShortTitle();
	virtual void GetShortTitleColor(byte &r, byte &g, byte &b);
	virtual const char *GetDescription();

	virtual void OnFreezeStart();

	void OnSnarkSpawn(CBaseEntity *pSnark, CBasePlayer *pPlayer);
	void OnSnarkDeath(CBaseEntity *pSnark, CBasePlayer *pPlayer);

	virtual void ClientDisconnected(edict_t *pClient);
	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);
	virtual bool ShouldRespawnWeapons();
	virtual bool ShouldRespawnAmmo();
	virtual void PlayerThink(CBasePlayer *pPlayer);

private:
	enum class WeaponType
	{
		None = 0,
		Snark,
		Hornet
	};

	struct PlayerInfo
	{
		WeaponType wpnType = WeaponType::None;
		int snarkCount = 0;
	};

	PlayerInfo m_Players[MAX_PLAYERS];

	void KillAllSnarks(CBasePlayer *pPlayer);
};

#endif
