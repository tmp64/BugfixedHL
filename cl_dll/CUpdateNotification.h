#ifndef CUPDATENOTIFICATION_H
#define CUPDATENOTIFICATION_H

class CUpdateNotification
{
public:
	CUpdateNotification();
	~CUpdateNotification();

	void SetActive(bool state);

private:
	bool m_bIsActive = false;
	bool m_bIsNotified = false;
	int m_iCallbackId = -1;

	void CheckFinishedCallback(bool isUpdateFound);
};

extern CUpdateNotification *gUpdateNotif;

#endif
