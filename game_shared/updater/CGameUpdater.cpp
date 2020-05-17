#ifdef _WIN32
// Define all that stuff before cURL includes windows.h
#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#endif

#include <atomic>
#include <cassert>
#include <cstdarg>
#include <exception>
#include <mutex>
#include <queue>
#include <string>
#include <sstream>
#include <vector>
#include <curl/curl.h>
#include <curl/easy.h>
#include <json.hpp>
#include <appversion.h>
#include "CGameUpdater.h"
#include "CGameVersion.h"

#if defined(CLIENT_DLL)
#include <cl_dll.h>
#include <cl_util.h>
#elif defined(SERVER_DLL)
#include <extdll.h>
#include <util.h>
#endif

#ifdef UPDATER_WIP_DL
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
// TODO
#endif
#endif

#define LOG_BUFFER_SIZE	1024	// 1 KiB
#define BUF_CHUNK_SIZE	16384	// 16 KiB

CGameUpdater *gGameUpdater = nullptr;

#ifdef UPDATER_WIP_DL
/////////////////////////////////////////////////////////////////
// Platform-dependent code
/////////////////////////////////////////////////////////////////
struct update_path_t
{
	std::string updateDirPath;
	std::string updatePath;
	std::string gameDirPath;
};

// Creates temporary directory and stores it in data
// May throw exceptions
static void _GetUpdatePath(update_path_t &data);

// Return true if an external program is needed to install the update
// E.g. on Windows running binaries cannot be replaced
static bool _NeedExternalUpdater();

// If _NeedExternalUpdater() == true
//   Starts the external updater executable
// Else
//   Unpacks and installs the update
// May throw exceptions
static bool _InstallUpdate(update_path_t &data);

#ifdef _WIN32

static void _GetUpdatePath(update_path_t &data)
{
	static char buf[1024];
	static wchar_t wbuf[MAX_PATH + 1], wbuf2[MAX_PATH + 1];
	GetTempPathW(MAX_PATH + 1, wbuf2);
	if (!GetTempFileNameW(wbuf2, L"BugfixedHL", 0, wbuf))
		throw std::runtime_error("failed to generate temporary directory name");
	DeleteFileW(wbuf);	// GetTempFileName also creates the file but we need a directory
	if (!CreateDirectoryW(wbuf, nullptr))
		throw std::runtime_error("failed to create temporary directory");
	WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, sizeof(buf), NULL, NULL);
	data.updateDirPath = buf;

	data.updatePath = data.updateDirPath + "\\update_win.zip";

	GetCurrentDirectoryW(MAX_PATH + 1, wbuf);
	WIN32_FIND_DATAW fileInfo;
	HANDLE hHlExe = FindFirstFileW((std::wstring(wbuf) + L"\\hl.exe").c_str(), &fileInfo);
	WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, sizeof(buf), NULL, NULL);
	if (hHlExe == INVALID_HANDLE_VALUE)
		throw std::runtime_error(std::string("invalid game directory: ") + buf);
	data.gameDirPath = buf;
}

static bool _NeedExternalUpdater()
{
	return true;
}

static bool _InstallUpdate(update_path_t &data)
{
	throw std::logic_error("Not implemented");
}

#else
// TODO
#error
#endif
#endif

/////////////////////////////////////////////////////////////////
// CUpdateWorker
/////////////////////////////////////////////////////////////////
class CUpdateWorker
{
public:
#ifdef UPDATER_WIP_DL
	update_path_t m_UpdPath;
#endif

	CUpdateWorker(CGameUpdater *pUpdater)
	{
		m_pUpdater = pUpdater;
	}

	~CUpdateWorker()
	{
		m_bStopThread = true;
		m_pUpdater->m_WorkerThread.join();
		GameFrame();	// Flush logs and etc before we get deleted (destructor can only be called from main thread)
	}

