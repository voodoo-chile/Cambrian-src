#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

CVaultEvents::CVaultEvents(PA_PARENT ITreeItemChatLogEvents * pTreeItemParent, const SHashSha1 * pHashFileName)
	{
	Assert(pHashFileName != NULL);
	Assert(pTreeItemParent != NULL);
	Assert(pTreeItemParent->m_paVaultEvents == NULL && "Memory leak!");
	pTreeItemParent->m_paVaultEvents = this;
	m_pParent = pTreeItemParent;
	m_pEventLastSaved = NULL;
	InitToZeroes(OUT &m_history, sizeof(m_history));
	ReadEventsFromDisk(pHashFileName);
	}

CVaultEvents::~CVaultEvents()
	{
	m_arraypaEvents.DeleteAllEvents();
	delete m_history.m_paVault;	// Recursively the vault history
	}

const char c_szE[] = "E";	// For events
void
CVaultEvents::ReadEventsFromDisk(const SHashSha1 * pHashFileName)
	{
	CWaitCursor wait;
	CXmlTree oXmlTreeEvents;
	m_sPathFileName = m_pParent->PGetConfiguration()->SGetPathOfFileName(IN pHashFileName);
	if (oXmlTreeEvents.m_binXmlFileData.BinFileReadE(m_sPathFileName) == errSuccess)
		{
		if (oXmlTreeEvents.EParseFileDataToXmlNodes_ML() == errSuccess)
			{
			if (oXmlTreeEvents.FCompareTagName(c_szE) || oXmlTreeEvents.FCompareTagName("Events"))
				{
				m_arraypaEvents.EventsUnserializeFromDisk(IN oXmlTreeEvents.m_pElementsList, m_pParent);
				// TODO: We need to check if this is the first vault in the chain.  So far, there is always only one vault, however this code will have to be revised when chaining vaults.

				/*
				TIMESTAMP tsOtherLastReceived = d_ts_zNA;
				IEvent * pEventLastSent = NULL;
				IEvent ** ppEventStop;
				IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
				while (ppEvent != ppEventStop)
					{
					IEvent * pEvent = *--ppEventStop;
					if (pEvent->Event_FIsEventTypeReceived())
						{
						if (pEventLastReceived == NULL)
							{
							pEventLastReceived = pEvent;
							m_pParent->m_tsOtherLastReceived = pEvent->m_tsOther;
							}
						else if (pEventLastSent != NULL)
							break;	// We are done
						}
					else
						{
						Assert(pEvent->Event_FIsEventTypeSent());
						if (pEventLastSent == NULL)
							{
							pEventLastSent = pEvent;
							m_pParent->m_tsEventIdLastSent = pEvent->m_tsEventID;
							}
						else if (pEventLastReceived != NULL)
							break;	// We are done
						} // if...else
					} // while
				*/
				} // if
			}
		}
	m_pParent->m_tsEventIdLastSentCached = m_arraypaEvents.TsEventIdLastEventSent();	// Update the timestamp so it is what is from the vault, rather than what was loaded from the configuration, as the Chat Log may have been deleted.
	MessageLog_AppendTextFormatSev(eSeverityNoise, "CVaultEvents::ReadEventsFromDisk(\"{h!}.dat\") for '$s': $I events, m_tsEventIdLastSentCached=$t\n", pHashFileName, m_pParent->TreeItem_PszGetNameDisplay(), m_arraypaEvents.GetSize(), m_pParent->m_tsEventIdLastSentCached);
	if (m_pParent->EGetRuntimeClass() == RTI(TContact))
		{
		TContact * pContact = (TContact *)m_pParent;
		TIMESTAMP tsOtherLastReceived = m_arraypaEvents.TsEventOtherLastEventReceived();
		if (tsOtherLastReceived < pContact->m_tsOtherLastSynchronized || pContact->m_tsOtherLastSynchronized == d_ts_zNA)
			{
			if (pContact->m_tsOtherLastSynchronized != tsOtherLastReceived)
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t Adjusting m_tsOtherLastSynchronized from $t to $t for '$s'\n", pContact->m_tsOtherLastSynchronized, tsOtherLastReceived, m_pParent->TreeItem_PszGetNameDisplay());
			pContact->m_tsOtherLastSynchronized = tsOtherLastReceived;
			}
		}
	else
		{
		Assert(m_pParent->EGetRuntimeClass() == RTI(TGroup));
		if (m_arraypaEvents.FIsEmpty())
			{
			// If there are no events, this means the Chat Log is new or was deleted.  In any regards, make sure the timestamps are initialized to zero to make sure the Chat Log is properly reconstructed.
			TGroupMember ** ppMemberStop;
			TGroupMember ** ppMember = ((TGroup *)m_pParent)->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
			while (ppMember != ppMemberStop)
				{
				TGroupMember * pMember = *ppMember++;
				Assert(pMember != NULL);
				Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
				if (pMember->m_tsOtherLastSynchronized != d_ts_zNULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t Clearing m_tsOtherLastSynchronized $t for group member ^j\n", pMember->m_tsOtherLastSynchronized, pMember->m_pContact);
					pMember->m_tsOtherLastSynchronized = d_ts_zNULL;
					}
				} // while
			}
		} // if...else
	} // ReadEventsFromDisk()

