#ifndef MULTIMODE_GAMERULES_H
#define MULTIMODE_GAMERULES_H
#include <cassert>
#include <string>
#include <vector>
#include <json.hpp>
#include "cdll_dll.h"
#include "skill.h"
#include "multimode.h"
#include "basemode.h"

class CHalfLifeMultimode : public CHalfLifeMultiplay
{
public:
	using BaseClass = CHalfLifeMultiplay;

	enum class State
	{
		Initial = 0,
		InvalidConfig,
		Waiting,
		Warmup,
		FreezeTime,
		Game,
		Intermission,
		Endgame,
		FinalIntermission,
		StateCount
	};

	//-------------------------------------------------------------------
	// CModeManager
	//-------------------------------------------------------------------
	/**
	 * Controls mode initialization and correct swithing.
	 */
	class CModeManager
	{
	public:
		CModeManager();
		CModeManager(const CModeManager &) = delete;
		CModeManager &operator=(const CModeManager &) = delete;
		~CModeManager();

		/**
		 * Returns an active mode or nullptr if none is running.
		 */
		inline CBaseMode *GetBaseMode()
		{
			return m_pActiveMode;
		}

		/**
		 * If no mode is running, returns nullptr.
		 * If mode T is running, returns GetBaseMode() casted to T.
		 * If other mode is running, behavior is undefined.
		 */
		template <typename T>
		inline T *GetMode()
		{
			ASSERT(GetBaseMode()->GetModeID() == T::MODE_ID);
			return static_cast<T *>(GetBaseMode());
		}

		/**
		 * Returns ID of active mode.
		 */
		inline ModeID GetModeID()
		{
			return m_ActiveModeID;
		}

		/**
		 * Returns list of all modes.
		 */
		inline const std::vector<CBaseMode *> &GetModeList()
		{
			return m_ModeList;
		}

		/**
		 * Prepares the mode for freezetime, shows mode info.
		 * If any mode was running before, it is switched off.
		 */
		void PrepareMode(ModeID mode, bool bShowModeInfo);

		/**
		 * Begins the mode.
		 */
		void StartMode();

		/**
		 * Finishes the mode (intermission).
		 */
		void FinishMode();

		/**
		 * Disables active mode.
		 */
		void SwitchOffMode();

	private:
		CBaseMode *m_pActiveMode = nullptr;
		ModeID m_ActiveModeID = ModeID::None;

		CBaseMode *m_pModes[(int)ModeID::ModeCount] = {};
		std::vector<CBaseMode *> m_ModeList;

		/**
		 * Respawns items, etc.
		 */
		void PrepareWorld();

		/**
		 * Registes mode of type T.
		 */
		template <typename T>
		inline T *RegisterMode()
		{
			static_assert((int)T::MODE_ID < (int)ModeID::ModeCount, "mode ID is invalid");

			T *pMode = new T();
			m_pModes[(int)T::MODE_ID] = pMode;
			m_ModeList.push_back(pMode);

			// Sanity checks
			ASSERT(pMode->GetModeID() == T::MODE_ID);
			ASSERT(!strcmp(pMode->GetModeName(), T::MODE_NAME));

			pMode->OnInit();

			return pMode;
		}

		inline CBaseMode *GetModeForID(ModeID id)
		{
			return m_pModes[(int)id];
		}
	};

	//-------------------------------------------------------------------
	// CStateMachine
	//-------------------------------------------------------------------
	/**
	 * An implementation of finite state machine.
	 */
	class CStateMachine
	{
	public:
		class CBaseState
		{
		public:
			CBaseState() = default;
			CBaseState(const CBaseState &) = delete;
			CBaseState &operator=(const CBaseState &) = delete;
			virtual ~CBaseState() = default;

			/**
			 * Called every server frame when state is active.
			 */
			virtual void Think() = 0;

			/**
			 * Called when this state is activated.
			 */
			virtual void OnSwitchTo(State oldState);

			/**
			 * Called when this state is deactivated.
			 */
			virtual void OnSwitchFrom(State newState);

			/**
			 * Causes timer to update next tick.
			 * Called automatically before OnSwitchTo.
			 */
			inline void ResetTimerUpdate()
			{
				m_flNextTimerUpdate = 0;
			}

		protected:
			/**
			 * Time of next timer update (common var used in nearly all states).
			 */
			float m_flNextTimerUpdate = 0;
		};

		CStateMachine();
		CStateMachine(const CStateMachine &) = delete;
		CStateMachine &operator=(const CStateMachine &) = delete;
		~CStateMachine();

		/**
		 * Adds a new state with specified state ID.
		 */
		void AddState(State stateId, CBaseState *stateObj);

		/**
		 * Switches state machine to a new state.
		 */
		void SwitchTo(State newStateId);

		/**
		 * Think the active state.
		 */
		void Think();

		/**
		 * Returns active state.
		 */
		inline State GetState()
		{
			return m_ActiveStateID;
		}

