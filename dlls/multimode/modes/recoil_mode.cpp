#include <unordered_map>
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "multimode/multimode_gamerules.h"
#include "game.h"
#include "recoil_mode.h"

static MMConfigVar<CRecoilMode, nlohmann::json> mp_mm_recoil_spawn_weapons("spawn_weapons", nullptr);
static MMConfigVar<CRecoilMode, nlohmann::json> mp_mm_recoil_spawn_ammo("spawn_ammo", nullptr);
static MMConfigVar<CRecoilMode, nlohmann::json> mp_mm_recoil_allowed_weapons("allowed_weapons", nullptr);
static MMConfigVar<CRecoilMode, nlohmann::json> mp_mm_recoil_knockback("knockback", nullptr);
static MMConfigVar<CRecoilMode, bool> mp_mm_recoil_enable_crits("enable_crits", true);
static MMConfigVar<CRecoilMode, float> mp_mm_recoil_air_time("air_time", 0.8f);
static MMConfigVar<CRecoilMode, float> mp_mm_recoil_crit_dmg("crit_dmg", 2.f);
static MMConfigVar<CRecoilMode, int> mp_mm_recoil_msg_channel("msg_channel", 1);

static const int s_CritColor[3] = { 245, 229, 12 };
static const int s_NoCritColor[3] = { 209, 8, 8 };
constexpr float MSG_PERIOD = 0.3f;

static const std::unordered_map<std::string, CRecoilMode::WeaponType> s_BulletTypes = {
	{ "glock", CRecoilMode::WeaponType::WPNTYPE_GLOCK },
	{ "python", CRecoilMode::WeaponType::WPNTYPE_PYTHON },
	{ "shotgun", CRecoilMode::WeaponType::WPNTYPE_SHOTGUN },
	{ "mp5", CRecoilMode::WeaponType::WPNTYPE_MP5 }
};

extern "C" void SV_OnMoveWalk(int entidx, int onground, int waterlevel)
{
	if (!IsRunningMultimode(ModeID::Recoil))
		return;
	CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(entidx);
	if (pPlayer)
		GetRunningMultimode<CRecoilMode>()->OnMoveWalk(pPlayer, onground, waterlevel);
}

CRecoilMode::CRecoilMode() : CBaseMode()
{
	m_CritTextParams.x = -1.0f;
	m_CritTextParams.y = 0.6f;
	m_CritTextParams.effect = 0;
	m_CritTextParams.fadeinTime = 0.05f;
	m_CritTextParams.fadeoutTime = 0.05f;
	m_CritTextParams.holdTime = 4.f;
	m_CritTextParams.fxTime = 4.f;
	m_CritTextParams.channel = mp_mm_recoil_msg_channel.Get();
}

ModeID CRecoilMode::GetModeID()
{
	return MODE_ID;
}

const char *CRecoilMode::GetModeName()
{
	return MODE_NAME;
}

const char *CRecoilMode::GetShortTitle()
{
	return "Extreme Recoil";
}

const char *CRecoilMode::GetDescription()
{
	return "All weapons push you back by a lot.";
}

void CRecoilMode::ValidateConfig(const nlohmann::json &json)
{
	CBaseMode::ValidateConfig(json);

	// Check mp_mm_recoil_spawn_weapons
	nlohmann::json spawn_weapons = json.at("spawn_weapons");

	if (!spawn_weapons.is_array())
		throw std::runtime_error("'spawn_weapons' is not an array");

	for (const nlohmann::json &item : spawn_weapons)
	{
		if (!item.is_object())
			throw std::runtime_error("spawn_weapons[x] is not an array");
		if (!item.at("entity").is_string())
			throw std::runtime_error("spawn_weapons[x].entity is not a string");
		if (!item.at("probability").is_number() && !item.at("probability").is_null())
			throw std::runtime_error("spawn_weapons[x].probability is not a number or null");
	}

	// Check mp_mm_recoil_spawn_ammo
	nlohmann::json spawn_ammo = json.at("spawn_ammo");

	if (!spawn_ammo.is_array())
		throw std::runtime_error("'spawn_ammo' is not an array");

	for (const nlohmann::json &item : spawn_ammo)
	{
		if (!item.is_object())
			throw std::runtime_error("spawn_ammo[x] is not an array");
		if (!item.at("ammo").is_string())
			throw std::runtime_error("spawn_ammo[x].ammo is not a string");
		if (!item.at("count").is_number_integer())
			throw std::runtime_error("spawn_ammo[x].count is not an integer");
	}

	// Check mp_mm_recoil_spawn_weapons
	nlohmann::json allowed_weapons = json.at("allowed_weapons");

	if (!allowed_weapons.is_array())
		throw std::runtime_error("'allowed_weapons' is not an array");

	for (const nlohmann::json &item : allowed_weapons)
	{
		if (!item.is_string())
			throw std::runtime_error("allowed_weapons[x] is not a string");
	}

	// Check mp_mm_recoil_knockback
	nlohmann::json knockback = json.at("knockback");

	if (!knockback.is_object())
		throw std::runtime_error("'knockback' is not an array");

	for (auto it = knockback.begin(); it != knockback.end(); ++it)
	{
		if (!it.value().is_object())
			throw std::runtime_error("knockback." + it.key() + " is not an object");

		auto it2 = s_BulletTypes.find(it.key());
		if (it2 == s_BulletTypes.end())
			throw std::runtime_error("knockback." + it.key() + ": unknown weapon type");

		if (!it.value().at("shooter").is_number())
			throw std::runtime_error("knockback." + it.key() + ".shooter is not a number");
		if (!it.value().at("victim").is_number())
			throw std::runtime_error("knockback." + it.key() + ".victim is not a number");
	}
}