void
CVaultEvents::WriteEventsToDiskIfModified()
	{
	Endorse(m_pEventLastSaved == NULL);	// Always write the vault to disk
	IEvent * pEventLastSaved = PGetEventLast_YZ();
	if (pEventLastSaved != m_pEventLastSaved)
		{
		int cEvents = m_arraypaEvents.GetSize();
		CBinXcpStanzaTypeInfo binXmlEvents;
		binXmlEvents.PvSizeAlloc(100 + 64 * cEvents);	// Pre-allocate about 64 bytes per event.  This estimate will reduce the number of memory re-allocations.
		binXmlEvents.BinAppendTextSzv_VE("<Events v='1' c='$i'>\n", cEvents);
		m_arraypaEvents.EventsSerializeForDisk(INOUT &binXmlEvents);
		binXmlEvents.BinAppendTextSzv_VE("</Events>");
		if (binXmlEvents.BinFileWriteE(m_sPathFileName) == errSuccess)
			m_pEventLastSaved = pEventLastSaved;
		}
	}

void
CVaultEvents::GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC
	{
	parraypEventsChatLog->Copy(IN &m_arraypaEvents);
	}

//	Return the number of remaining events after pEvent.
//	Return 0 if pEvent is at the end of the array, or not within the array.
int
CVaultEvents::UEventsRemaining(IEvent * pEvent) const
	{
	Assert(pEvent != NULL);
	return m_arraypaEvents.UFindRemainingElements(pEvent);
	}

//	Return the number of events received since (after) tsEventID
int
CVaultEvents::UCountEventsReceivedByOtherGroupMembersSinceTimestampEventID(TIMESTAMP tsEventID, TContact * pContactExclude) CONST_MCC
	{
	Assert(pContactExclude != NULL);
	int cEventsReceived = 0;

	if (tsEventID > d_tsOther_kmReserved)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		IEvent ** ppEventSince = ppEventStop;	// Search the array from the end, as the event to search is likely to be a recent one
		while (ppEvent != ppEventSince)
			{
			IEvent * pEvent = *--ppEventSince;
			AssertValidEvent(pEvent);
			Assert(pEvent->m_tsEventID > d_tsOther_kmReserved);
			if (pEvent->m_tsEventID == tsEventID)
				{
				ppEventSince++;	// Skip the event we just found
				break;
				}
			} // while
		// Now, count how many events have been received
		while (ppEventSince != ppEventStop)
			{
			IEvent * pEvent = *ppEventSince++;
			AssertValidEvent(pEvent);
			if (pEvent->m_pContactGroupSender_YZ != pContactExclude && pEvent->Event_FIsEventTypeReceived())
				cEventsReceived++;
			}
		} // if
	return cEventsReceived;
	} // UCountEventsReceivedByOtherGroupMembersSinceTimestampEventID()