	private:
		CBaseState *m_States[(int)State::StateCount] = {};
		CBaseState *m_pActiveState = nullptr;
		State m_ActiveStateID = State::Initial;

		inline CBaseState *GetStateObj(State stateId)
		{
			return m_States[(int)stateId];
		}
	};

	//-------------------------------------------------------------------
	// States
	//-------------------------------------------------------------------
	class CInitialState : public CStateMachine::CBaseState
	{
	public:
		virtual void Think() override;
	};
	//-------------------------------------------------------------------
	class CInvalidConfigState : public CStateMachine::CBaseState
	{
	public:
		virtual void Think() override;
		virtual void OnSwitchTo(State oldState) override;
	};
	//-------------------------------------------------------------------
	class CWaitingState : public CStateMachine::CBaseState
	{
	public:
		virtual void Think() override;
		virtual void OnSwitchTo(State oldState) override;
	};
	//-------------------------------------------------------------------
	class CWarmupState : public CStateMachine::CBaseState
	{
	public:
		virtual void Think() override;
		virtual void OnSwitchTo(State oldState) override;

	private:
		float m_flWarmupEndTime = 0;
	};
	//-------------------------------------------------------------------
	class CFreezeTimeState : public CStateMachine::CBaseState
	{
	public:
		virtual void Think() override;
		virtual void OnSwitchTo(State oldState) override;

	private:
		float m_flFreezeEndTime = 0;
		int m_iFreezeNextSec = 0;
	};
	//-------------------------------------------------------------------
	class CGameState : public CStateMachine::CBaseState
	{
	public:
		virtual void Think() override;
		virtual void OnSwitchTo(State oldState) override;

	private:
		float m_flEndTime = 0;
	};
	//-------------------------------------------------------------------
	class CIntermissionState : public CStateMachine::CBaseState
	{
	public:
		virtual void Think() override;
		virtual void OnSwitchTo(State oldState) override;
		virtual void OnSwitchFrom(State newState) override;

	private:
		float m_flIntermEndTime = 0;
	};
	//-------------------------------------------------------------------
	class CEndgameState : public CStateMachine::CBaseState
	{
	public:
		virtual void Think() override;
		virtual void OnSwitchTo(State oldState) override;
	};
	//-------------------------------------------------------------------
	class CFinalIntermissionState : public CStateMachine::CBaseState
	{
	public:
		virtual void Think() override;
		virtual void OnSwitchTo(State oldState) override;
	};
	//-------------------------------------------------------------------

	//-------------------------------------------------------------------
	// Playlist
	//-------------------------------------------------------------------
	class CPlaylist
	{
	public:
		/**
		 * Returns ID of the next mode in the playlist or ModeID::None if it's the end.
		 */
		ModeID GetNextModeID();

		/**
		 * Call after active mode has finished to update internal mode counter.
		 */
		void OnModeFinished();

		/**
		 * Restarts playlist from beginning.
		 */
		void ResetPos();

		/**
		 * Adds all enabled modes to the playlist.
		 */
		void AddAllModes();

		/**
		 * Shuffles contents of the playlist.
		 */
		void Shuffle();
	private:
		std::vector<ModeID> m_Queue;
		int m_iPos = 0;
	};

	CHalfLifeMultimode();
	virtual ~CHalfLifeMultimode();

	/**
	 * Prepares next mode in the queue.
	 * Resets world, respawns and freezes players.
	 * Switches state to FreezeTime or finishes the game.
	 */
	void PrepareNextMode(bool bShowModeInfo = true);

	/**
	 * Prepares specific mode.
	 * State is not changed.
	 */
	void PrepareMode(ModeID modeId, bool bShowModeInfo = true);

	/**
	 * Unfreezes players, the main game period.
	 * Switches state to Game.
	 */
	void StartMode();

	/**
	 * Freezes players again, shows stats on the screen.
	 * Switches state to Intermission
	 */
	void FinishMode();

	/**
	 * Disables active mode.
	 * State is not changed.
	 */
	void SwitchOffMode();

	/**
	 * Finish the game - go to map vote or restart
	 */
	void FinishGame();

	/**
	 * Returns current state.
	 */
	inline State GetState()
	{
		return m_StateMachine.GetState();
	}

	virtual const char *GetGameDescription();
	virtual void Think();

	/**
	 * Returns an active mode or nullptr if none is running.
	 */
	inline CBaseMode *GetBaseMode()
	{
		return m_ModeManager.GetBaseMode();
	}

	/**
	 * If no mode is running, return nullptr.
	 * If mode T is running, returns GetBaseMode() casted to T.
	 * If other mode is running, behavior is undefined.
	 */
	template <typename T>
	inline T *GetMode()
	{
		return m_ModeManager.GetMode<T>();
	}

	/**
	 * Returns iD of running mode.
	 */
	inline ModeID GetModeID()
	{
		return m_ModeManager.GetModeID();
	}

	/**
	 * Returns list of all modes.
	 */
	inline const std::vector<CBaseMode *> &GetModeList()
	{
		return m_ModeManager.GetModeList();
	}