	void operator() ()
	{
		m_bIsThreadRunning = true;
		dlogf("Worker thread started.");
		UpdateStatus(CGameUpdater::LOADING_RELEASES);
#ifdef UPDATER_WIP_DL
		m_UpdPath = update_path_t();
#endif
		try
		{
			assert(!m_hCurl);

			// Init cURL
			m_hCurl = curl_easy_init();

			struct curl_slist *headers = nullptr;
			headers = curl_slist_append(headers, "Accept: application/vnd.github.v3+json");
			headers = curl_slist_append(headers, "User-Agent: tmp64-BugfixedHL");

			curl_easy_setopt(m_hCurl, CURLOPT_WRITEFUNCTION, WriteData);
			curl_easy_setopt(m_hCurl, CURLOPT_ERRORBUFFER, m_szCurlError);
			if (gEngfuncs.CheckParm("-bhl_no_ssl_check", nullptr))
			{
				curl_easy_setopt(m_hCurl, CURLOPT_SSL_VERIFYPEER, 0);
				logf("Warning! SSL certificate check disabled with -bhl_no_ssl_check");
			}
			curl_easy_setopt(m_hCurl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(m_hCurl, CURLOPT_FOLLOWLOCATION, 1);

			if (!LoadReleases()) throw false;
			if (m_bStopThread) throw false;
			GenerateChangeLog();
#ifdef UPDATER_WIP_DL
			dlogf("Waiting for DOWNLOADING...");
			while (m_pUpdater->m_iUpdateStatus != CGameUpdater::DOWNLOADING)
			{
				if (m_bStopThread) throw false;
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			DownloadUpdate();
#endif
		}
		catch (const std::exception &e)
		{
			logf("Error: %s", e.what());
			UpdateStatus(CGameUpdater::ERROR_EXCEPTION);
			CallErrorOccured(e.what());
		}
		catch (bool) {}

		// Clean up
		if (m_hCurl)
		{
			curl_easy_cleanup(m_hCurl);
			m_hCurl = nullptr;
		}

		dlogf("Worker thread finished.");
		m_bStopThread = false;
		m_bIsThreadRunning = false;
	}

	void GameFrame()
	{
		// Command queue
		{
			// Process only one request per frame to not lag the main thread
			// E.g. in case of log spam
			m_CmdQueueMutex.lock();
			if (m_CmdQueue.size() > 0)
			{
				command_t item = m_CmdQueue.front();
				m_CmdQueue.pop();
				m_CmdQueueMutex.unlock();
				if (item.type == command_t::CMD_NOP) {}
				else if (item.type == command_t::CMD_LOG)
				{
					m_pUpdater->log(static_cast<char *>(item.data));
					delete[] static_cast<char *>(item.data);
				}
				else if (item.type == command_t::CMD_DLOG)
				{
					m_pUpdater->dlog(static_cast<char *>(item.data));
					delete[] static_cast<char *>(item.data);
				}
				else if (item.type == command_t::CMD_ELOG)
				{
					m_pUpdater->log(static_cast<char *>(item.data));	// FIXME: Use error log function
					delete[] static_cast<char *>(item.data);
				}
				else if (item.type == command_t::CMD_CB_ERROR)
				{
					cb_error_t *data = static_cast<cb_error_t *>(item.data);
					for (auto &i : m_pUpdater->m_cbErrorOccured)
					{
						if (i)
							i(data->error);
					}
				}
				else if (item.type == command_t::CMD_CB_CHECK_FINISHED)
				{
					cb_check_finished_t *data = static_cast<cb_check_finished_t *>(item.data);
					for (auto &i : m_pUpdater->m_cbCheckFinished)
					{
						if (i)
							i(data->isUpdateFound);
					}
				}
#ifdef UPDATER_WIP_DL
				else if (item.type == command_t::CMD_CB_DL_FINISHED)
				{
					for (auto &i : m_pUpdater->m_cbDownloadFinished)
					{
						if (i)
							i();
					}
				}
#endif
				else
					assert(!("invalid command type"));
			}
			else
			{
				m_CmdQueueMutex.unlock();
			}
		}

		// Changelog
		{
			std::lock_guard<std::mutex> guard(m_ChangelogMutex);
			if (m_pUpdater->m_ChangeLog.size() == 0 && m_ChangeLog.size() != 0)
			{
				m_pUpdater->m_ChangeLog = m_ChangeLog;
				UpdateStatus(CGameUpdater::UPDATE_FOUND);
			}
		}
	}

private:
	struct command_t
	{
		enum CommandType
		{
			CMD_NOP = 0,
			CMD_LOG,
			CMD_DLOG,
			CMD_ELOG,
			CMD_CB_ERROR,
			CMD_CB_CHECK_FINISHED,
			CMD_CB_DL_FINISHED
		};
		CommandType type;
		void *data;
	};

	struct cb_error_t
	{
		std::string error;
	};

	struct cb_check_finished_t
	{
		bool isUpdateFound;
	};
	
	char m_szCurlError[CURL_ERROR_SIZE];
	CURL *m_hCurl = nullptr;
	CGameUpdater *m_pUpdater;
	std::queue<command_t> m_CmdQueue;
	std::mutex m_CmdQueueMutex, m_StatusMutex, m_ChangelogMutex;
	std::atomic_bool m_bIsThreadRunning = { false }, m_bStopThread = { false };
	std::string m_ChangeLog;
	nlohmann::json m_jReleases;

	// Loads releases from GitHub to m_jReleases. Returns true if updates found.
	bool LoadReleases()
	{
		dlogf("Loading release list...");
		std::vector<char> buffer;
		buffer.reserve(BUF_CHUNK_SIZE);

		curl_easy_setopt(m_hCurl, CURLOPT_URL, "https://api.github.com/repos/tmp64/BugfixedHL/releases");
		curl_easy_setopt(m_hCurl, CURLOPT_WRITEDATA, &buffer);
		CheckCurlError(curl_easy_perform(m_hCurl));
		buffer.push_back('\0');

		m_jReleases = nlohmann::json::parse(buffer.data());

		std::string latestTag = m_jReleases.at(0).at("tag_name").get<std::string>();	// v1.3.0
		CGameVersion latestVer;
		if (latestTag.empty() || !latestVer.TryParse(latestTag.c_str() + 1))	// +1 to skip 'v'
			throw std::runtime_error(std::string("Tag name is not valid: ") + latestTag);

		logf("Current version: %s", VerToStr(m_pUpdater->m_GameVersion).c_str());
		logf("Latest version:  %s", VerToStr(latestVer).c_str());

		m_pUpdater->m_LatestVersion = latestVer;

		if (m_pUpdater->m_GameVersion >= latestVer)
		{
			logf("Game is up to date.");
			UpdateStatus(CGameUpdater::UP_TO_DATE);
			CallCheckFinished(false);
			return false;
		}
		CallCheckFinished(true);
		return true;
	}

	void GenerateChangeLog()
	{
		dlogf("Generating change log...");
		int count = 0;
		std::string changelog;
		for (auto &item : m_jReleases)
		{
			CGameVersion ver;
			std::string tag = item.at("tag_name").get<std::string>();
			if (tag.empty() || !ver.TryParse(tag.c_str() + 1))
			{
				throw std::runtime_error(std::string("Invalid tag: ") + tag);
			}
			else
			{
				if (ver <= m_pUpdater->m_GameVersion)	// Don't show current version
					break;
			}

			if (count > 0)
			{
				changelog += "\n\n\n\n";
			}

			changelog += item.at("name").get<std::string>();
			changelog += "\n\n";
			changelog += item.at("body").get<std::string>();

			count++;

			if (count >= 20)	// Limit the output
				break;
		}

		// Change \r\n to \n (remove \r)
		size_t index = 0;
		while (true)
		{
			index = changelog.find("\r", index);
			if (index == std::string::npos)
				break;
			changelog.replace(index, 1, "");
			index++;
		}

		m_ChangelogMutex.lock();
		m_ChangeLog = changelog;
		m_ChangelogMutex.unlock();
	}

#ifdef UPDATER_WIP_DL
	void DownloadUpdate()
	{
		dlogf("Downloading new version...");
		FILE *file = nullptr;
		try
		{
			auto assets = m_jReleases.at(0).at("assets");
			std::string fileUrl;
			for (auto &asset : assets)
			{
				std::string name = asset.at("name");
				if (name.find("-client-vgui2-windows.zip") != name.npos)
				{
					fileUrl = asset.at("browser_download_url").get<std::string>();
					break;
				}
			}

			if (fileUrl.size() == 0)
				throw std::runtime_error("URL not found in JSON");

			_GetUpdatePath(m_UpdPath);
			file = fopen(m_UpdPath.updatePath.c_str(), "wb");
			if (!file)
				throw std::runtime_error(std::string("failed to open update file '") + m_UpdPath.updatePath + "' for writing: " + strerror(errno));	// FIXME: errno is not thread safe
			curl_easy_setopt(m_hCurl, CURLOPT_WRITEFUNCTION, WriteDataToFile);	// Write to FILE *file
			curl_easy_setopt(m_hCurl, CURLOPT_WRITEDATA, file);
			curl_easy_setopt(m_hCurl, CURLOPT_URL, fileUrl.c_str());
			CheckCurlError(curl_easy_perform(m_hCurl));
			dlogf("Download finished.");
			UpdateStatus(CGameUpdater::DL_FINISHED);
			CallDownloadFinished();
		}
		catch (...)
		{
			if (file)
				fclose(file);
			throw;
		}

		if (file)
			fclose(file);
	}
#endif

	// Callbacks
	void CallErrorOccured(std::string error)
	{
		m_CmdQueueMutex.lock();
		m_CmdQueue.push({ command_t::CMD_CB_ERROR, new cb_error_t{error} });
		m_CmdQueueMutex.unlock();
	}

	void CallCheckFinished(bool isUpdateFound)
	{
		m_CmdQueueMutex.lock();
		m_CmdQueue.push({ command_t::CMD_CB_CHECK_FINISHED, new cb_check_finished_t{isUpdateFound} });
		m_CmdQueueMutex.unlock();
	}

	void CallDownloadFinished()
	{
		m_CmdQueueMutex.lock();
		m_CmdQueue.push({ command_t::CMD_CB_DL_FINISHED, nullptr });
		m_CmdQueueMutex.unlock();
	}

	// Thread-safe logger
	void logf(const char *fmt, ...)
	{
		char *buf = new char[LOG_BUFFER_SIZE];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, LOG_BUFFER_SIZE, fmt, args);
		m_CmdQueueMutex.lock();
		m_CmdQueue.push({ command_t::CMD_LOG, buf });
		m_CmdQueueMutex.unlock();
		va_end(args);
	}

	void dlogf(const char *fmt, ...)
	{
		char *buf = new char[LOG_BUFFER_SIZE];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, LOG_BUFFER_SIZE, fmt, args);
		m_CmdQueueMutex.lock();
		m_CmdQueue.push({ command_t::CMD_DLOG, buf });
		m_CmdQueueMutex.unlock();
		va_end(args);
	}