int
CVaultEvents::UCountEventsReceivedByOtherGroupMembersSinceTimestampOther(TIMESTAMP tsOther) CONST_MCC
	{
	int cEventsReceived = 0;

	if (tsOther > d_tsOther_kmReserved)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		IEvent ** ppEventSince = ppEventStop;	// Search the array from the end, as the event to search is likely to be a recent one
		while (ppEvent != ppEventSince)
			{
			IEvent * pEvent = *--ppEventSince;
			AssertValidEvent(pEvent);
			if (pEvent->m_tsOther == tsOther)
				{
				ppEventSince++;	// Skip the event we just found
				break;
				}
			} // while
		// Now, count how many events have been received
		while (ppEventSince != ppEventStop)
			{
			IEvent * pEvent = *ppEventSince++;
			AssertValidEvent(pEvent);
			if (pEvent->Event_FIsEventTypeReceived())
				cEventsReceived++;
			}
		}
	return cEventsReceived;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Reset the nickname for the Chat Log if it was automatically generated.
void
ITreeItemChatLog::ChatLog_ResetNickname()
	{
	m_strNicknameChatLog.Empty();
	}

//	Try to return the shortest possible display name for the Chat Log.
//	Since this method is called multiple times, the nickname is cached.
PSZUC
ITreeItemChatLog::ChatLog_PszGetNickname() CONST_MCC
	{
	PSZUC pszNickName = m_strNicknameChatLog;
	if (pszNickName[0] != '\0')
		return pszNickName;
	// The nickname is empty, so attempt to find the best shortest nickname
	PSZUC pszNameDisplay = TreeItem_PszGetNameDisplay();
	PCHUC pchAt = pszNameDisplay;
	while (TRUE)
		{
		CHS ch = *pchAt;
		if (ch == ' ' || ch == '@' || ch == ':')
			{
			BOOL fDigitsRemoved = FALSE;
			PCHUC pchTemp = pchAt;
			while (--pchTemp > pszNameDisplay)
				{
				if (Ch_FIsDigit(*pchTemp))
					fDigitsRemoved = TRUE;
				else if (fDigitsRemoved)
					{
					if (_FSetNickname(pszNameDisplay, pchTemp + 1))
						goto Done;
					break;
					}
				} // while
			if (_FSetNickname(pszNameDisplay, pchAt))
				goto Done;
			}
		else if (ch == '\0')
			break;
		pchAt++;
		} // while
	if (!_FSetNickname(pszNameDisplay, pchAt))
		{
		// If we are unable to set the full display name, it means the contact name is the same as the account, in this case, use the JID
		Assert(EGetRuntimeClass() == RTI(TContact));
		m_strNicknameChatLog = ((TContact *)this)->m_strJidBare;
		}
	Done:
	return m_strNicknameChatLog;
	} // ChatLog_PszGetNickname()

