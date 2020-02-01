#ifndef MULTIMODE_BASEMODE_H
#define MULTIMODE_BASEMODE_H

class CBaseMode
{
public:
	//-----------------------------------------------------------------
	// Mode info
	//-----------------------------------------------------------------
	/**
	 * Returns the name of the gamemode (usually class name)
	 */
	virtual const char *GetModeName() = 0;

	/**
	 * Returns short title of the gamemode.
	 * Will be displayed in the middle of the screen
	 */
	virtual const char *GetShortTitle() = 0;

	/**
	 * Sets r, g, b to color of the sgort title
	 */
	virtual void GetShortTitleColor(byte &r, byte &g, byte &b);

	/**
	 * Returns description of the mode.
	 * Will be displayed in the middle of the screen
	 */
	virtual const char *GetDescription() = 0;

	//-----------------------------------------------------------------
	// CHalfLifeMultimode callbacks
	//-----------------------------------------------------------------
	/**
	 * Called at the start of the freeze time before start
	 * You can change gSkillData, it will be restored automatically later.
	 */
	virtual void OnFreezeStart();

	/**
	 * Called at the start of the freeze time to give players weapons
	 * e.g. pPlayer->GiveNamedItem("weapon_crowbar");
	 */
	virtual void GivePlayerWeapons(CBasePlayer *pPlayer);

	/**
	 * Called at the start of the mode
	 */
	virtual void OnStart();

	/**
	 * Called at the end of the mode
	 */
	virtual void OnEnd();

	/**
	 * Runs every server frame, should handle any timer tasks, periodic events, etc.
	 */
	virtual void Think();

	/**
	 * Called when a client just disconnected from the server.
	 */
	virtual void ClientDisconnected(edict_t *pClient);

	/**
	 * Called just before releasing player into the game.
	 */
	virtual void PlayerSpawn(CBasePlayer *pPlayer);

	/**
	 * Called every frame, before physics are run and after keys are accepted.
	 */
	virtual void PlayerThink(CBasePlayer *pPlayer);

	/**
	 * Called just after Weapon::PrimaryAttack.
	 */
	virtual void OnPrimaryAttack(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);

	//-----------------------------------------------------------------
	// Player spawn
	//-----------------------------------------------------------------
	/**
	 * Is this player allowed to respawn now?
	 */
	virtual bool PlayerCanRespawn(CBasePlayer *pPlayer);

	/**
	 * When in the future will this player be able to spawn?
	 */
	virtual float PlayerSpawnTime(CBasePlayer *pPlayer);

	/**
	 * Place this player on their spawnspot and face them the proper direction.
	 * Return nullptr for default HLDM behaviour.
	 */
	virtual edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer);

	//-----------------------------------------------------------------
	// Scoring
	//-----------------------------------------------------------------
	/**
	 * Returns how many points should we give (or take, if negative) to
	 * pAttacker after they killed pKilled
	 */
	virtual int PointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled);

	/**
	 * Called each time a player dies.
	 */
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);

	//-----------------------------------------------------------------
	// Item management
	//-----------------------------------------------------------------
	/**
	 * Returns true if weapons should be respawned at the start of the mode.
	 */
	virtual bool ShouldRespawnWeapons();

	/**
	 * Returns true if weapons should be respawned later after pick up.
	 * @see GetWeaponRespawnTime
	 */
	virtual bool ShouldRespawnWeaponsAfterPickUp();

	/**
	 * Returns true if ammo should be respawned at the start of the mode.
	 */
	virtual bool ShouldRespawnAmmo();

	/**
	 * Returns true if ammo should be respawned later after pick up.
	 * @see GetAmmoRespawnTime
	 */
	virtual bool ShouldRespawnAmmoAfterPickUp();

	/**
	 * Returns true if Items should be respawned at the start of the mode.
	 */
	virtual bool ShouldRespawnItems();

	/**
	 * Returns true if Items should be respawned later after pick up.
	 * @see GetItemRespawnTime
	 */
	virtual bool ShouldRespawnItemsAfterPickUp();

	/**
	 * Called when respawning weapons. Returns true if weapon should be respawned
	 * @arg classname Classname of the weapon (e.g. weapon_crowbar)
	 */
	virtual bool ShouldRespawnWeapon(const char *classname);

	/**
	 * Returns number of seconds weapons should be respawned after pick up
	 */
	virtual float GetWeaponRespawnTime();

	/**
	 * Returns number of seconds ammo should be respawned after pick up
	 */
	virtual float GetAmmoRespawnTime();

	/**
	 * Returns number of seconds items should be respawned after pick up
	 */
	virtual float GetItemRespawnTime();

	/**
	 * The player is touching an CBasePlayerItem, do I give it to him?
	 */
	virtual bool CanHaveWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);

	/**
	 * Called each time a player picks up a weapon from the ground
	 */
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);

	/**
	 * is this player allowed to take this item?
	 */
	virtual bool CanHaveItem(CBasePlayer *pPlayer, CItem *pItem);

	/**
	 * call each time a player picks up an item (battery, healthkit, longjump)
	 */
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem);

	//-----------------------------------------------------------------
	// Player damage
	//-----------------------------------------------------------------
	/**
	 * Whether or not pPlayer can be damaged by pAttacker
	 */
	virtual bool PlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker);

	/**
	 * pPlayer just hit the ground after a fall. How much damage?
	 * Return -1 for default HLDM behaviour.
	 */
	virtual float PlayerFallDamage(CBasePlayer *pPlayer);

	//-----------------------------------------------------------------
	// Wall chargers
	//-----------------------------------------------------------------
	/**
	 * How long until a depleted health charger recharges itself?
	 * Return 0 for never or -1 for default.
	 */
	virtual float HealthChargerRechargeTime();

	/**
	 * How long until a depleted suit recharges itself?
	 * Return 0 for never or -1 for default.
	 */
	virtual float SuitChargerRechargeTime();

	//-----------------------------------------------------------------
	// Items on death
	//-----------------------------------------------------------------
	/**
	 * What do I do with a player's weapons when he's killed?
	 * Return GR_PLR_DROP_GUN_*
	 */
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);

	/**
	 * What do I do with a player's ammo when he's killed?
	 * Return GR_PLR_DROP_AMMO_*
	 */
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);

protected:
	CBaseMode();

private:
	CBaseMode(const CBaseMode &) = delete;
	CBaseMode &operator=(const CBaseMode &) = delete;
};

#endif
