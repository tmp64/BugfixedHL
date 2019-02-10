#ifndef CGAMEUPDATER_H
#define CGAMEUPDATER_H
#include <atomic>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include <CGameVersion.h>

// #define UPDATER_WIP_DL		// Not finished yet - do not use

class CUpdateWorker;

class CGameUpdater
{
public:
	enum E_UpdateStatus
	{
		NONE = 0,
		ERROR_EXCEPTION,
		LOADING_RELEASES,
		UP_TO_DATE,
		UPDATE_FOUND,
		DOWNLOADING,
		DL_FINISHED,
		INSTALLING,
		UPDATE_STATUS_LENGTH
	};

	enum E_UpdateModule
	{
		UPD_CLIENT,
		UPD_SERVER
	};

	// Callbacks:
	//	ErrorOccuredCallback(std::string error)
	//		Called when an exception in updater thread is thrown
	//		std::string error - exception description
	//	CheckFinishedCallback(bool isUpdateFound)
	//		Releases list downloaded successfully
	//		bool isUpdateFound - true if an update was found
	//	DownloadFinishedCallback()
	//		The new update was downloaded successfully and it is ready to be installed
	typedef std::function<void(std::string error)> ErrorOccuredCallback_t;
	typedef std::function<void(bool isUpdateFound)> CheckFinishedCallback_t;
	typedef std::function<void()> DownloadFinishedCallback_t;

	CGameUpdater();
	void Frame();
	void CheckForUpdates();
	std::string GetChangeLog();
#ifdef UPDATER_WIP_DL
	void Download();
	void Install();	// May throw exceptions
#endif

	bool IsUpdateFound();
	bool IsReadyToDownload();
#ifdef UPDATER_WIP_DL
	bool IsReadyToInstall();
	bool IsExternalUpdaterRequired();
#endif

	int  AddErrorOccuredCallback(ErrorOccuredCallback_t f);
	void RemoveErrorOccuredCallback(int idx);
	int  AddCheckFinishedCallback(CheckFinishedCallback_t f);
	void RemoveCheckFinishedCallback(int idx);
#ifdef UPDATER_WIP_DL
	int AddDownloadFinishedCallback(DownloadFinishedCallback_t f);
	void RemoveDownloadFinishedCallback(int idx);
#endif

	CGameVersion GetGameVersion();
	CGameVersion GetLatestVersion();

private:
	std::atomic<E_UpdateStatus>	m_iUpdateStatus	= NONE;
	E_UpdateModule	m_iUpdateModule;
	CUpdateWorker  *m_pWorker = nullptr;
	std::thread		m_WorkerThread;
	std::string		m_ChangeLog;
	CGameVersion	m_LatestVersion;
	CGameVersion	m_GameVersion;
	bool m_bIsUpdateInProcess = false;

	// Callback containers
	std::vector<ErrorOccuredCallback_t> m_cbErrorOccured;
	std::vector<CheckFinishedCallback_t> m_cbCheckFinished;
#ifdef UPDATER_WIP_DL
	std::vector<DownloadFinishedCallback_t> m_cbDownloadFinished;
#endif

	// Logger for main thread. Line break is appended automatically.
	void log(const char *str);
	void logf(const char *fmt, ...);
	void dlog(const char *str);
	void dlogf(const char *fmt, ...);

	friend class CUpdateWorker;
};

extern CGameUpdater *gGameUpdater;

#endif