//	Return TRUE if the nickname is different than its parent account.
BOOL
ITreeItemChatLog::_FSetNickname(PSZUC pszBegin, PCHUC pchCopyUntil) CONST_MODIFIED
	{
	m_strNicknameChatLog.InitFromStringCopiedUntilPch(pszBegin, pchCopyUntil);
	if (EGetRuntimeClass() == RTI(TContact))
		{
		return !m_strNicknameChatLog.FCompareStringsNoCase(((TContact *)this)->m_pAccount->ChatLog_PszGetNickname());
		}
	return TRUE;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
ITreeItemChatLogEvents::ITreeItemChatLogEvents(TAccountXmpp * pAccount)
	{
	Assert(pAccount != NULL);
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	m_pAccount = pAccount;
	m_paVaultEvents = NULL;
	m_tsCreated = d_ts_zNULL;
	m_tsEventIdLastSentCached = d_ts_zNA;
	m_cMessagesUnread = 0;
	m_pawLayoutChatLog = NULL;
	}

ITreeItemChatLogEvents::~ITreeItemChatLogEvents()
	{
	delete m_paVaultEvents;
	MainWindow_DeleteLayout(PA_DELETING m_pawLayoutChatLog);
	}

CChatConfiguration *
ITreeItemChatLogEvents::PGetConfiguration() const
	{
	return m_pAccount->PGetConfiguration();
	}

//	ITreeItemChatLogEvents::IRuntimeObject::PGetRuntimeInterface()
void *
ITreeItemChatLogEvents::PGetRuntimeInterface(const RTI_ENUM rti) const
	{
	if (rti == RTI(ITreeItemChatLogEvents))
		return (ITreeItemChatLogEvents *)this;
	return ITreeItem::PGetRuntimeInterface(rti);
	}

//	Return the default download folder path to save files when the user clicks on the "[ Save ]" button.
//	Each contact may have his/her own folder when downloading files.
PSZUC
ITreeItemChatLogEvents::ChatLog_PszGetPathFolderDownload() const
	{
	if (!m_strPathFolderDownload.FIsEmptyString())
		return m_strPathFolderDownload;
	return m_pAccount->ChatLog_PszGetPathFolderDownload();
	}

//	ITreeItemChatLogEvents::IXmlExchange::XmlExchange()
void
ITreeItemChatLogEvents::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	ITreeItem::XmlExchange(pXmlExchanger);
	if (pXmlExchanger->m_fSerializing && m_tsCreated == d_zNA && m_paVaultEvents != NULL)
		{
		IEvent * pEventFirst = (IEvent *)m_paVaultEvents->m_arraypaEvents.PvGetElementFirst_YZ();	// This code is a bit of legacy and should be moved when the Tree Item (contact) is created)
		if (pEventFirst != NULL)
			m_tsCreated = pEventFirst->m_tsEventID;
		else
			m_tsCreated = Timestamp_GetCurrentDateTime();
		}
	pXmlExchanger->XmlExchangeTimestamp("tsCreated", INOUT_F_UNCH_S &m_tsCreated);
	#if 0
	pXmlExchanger->XmlExchangeTimestamp("tsEventSyncSent", INOUT_F_UNCH_S &m_synchronization.tsEventID);
	pXmlExchanger->XmlExchangeTimestamp("tsEventSyncReceived", INOUT_F_UNCH_S &m_synchronization.tsOther);
	#endif
	pXmlExchanger->XmlExchangeStr("DownloadFolder", INOUT_F_UNCH_S &m_strPathFolderDownload);
	pXmlExchanger->XmlExchangeInt("MessagesUnread", INOUT_F_UNCH_S &m_cMessagesUnread);
	}

//	ITreeItemChatLogEvents::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
ITreeItemChatLogEvents::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_FindText:
		ChatLog_FindText();
		return ezMenuActionNone;
	case eMenuSpecialAction_ITreeItemRenamed:
		ChatLog_ResetNickname();
		if (m_pawLayoutChatLog != NULL)
			m_pawLayoutChatLog->ChatLog_EventsRepopulateUpdateUI();
		// Fall Through //
	default:
		return ITreeItemChatLog::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	}

void
ITreeItemChatLogEvents::Vault_GetEventsForChatLog(OUT CArrayPtrEvents * parraypEventsChatLog) CONST_MCC
	{
	Assert(parraypEventsChatLog != NULL);
	parraypEventsChatLog->Copy(IN &Vault_PGet_NZ()->m_arraypaEvents);
	}

const QBrush &
ITreeItemChatLogEvents::ChatLog_OGetBrushForNewMessageReceived()
	{
	return c_brushGreenSuperPale;	// Needs to be fixed
	}