	/**
	 * Returns game time frm the config (not overriden by modes).
	 */
	inline int GetDefaultGameTime()
	{
		return m_ParsedConfig.gameTime;
	}

	// After primary attack
	void OnPrimaryAttack(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);

private:
	//-------------------------------------------------------------------
	// Type definitions
	//-------------------------------------------------------------------
	enum class EndAction
	{
		StartOver,
		Restart
	};

	enum class PlaylistType
	{
		All,	// All enabled modes are added to the playlist
	};

	struct ParsedConfig
	{
		int minPlayers = 1;
		int warmupTime = 60;
		int freezeTime = 5;
		int gameTime = 60;
		int intermTime = 5;
		EndAction onEnd = EndAction::StartOver;
		PlaylistType playlistType = PlaylistType::All;
		bool playlistAllRandom = true;
		int rounds = 1;
	};

	CStateMachine m_StateMachine;
	CModeManager m_ModeManager;
	CPlaylist m_Playlist;
	bool m_bFreezeOnSpawn = false;
	int m_iRoundsFinished = 0;

	//-------------------------------------------------------------------
	// Const data (not changed during runtime)
	//-------------------------------------------------------------------
	skilldata_t m_DefSkillData;

	//-------------------------------------------------------------------
	// HUD Texts
	//-------------------------------------------------------------------
	hudtextparms_t m_WarmupTextParams;
	hudtextparms_t m_TimerTextParams;
	hudtextparms_t m_ModeTitleTextParams;
	hudtextparms_t m_ModeInfoTextParams;
	hudtextparms_t m_IntermStatsTextParams;

	//-------------------------------------------------------------------
	// Configuration
	//-------------------------------------------------------------------
	ParsedConfig m_ParsedConfig;

	//-------------------------------------------------------------------
	// Non-copyable
	//-------------------------------------------------------------------
	CHalfLifeMultimode(const CHalfLifeMultimode &) = delete;
	CHalfLifeMultimode &operator=(const CHalfLifeMultimode &) = delete;

	//-------------------------------------------------------------------
	// Intialization
	//-------------------------------------------------------------------
	/**
	 * Loads config file or throws an exception.
	 */
	nlohmann::json LoadConfigFile();

	/**
	 * Applies config file or throws an exception.
	 * In case of exception, no changes.
	 */
	void ApplyConfigFile(const nlohmann::json &config);

	/**
	 * Populates hudtextparms_t with config data.
	 */
	void InitHudTexts();

	//-------------------------------------------------------------------

	friend bool IsRunningMultimode(ModeID mode);

public:
	// CGameRules overrides
	virtual void ClientDisconnected(edict_t *pClient);
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer);
	virtual BOOL  FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker);
	virtual void PlayerSpawn(CBasePlayer *pPlayer);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual BOOL FPlayerCanRespawn(CBasePlayer *pPlayer);
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer);
	virtual edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer);
	virtual int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled);
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);

	// Weapon retrieval
	virtual BOOL CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);// The player is touching an CBasePlayerItem, do I give it to him?
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);// Called each time a player picks up a weapon from the ground

	// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn(CBasePlayerItem *pWeapon);// should this weapon respawn?
	virtual float FlWeaponRespawnTime(CBasePlayerItem *pWeapon);// when may this weapon respawn?
	virtual float FlWeaponTryRespawn(CBasePlayerItem *pWeapon); // can i respawn now,  and if not, when should i try again?
	virtual Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon);// where in the world should this weapon respawn?

	// Item retrieval
	virtual BOOL CanHaveItem(CBasePlayer *pPlayer, CItem *pItem);// is this player allowed to take this item?
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem);// call each time a player picks up an item (battery, healthkit, longjump)

	// Item spawn/respawn control
	virtual int ItemShouldRespawn(CItem *pItem);// Should this item respawn?
	virtual float FlItemRespawnTime(CItem *pItem);// when may this item respawn?
	virtual Vector VecItemRespawnSpot(CItem *pItem);// where in the world should this item respawn?

	// Ammo retrieval
	virtual BOOL CanHaveAmmo(CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry);// can this player take more of this ammo?
	virtual void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount);// called each time a player picks up some ammo in the world

	// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo);// should this ammo item respawn?
	virtual float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo);// when should this ammo item respawn?
	virtual Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo);// where in the world should this ammo item respawn?
																			// by default, everything spawns

	// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime();// how long until a depleted HealthCharger recharges itself?
	virtual float FlHEVChargerRechargeTime();// how long until a depleted HealthCharger recharges itself?

	// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);// what do I do with a player's weapons when he's killed?

	// What happens to a dead player's ammo	
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);// Do I drop ammo when the player dies? How much?

	// Critical hits
	virtual int GetCritDamage(CBasePlayer *pAttacker, CBaseEntity *pVictim, int iOrigDmg, int iWeapon);
	virtual void OnCritHit(CBasePlayer *pAttacker, CBaseEntity *pVictim, int iOrigDmg, int iCritDmg, int iWeapon);
};

#endif
