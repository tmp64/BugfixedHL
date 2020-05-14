#ifndef MULTIMODE_RECOIL_MODE_H
#define MULTIMODE_RECOIL_MODE_H
#include <vector>
#include "multimode/basemode.h"

class CRecoilMode : public CBaseMode
{
public:
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

private:
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

	std::vector<Wpn> m_RandomWeapons;
	std::vector<std::string> m_SpawnWeapons;
	std::vector<Ammo> m_SpawnAmmo;
	std::vector<std::string> m_AllowedWeapons;
};

#endif