CVaultEvents *
ITreeItemChatLogEvents::Vault_PGet_NZ()
	{
	if (m_paVaultEvents == NULL)
		{
		SHashSha1 hashFileNameVault;
		Vault_GetHashFileName(OUT &hashFileNameVault);
		new CVaultEvents(PA_PARENT this, IN &hashFileNameVault);
		Assert(m_paVaultEvents != NULL);
		}
	return m_paVaultEvents;
	}

void
ITreeItemChatLogEvents::Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD IEvent * paEvent)
	{
	AssertValidEvent(paEvent);
	paEvent->EventAddToVault(PA_PARENT Vault_PGet_NZ());
	if (m_pawLayoutChatLog != NULL)
		m_pawLayoutChatLog->ChatLog_EventAppend(IN paEvent);
	}

void
ITreeItemChatLogEvents::Vault_SetNotModified()
	{
	if (m_paVaultEvents != NULL)
		m_paVaultEvents->SetNotModified();
	}

void
ITreeItemChatLogEvents::Vault_SetModified()
	{
	if (m_paVaultEvents != NULL)
		m_paVaultEvents->SetModified();
	}

IEvent *
ITreeItemChatLogEvents::Vault_PFindEventByID(TIMESTAMP tsEventID) CONST_MCC
	{
	return Vault_PGet_NZ()->m_arraypaEvents.PFindEventByID(tsEventID);
	}

CEventMessageTextSent *
ITreeItemChatLogEvents::Vault_PFindEventLastMessageTextSentMatchingText(const CStr & strMessageText) const
	{
	if (m_paVaultEvents != NULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_paVaultEvents->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			CEventMessageTextSent * pEvent = (CEventMessageTextSent *)*--ppEventStop;
			if (pEvent->EGetEventClass() == CEventMessageTextSent::c_eEventClass)
				{
				if (pEvent->m_strMessageText.FCompareBinary(strMessageText))
					return pEvent;
				break;
				}
			} // while
		}
	return NULL;
	}
/*
CEventMessageTextSent *
ITreeItemChatLogEvents::Vault_PGetEventLastMessageSentEditable_YZ() const
	{
	if (m_paVaultEvents != NULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_paVaultEvents->m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			if (pEvent->EGetEventClass() == CEventMessageTextSent::c_eEventClass)
				{
				if (pEvent->Event_FIsEventRecentThanMinutes(10))
					return (CEventMessageTextSent *)pEvent;	// Only edit the last message if it was written within 10 minutes.  This is to prevent to accidentally edit an old message by pressing the up arrow.
				break;
				}
			} // while
		}
	return NULL;
	}
*/

void
ITreeItemChatLogEvents::Vault_WriteEventsToDiskIfModified()
	{
	if (m_paVaultEvents != NULL)
		m_paVaultEvents->WriteEventsToDiskIfModified();
	}

QString
ITreeItemChatLogEvents::Vault_SGetPath() const
	{
	SHashSha1 hashFileNameEvents;
	Vault_GetHashFileName(OUT &hashFileNameEvents);
	return PGetConfiguration()->SGetPathOfFileName(IN &hashFileNameEvents);
	}

/*
void
ITreeItemChatLogEvents::Socket_WriteXmlFormatted(PSZAC pszFmtTemplate, ...) const
	{
	CSocketXmpp * pSocket = m_pAccount->PGetSocket_YZ();
	if (pSocket != NULL)
		{
		va_list vlArgs;
		va_start(OUT vlArgs, pszFmtTemplate);
		pSocket->Socket_WriteXmlFormatted_VL(pszFmtTemplate, vlArgs);
		}
	}

*/

#if 0
	// API call
	<_a i='identifier' f='function'/>parameters</_a>
	<_A i='identifier' e='error (if any)'>result</_A>

#endif