void CRecoilMode::ApplyConfig(const nlohmann::json &json)
{
	CBaseMode::ApplyConfig(json);

	m_RandomWeapons.clear();
	m_SpawnAmmo.clear();
	m_AllowedWeapons.clear();

	//
	for (const nlohmann::json &item : mp_mm_recoil_spawn_weapons.Get())
	{
		if (item.at("probability").is_null())
		{
			m_SpawnWeapons.push_back(item.at("entity").get<std::string>());
		}
		else
		{
			m_RandomWeapons.push_back({
				item.at("entity").get<std::string>(),
				item.at("probability").get<float>()
			});
		}
	}

	std::sort(m_RandomWeapons.begin(), m_RandomWeapons.end(), [](const Wpn &lhs, const Wpn &rhs) -> bool {
		return (lhs.rnd < rhs.rnd);
	});

	float sum = 0;
	for (size_t i = 0; i < m_RandomWeapons.size(); i++)
	{
		sum += m_RandomWeapons[i].rnd;
		m_RandomWeapons[i].rnd = sum;
	}
	
	//
	for (const nlohmann::json &item : mp_mm_recoil_spawn_ammo.Get())
	{
		m_SpawnAmmo.push_back({
			item.at("ammo").get<std::string>(),
			item.at("count").get<int>()
		});
	}

	//
	for (const nlohmann::json &item : mp_mm_recoil_allowed_weapons.Get())
	{
		m_AllowedWeapons.push_back(item.get<std::string>());
	}

	//
	for (auto it = mp_mm_recoil_knockback.Get().begin(); it != mp_mm_recoil_knockback.Get().end(); ++it)
	{
		auto it2 = s_BulletTypes.find(it.key());
		if (it2 == s_BulletTypes.end())
			throw std::runtime_error("knockback." + it.key() + ": unknown weapon type");

		m_WeaponInfo[it2->second] = {
			it.value().at("shooter").get<float>(),
			it.value().at("victim").get<float>()
		};
	}
}

void CRecoilMode::GivePlayerWeapons(CBasePlayer *pPlayer)
{
	// Give default items
	for (std::string &i : m_SpawnWeapons)
	{
		pPlayer->GiveNamedItem(i.c_str());
	}

	// Give random weapon
	float rnd = RANDOM_FLOAT(0, 1);
	size_t i;
	for (i = 0; i < m_RandomWeapons.size(); i++)
	{
		if (rnd <= m_RandomWeapons[i].rnd || fabs(rnd - m_RandomWeapons[i].rnd) <= 0.001)
			break;
	}

	if (i < m_RandomWeapons.size())
		pPlayer->GiveNamedItem(m_RandomWeapons[i].ent.c_str());

	// Give ammo
	for (Ammo &i : m_SpawnAmmo)
	{
		int idx = pPlayer->GetAmmoIndex(i.type.c_str());
		if (idx != -1)
			pPlayer->m_rgAmmo[idx] = i.count;
	}
}

bool CRecoilMode::ShouldRespawnWeapon(const char *classname)
{
	for (std::string &i : m_AllowedWeapons)
	{
		if (!_stricmp(classname, i.c_str()))
			return true;
	}

	return false;
}