	void elogf(const char *fmt, ...)
	{
		char *buf = new char[LOG_BUFFER_SIZE];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, LOG_BUFFER_SIZE, fmt, args);
		m_CmdQueueMutex.lock();
		m_CmdQueue.push({ command_t::CMD_ELOG, buf });
		m_CmdQueueMutex.unlock();
		va_end(args);
	}

	void UpdateStatus(CGameUpdater::E_UpdateStatus s)
	{
		m_pUpdater->m_iUpdateStatus = s;
	}

	void CheckCurlError(CURLcode rc)
	{
		if (rc)
			throw std::runtime_error(std::string("cURL Error: ") + m_szCurlError);
	}

	std::string VerToStr(const CGameVersion &ver)
	{
		int major, minor, patch;
		ver.GetVersion(major, minor, patch);
		std::stringstream s;
		s << major << "." << minor << "." << patch;
		return s.str();
	}

	size_t static WriteData(void *buffer, size_t size, size_t nmemb, void *userp)
	{
		if (gGameUpdater->m_pWorker->m_bStopThread)
			return 0;
		size_t bytes = 0;
		std::vector<char> *pBuf = static_cast<std::vector<char> *>(userp);
		try
		{
			if (pBuf->capacity() - pBuf->size() < nmemb)
				pBuf->reserve(pBuf->capacity() + (((nmemb - 1) / BUF_CHUNK_SIZE) + 1) * BUF_CHUNK_SIZE);	// Reserve additional n*BUF_CHUNK_SIZE bytes
			for (bytes = 0; bytes < nmemb; bytes++)
			{
				pBuf->push_back(static_cast<char *>(buffer)[bytes]);
			}
		}
		catch (const std::exception &e)
		{
			// We shouldn't throw exceptions in C code
			gGameUpdater->m_pWorker->logf("WriteData exception: %s", e.what());
			return 0;
		}
		return bytes;
	}

	size_t static WriteDataToFile(void *buffer, size_t size, size_t nmemb, void *userp)
	{
		if (gGameUpdater->m_pWorker->m_bStopThread)
			return 0;
		return fwrite(buffer, 1, nmemb, static_cast<FILE *>(userp));
	}
};