//	SUPPORTED COMMANDS
//
//	/ping					Ping the contact.  If it is a group, broadcast a ping and record the timestamp of the first group member responding.
//	/sendxml	<xml>		Send XML data directly through the socket.  The user should know what he is doing, because any invalid XML may terminate the network connection.
//	/sendfile
//	/sendbtc
//	/version				Query the version of the software on the contact
//	/api		[fn] [params}	Query an API on the contact
//	/info					Query the information of the contact or group.  This is essentially a PAPI call.
//	/add [user]
//	/find [text]

#define d_chChatLogPrefix_CommandLine		'/'

//	Return the remaining of the command, which are its parameters, or a pointer to an empty string.
//	Return NULL if the command was not recognized
PSZR
PszrCompareStringBeginCommand(PSZUC pszStringCompare, PSZAC pszCommand)
	{
	PSZR pszr = PszrCompareStringBeginNoCase(pszStringCompare, pszCommand);
	if (pszr != NULL)
		{
		if (!Ch_FIsWhiteSpaceOrNullTerminator(*pszr))
			return NULL;	// If there is a character following the command, it means the user typed something else, such as "/pingpong"
		while (Ch_FIsWhiteSpace(*pszr))
			pszr++;	// Skip any white spaces after the command so we return its parameters
		}
	return pszr;
	}


//	Parse the text the user typed and act accordingly:
//	- The most common case is creating an event to send an instant text message to the contact or group.
//	- If the user typed a command, then execute the command.
//	Return TRUE if a 'pause' should be sent to the remote client.
EUserCommand
ITreeItemChatLogEvents::Xmpp_EParseUserCommandAndSendEvents(IN_MOD_INV CStr & strCommandLineOrMessage)
	{
	PSZU pszMessage = strCommandLineOrMessage.PbGetData();
	Assert(pszMessage != NULL);
	// Check if there is a command line instead of a message
	CHS chMessage0 = pszMessage[0];
	if (chMessage0 == d_chChatLogPrefix_CommandLine)
		{
		// We have a command line
		PSZUC pszCommand = pszMessage + 1;
		if (PszrCompareStringBeginCommand(pszCommand, c_sza_ping))
			{
			Xmpp_Ping();
			goto Done;
			}
		if (PszrCompareStringBeginCommand(pszCommand, "version"))
			{
			Xmpp_QueryVersion();
			goto Done;
			}
		PSZUC pszParameters =  PszrCompareStringBeginCommand(pszCommand, "api");
		if (pszParameters != NULL)
			{
			// Query an API on the remote contact
			if (EGetRuntimeClass() == RTI(TContact))
				{
				PSZUC pszApiName = pszParameters;	// The first parameter after "/api" is the API name
				if (*pszApiName != '\0')
					{
					// Find the parameters of the API
					while (TRUE)
						{
						break;
						}
					((TContact *)this)->Xcp_ApiRequest(pszParameters, d_zNA, NULL);
					goto Done;
					}
				}
			}
		pszParameters = PszrCompareStringBeginCommand(pszCommand, "sendxml");
		if (pszParameters != NULL && pszParameters[0] == '<')
			{
			Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD new CEventMessageXmlRawSent(pszParameters));
			goto Done;
			}
		if (pszCommand[0] == d_chChatLogPrefix_CommandLine)
			{
			pszMessage = (PSZU)pszCommand;	// The message begins with "//", therefore the message is the command
			goto SendMessageText;
			}

		// The command is invalid, therefore display something to the user so he/she may learn about the syntax of the command line interface
		Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD new CEventHelp(
			g_strScratchBufferStatusBar.Format(
			"Invalid command: <b>^s</b><br/>"
			"Valid commands are:<br/>"
			"^_^_^_<b>/ping</b> to ping a contact<br/>"
			"^_^_^_<b>/version</b> to query the version of the contact<br/>"
			"^_^_^_<b>/sendxml</b> to send XML data directly through the socket<br/>"
			"^_^_^_<b>/api</b> to invoke a remote API call on the contact<br/>"
			"^_^_^_<b>//</b> to send a text message starting with a <b>/</b><br/>"
			, pszMessage)));
		return eUserCommand_Error;
		} // if (command line)

	// No command line, therefore attempt to send an event
	if (PszrCompareStringBeginNoCase(pszMessage, "file://") == NULL)
		{
		SendMessageText:
		Xmpp_SendEventMessageText(pszMessage);	// Send the text message to the contact (or group)
		return eUserCommand_zMessageTextSent;
		}
	else
		{
		// We wish to send file(s) to the user or group
		Xmpp_SendEventsFileUpload(IN_MOD_INV pszMessage);
		}
	Done:
	return eUserCommand_ComposingStopped;
	} // Xmpp_EParseUserCommandAndSendEvents()

