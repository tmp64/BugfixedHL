/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Svc_messages.h
//
// Engine messages handlers
//

#ifndef SVC_MESSAGES_H
#define SVC_MESSAGES_H
#ifdef _WIN32

void HookSvcMessages(void);
void UnHookSvcMessages(void);
void SvcMessagesInit(void);

bool SanitizeCommands(char *str);

#endif
#endif SVC_MESSAGES_H