/////////////////////////////////////////////////////////////////
// CGameUpdater
/////////////////////////////////////////////////////////////////
CGameUpdater::CGameUpdater()
{
	const char *version;
	if (!gEngfuncs.CheckParm("-veroverride", const_cast<char **>(&version)))
		version = APP_VERSION;
	if (!m_GameVersion.TryParse(version))
	{
		logf("Version is invalid: '%s'. Using '0.0.0-invalid+eeeeeee'.", version);
		m_GameVersion.TryParse("0.0.0-invalid+eeeeeee");
	}
}

CGameUpdater::~CGameUpdater()
{
	delete m_pWorker;
}

void CGameUpdater::Frame()
{
	if (m_pWorker)
		m_pWorker->GameFrame();
}

/////////////////////////////////////////////////////////////////
// Checking, downloading, installing
/////////////////////////////////////////////////////////////////
void CGameUpdater::CheckForUpdates()
{
	delete m_pWorker;

#if defined(CLIENT_DLL)
	m_iUpdateModule = UPD_CLIENT;
#elif defined(SERVER_DLL)
	m_iUpdateModule = UPD_SERVER;
#else
	return;
#endif
	m_iUpdateStatus = NONE;
	m_ChangeLog = "";
	m_LatestVersion = CGameVersion();
	m_pWorker = new CUpdateWorker(this);
	m_WorkerThread = std::thread(std::ref(*m_pWorker));
}