//	Send a text message to a contact or a group.
void
ITreeItemChatLogEvents::Xmpp_SendEventMessageText(PSZUC pszMessage)
	{
	Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD new CEventMessageTextSent(pszMessage));
	}


//	Upload a file to the contact.
//	The file name is expected to be a local path.
void
ITreeItemChatLogEvents::Xmpp_SendEventFileUpload(PSZUC pszFileUpload)
	{
	Assert(m_pawLayoutChatLog != NULL);
	if (pszFileUpload == NULL || pszFileUpload[0] == '\0')
		return;
	Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD new CEventFileSent(pszFileUpload));
	}

//	Upload multiple files.
//	This method expect files to be in the 'URL' with the prefix "file://"
void
ITreeItemChatLogEvents::Xmpp_SendEventsFileUpload(IN_MOD_INV PSZU pszmFilesUpload)
	{
	Assert(pszmFilesUpload != NULL);
	PSZUC pszFileUpload = pszmFilesUpload;
	while (TRUE)
		{
		UINT ch = *pszmFilesUpload++;
		if (ch == '\n' || ch == '\0')
			{
			pszmFilesUpload[-1] = '\0';	// Insert a null-terminator
			CStr strFile = QUrl::fromUserInput(CString(pszFileUpload)).toLocalFile();	// This code is grossly inefficient, however necessary because Qt uses triple slashes (///) after the schema, such as: "file:///c:/folder/file.txt".  I could manually skip the extra slash, however it may break under other platrorms.
			MessageLog_AppendTextFormatCo(d_coBlack, "XmppUploadFiles() - $S\n", &strFile);
			Xmpp_SendEventFileUpload(strFile);
			if (ch == '\0')
				return;	// We are done
			pszFileUpload = pszmFilesUpload;
			}
		}
	}

void
ITreeItemChatLogEvents::DisplayDialogSendFile()
	{
	CStr strCaption;
	strCaption.Format("Send file to $s", TreeItem_PszGetNameDisplay());
	strCaption = QFileDialog::getOpenFileName(g_pwMainWindow, IN strCaption);
	Xmpp_SendEventFileUpload(strCaption);	// An empty filename will be ignored by Xmpp_SendEventFileUpload()
	}


CSocketXmpp *
ITreeItemChatLogEvents::Xmpp_PGetSocketOnlyIfReady() const
	{
	return m_pAccount->Socket_PGetOnlyIfReadyToSendMessages();
	}

CSocketXmpp *
TContact::Xmpp_PGetSocketOnlyIfContactIsUnableToCommunicateViaXcp() const
	{
	if (m_cVersionXCP <= 0)
		return m_pAccount->Socket_PGetOnlyIfReadyToSendMessages();
	return NULL;
	}

void
ITreeItemChatLogEvents::Xmpp_Ping()
	{
	Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD new CEventPing);	// Pinging a group makes little sense, however there is no harm.  The first group member responding to the ping will set the timestamp.
	}

void
ITreeItemChatLogEvents::Xmpp_QueryVersion()
	{
	Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD new CEventVersion);
	}