float CRecoilMode::GetWeaponKnockback(Bullet bullet)
{
	if (GetMultimodeGR()->GetState() != CHalfLifeMultimode::State::Game)
		return 0;

	switch (bullet)
	{
	case BULLET_PLAYER_9MM:
		return m_WeaponInfo[WPNTYPE_GLOCK].shooter;
		break;
	case BULLET_PLAYER_MP5:
		return m_WeaponInfo[WPNTYPE_MP5].shooter;
		break;
	case BULLET_PLAYER_357:
		return m_WeaponInfo[WPNTYPE_PYTHON].shooter;
		break;
	case BULLET_PLAYER_BUCKSHOT:
		return m_WeaponInfo[WPNTYPE_SHOTGUN].shooter;
		break;
	}

	return 0;
}

float CRecoilMode::GetVictimKnockback(Bullet bullet)
{
	if (GetMultimodeGR()->GetState() != CHalfLifeMultimode::State::Game)
		return 0;

	switch (bullet)
	{
	case BULLET_PLAYER_9MM:
		return m_WeaponInfo[WPNTYPE_GLOCK].victim;
		break;
	case BULLET_PLAYER_MP5:
		return m_WeaponInfo[WPNTYPE_MP5].victim;
		break;
	case BULLET_PLAYER_357:
		return m_WeaponInfo[WPNTYPE_PYTHON].victim;
		break;
	case BULLET_PLAYER_BUCKSHOT:
		return m_WeaponInfo[WPNTYPE_SHOTGUN].victim;
		break;
	}

	return 0;
}

void CRecoilMode::PlayerSpawn(CBasePlayer *pPlayer)
{
	if (mp_mm_recoil_enable_crits.Get())
	{
		m_PlayerInfo[pPlayer->entindex()] = PlayerInfo();
		PlayerThink(pPlayer);
	}
}

void CRecoilMode::PlayerThink(CBasePlayer *pPlayer)
{
	if (mp_mm_recoil_enable_crits.Get() && GetMultimodeGR()->GetState() == CHalfLifeMultimode::State::Game)
	{
		int idx = pPlayer->entindex();
		PlayerInfo &pi = m_PlayerInfo[idx];
		if (!pi.bCritsActive)
		{
			if (pi.flLastWalkTime + mp_mm_recoil_air_time.Get() <= gpGlobals->time)
			{
				pi.bCritsActive = true;
				pi.flNextMsg = gpGlobals->time;
			}
		}

		if (pi.flNextMsg <= gpGlobals->time)
		{
			if (pi.bCritsActive)
			{
				m_CritTextParams.r1 = s_CritColor[0];
				m_CritTextParams.g1 = s_CritColor[1];
				m_CritTextParams.b1 = s_CritColor[2];
				UTIL_HudMessage(pPlayer, m_CritTextParams, "CRITS ACTIVE");
			}
			else
			{
				m_CritTextParams.r1 = s_NoCritColor[0];
				m_CritTextParams.g1 = s_NoCritColor[1];
				m_CritTextParams.b1 = s_NoCritColor[2];
				UTIL_HudMessage(pPlayer, m_CritTextParams, "NO CRITS");
			}

			pi.flNextMsg = gpGlobals->time + MSG_PERIOD;
		}
	}
}

void CRecoilMode::OnMoveWalk(CBasePlayer *pPlayer, int onground, int waterlevel)
{
	if (mp_mm_recoil_enable_crits.Get())
	{
		int idx = pPlayer->entindex();
		PlayerInfo &pi = m_PlayerInfo[idx];
		if (pi.bCritsActive)
		{
			if (!(onground == -1 && waterlevel == 0))
			{
				// Touched the ground
				pi.flLastWalkTime = gpGlobals->time;
				pi.bCritsActive = false;
				pi.flNextMsg = gpGlobals->time;
			}
		}
		else
		{
			if (!(onground == -1 && waterlevel == 0))
			{
				// Still walking
				pi.flLastWalkTime = gpGlobals->time;
			}
		}
	}
}

int CRecoilMode::GetCritDamage(CBasePlayer *pAttacker, CBaseEntity *pVictim, int iOrigDmg, int iWeapon)
{
	if (m_PlayerInfo[pAttacker->entindex()].bCritsActive)
		return (int)((float)iOrigDmg * mp_mm_recoil_crit_dmg.Get());
	else
		return iOrigDmg;
}