std::string CGameUpdater::GetChangeLog()
{
	return m_ChangeLog;
}

#ifdef UPDATER_WIP_DL
void CGameUpdater::Download()
{
	if (!IsReadyToDownload())
	{
		logf("CGameUpdater::Download(): Not ready to download. Status: %d", (int)m_iUpdateStatus);
		return;
	}
	m_iUpdateStatus = DOWNLOADING;
}

void CGameUpdater::Install()
{
	if (!IsReadyToInstall())
	{
		logf("CGameUpdater::Install(): Not ready to install. Status: %d", (int)m_iUpdateStatus);
		throw std::logic_error("not ready to install");
	}
	if (!m_pWorker)
		throw std::logic_error("m_pWorker == nullptr");
	_InstallUpdate(m_pWorker->m_UpdPath);
}
#endif

/////////////////////////////////////////////////////////////////
// Status functions
/////////////////////////////////////////////////////////////////
bool CGameUpdater::IsUpdateFound()
{
	return m_iUpdateStatus >= UPDATE_FOUND;
}

bool CGameUpdater::IsReadyToDownload()
{
	return m_iUpdateStatus == UPDATE_FOUND;
}

#ifdef UPDATER_WIP_DL
bool CGameUpdater::IsReadyToInstall()
{
	return m_iUpdateStatus == DL_FINISHED;
}

bool CGameUpdater::IsExternalUpdaterRequired()
{
	return _NeedExternalUpdater();
}
#endif

/////////////////////////////////////////////////////////////////
// Callbacks
/////////////////////////////////////////////////////////////////
int CGameUpdater::AddErrorOccuredCallback(ErrorOccuredCallback_t f)
{
	m_cbErrorOccured.push_back(f);
	return m_cbErrorOccured.size() - 1;
}

void CGameUpdater::RemoveErrorOccuredCallback(int idx)
{
	if (idx < 0 || (size_t)idx >= m_cbErrorOccured.size())
		return;
	m_cbErrorOccured[idx] = nullptr;
}

int CGameUpdater::AddCheckFinishedCallback(CheckFinishedCallback_t f)
{
	m_cbCheckFinished.push_back(f);
	return m_cbCheckFinished.size() - 1;
}

void CGameUpdater::RemoveCheckFinishedCallback(int idx)
{
	if (idx < 0 || (size_t)idx >= m_cbCheckFinished.size())
		return;
	m_cbCheckFinished[idx] = nullptr;
}

#ifdef UPDATER_WIP_DL
int CGameUpdater::AddDownloadFinishedCallback(DownloadFinishedCallback_t f)
{
	m_cbDownloadFinished.push_back(f);
	return m_cbDownloadFinished.size() - 1;
}

void CGameUpdater::RemoveDownloadFinishedCallback(int idx)
{
	if (idx < 0 || (size_t)idx >= m_cbDownloadFinished.size())
		return;
	m_cbDownloadFinished[idx] = nullptr;
}
#endif

CGameVersion CGameUpdater::GetGameVersion()
{
	return m_GameVersion;
}

CGameVersion CGameUpdater::GetLatestVersion()
{
	if (m_iUpdateStatus < UP_TO_DATE)
		return CGameVersion();
	return m_LatestVersion;
}

/////////////////////////////////////////////////////////////////
// Main thread logger
/////////////////////////////////////////////////////////////////
void CGameUpdater::log(const char *str)
{
#if defined(CLIENT_DLL)
	gEngfuncs.Con_Printf("[Updater] %s\n", str);
#elif defined(SERVER_DLL)
	ALERT(at_console, "[Updater] %s\n", str);
#endif
}

void CGameUpdater::logf(const char * fmt, ...)
{
	static char buf[LOG_BUFFER_SIZE];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, LOG_BUFFER_SIZE, fmt, args);
	log(buf);
	va_end(args);
}

void CGameUpdater::dlog(const char *str)
{
#if defined(CLIENT_DLL)
	gEngfuncs.Con_DPrintf("[Updater] %s\n", str);
#elif defined(SERVER_DLL)
	ALERT(at_console, "[Updater] %s\n", str);
#endif
}

void CGameUpdater::dlogf(const char * fmt, ...)
{
	static char buf[LOG_BUFFER_SIZE];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, LOG_BUFFER_SIZE, fmt, args);
	dlog(buf);
	va_end(args);
}
