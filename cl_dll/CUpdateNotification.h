#ifndef CUPDATENOTIFICATION_H
#define CUPDATENOTIFICATION_H

class CUpdateNotification
{
public:
	CUpdateNotification();
	~CUpdateNotification();

private:
	bool m_bIsNotified = false;
	int m_iCallbackId = -1;

	void CheckFinishedCallback(bool isUpdateFound);
};

extern CUpdateNotification *gUpdateNotif;

#endif
