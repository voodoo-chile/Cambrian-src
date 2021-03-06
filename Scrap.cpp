//	Code no longer used, however may be useful later


IEvent *
CVaultEvents::PFindEventLargerThanTimestamp(TIMESTAMP tsEventID) CONST_MCC
	{
	Endorse(tsEventID == d_ts_zNULL);	// Return the first event
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->m_tsEventID > tsEventID)
			return pEvent;
		} // while
	return NULL;
	}

IEvent *
CVaultEvents::PFindEventLargerThanTimestampOther(TIMESTAMP tsOther) CONST_MCC
	{
	Endorse(tsOther == d_ts_zNULL);	// Return the first event
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		Assert(pEvent->m_tsOther != d_ts_zNULL);	// Not sure about this Assert()
		if (pEvent->m_tsOther > tsOther)
			return pEvent;
		} // while
	return NULL;
	}

//	Find the last group event received
IEvent *
CVaultEvents::PFindEventReceivedLastMatchingContactSender(TContact * pContactGroupSender) CONST_MCC
	{
	Assert(pContactGroupSender != NULL);
	Assert(pContactGroupSender->EGetRuntimeClass() == RTI(TContact));
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->m_pContactGroupSender_YZ == pContactGroupSender)
			{
			Assert(pEvent->m_tsOther > d_tsOther_kmReserved);
			return pEvent;
			}
		} // while
	return NULL;
	}

//	Return the first event larger than the timestamp
IEvent *
CVaultEvents::PFindEventSentLargerThanTimestamp(TIMESTAMP tsEventID) CONST_MCC
	{
	Endorse(tsEventID == d_ts_zNULL);	// Return the first event sent
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *ppEvent++;
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->Event_FIsEventTypeSent())
			{
			if (pEvent->m_tsEventID > tsEventID)
				return pEvent;
			}
		} // while
	return NULL;
	}

IEvent *
CVaultEvents::PFindEventByTimestampOther(TIMESTAMP tsOther) CONST_MCC
	{
	if (tsOther != d_ts_zNULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			Assert(pEvent->m_tsEventID != d_ts_zNULL);
			Assert(pEvent->m_tsOther != d_ts_zNULL);
			if (pEvent->m_tsOther <= tsOther)
				{
				if (pEvent->m_tsOther == tsOther)
					return pEvent;
				break;
				}
			} // while
		} // if
	return NULL;
	}

//	Return the event received matching tsOther
IEvent *
CVaultEvents::PFindEventReceivedByTimestampOther(TIMESTAMP tsOther) CONST_MCC
	{
	if (tsOther != d_ts_zNULL)
		{
		IEvent ** ppEventStop;
		IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
		while (ppEvent != ppEventStop)
			{
			IEvent * pEvent = *--ppEventStop;
			Assert(pEvent->m_tsEventID != d_ts_zNULL);
			if (!pEvent->Event_FIsEventTypeReceived())
				continue;
			Assert(pEvent->m_tsOther != d_ts_zNULL);
			if (pEvent->m_tsOther <= tsOther)
				{
				if (pEvent->m_tsOther == tsOther)
					return pEvent;
				break;
				}
			} // while
		} // if
	return NULL;
	}

IEvent *
CArrayPtrEvents::PFindEventLastReceived() const
	{
	IEvent ** ppEventStop;
	IEvent ** ppEvent = PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		AssertValidEvent(pEvent);
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->Event_FIsEventTypeReceived())
			return pEvent;
		}
	return NULL;
	}

//	Filter the events from the QLineEdit
bool
WNavigationTreeCaption::eventFilter(QObject * obj, QEvent *event)
	{
	if (event->type() == QEvent::KeyPress)
		{
		QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->key() == Qt::Key_Escape)
			{
			// MessageLog_AppendTextFormatCo(d_coRed, "WNavigationTreeCaption::eventFilter() - Escape key pressed\n");

			}
		}
	return QWidget::eventFilter(obj, event);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core method to serialize the data for an XCP API request.
void
CBinXcpStanza::BinXmlAppendXcpElementForApiRequest_AppendApiParameterData(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiParameters != NULL);
	Assert(XcpApi_FIsXmlElementOpened());
	Assert(m_pContact != NULL);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	MessageLog_AppendTextFormatSev(eSeverityComment, "Processing API '$s' with the following parameters: ^N\n", pszApiName, pXmlNodeApiParameters);

	/*
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Synchronize))
		{
		BinXmlAppendXcpApiSynchronize_OnReply(pXmlNodeApiParameters);
		return;
		}
	*/
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Group_Profile_Get))
		{
		PSZUC pszProfileIdentifier = pXmlNodeApiParameters->m_pszuTagValue;
		TGroup * pGroup = pAccount->Group_PFindByIdentifier_YZ(pszProfileIdentifier, INOUT this, TAccountXmpp::eFindGroupOnly);
		if (pGroup != NULL)
			{
			pGroup->XcpApiGroup_ProfileSerialize(INOUT this);
			return;
			}
		Assert(XcpApi_FIsXmlElementClosedBecauseOfError());
		return;
		}
	if (FCompareStringsNoCase(pszApiName, c_szaApi_Contact_Recommendations_Get))
		{
		pAccount->m_pProfileParent->XcpApiProfile_RecommendationsSerialize(INOUT this); // Return all the recommendations related to the profile
		return;
		}

	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown API '$s'\n", pszApiName);
	} // BinXmlAppendXcpElementForApiRequest_AppendApiParameterData()


//	Method to unserialize the data from BinXmlAppendXcpElementForApiRequest_AppendApiParameterData().
//	Although the reply typically does not need the object CBinXcpStanza, the reply may necessitate the CBinXcpStanza to make additional request.
//	For instance, when unserializing a group, the method may need to query information about new group members to fetch the contact profile.
void
CBinXcpStanza::BinXmlAppendXcpElementForApiReply(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiParameters != NULL);
	Assert(m_pContact != NULL);
	MessageLog_AppendTextFormatSev(eSeverityComment, "BinXmlAppendXcpElementForApiReply($s):\n^N\n", pszApiName, pXmlNodeApiParameters);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	if (FCompareStringsNoCase(pszApiName, (PSZUC)c_szaApi_Group_Profile_Get))
		{
		// We received a profile information
		PSZUC pszGroupIdentifier = pXmlNodeApiParameters->PszuFindAttributeValue_NZ(d_chAPIa_TGroup_shaIdentifier);
		//MessageLog_AppendTextFormatCo(d_coRed, "BinXmlAppendXcpElementForApiReply($s) - Group '$s'\n", pszApiName, pszGroupIdentifier);
		TGroup * pGroup = pAccount->Group_PFindByIdentifier_YZ(pszGroupIdentifier, INOUT this, TAccountXmpp::eFindGroupOnly);
		if (pGroup != NULL)
			{
			pGroup->XcpApiGroup_ProfileUnserialize(IN pXmlNodeApiParameters, INOUT this);
			}
		return;
		}
	if (FCompareStringsNoCase(pszApiName, (PSZUC)c_szaApi_Contact_Recommendations_Get))
		{
		// We received new recommendations from the contact
		const_cast<CXmlNode *>(pXmlNodeApiParameters)->RemoveEmptyElements();
		m_pContact->Contact_RecommendationsUpdateFromXml(IN pXmlNodeApiParameters);
		return;
		}

	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown API: $s\n^N", pszApiName, pXmlNodeApiParameters);
	}

/*
void
CBinXcpStanza::BinXmlAppendXcpApiSynchronize_OnRequest(const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(m_pContact != NULL);
	BinXmlAppendXcpElementForApiRequest_ElementOpen(c_szaApi_Synchronize);
	CVaultEvents * pVault = m_pContact->Vault_PGet_NZ();
	CXmlNode * pXmlNodeSent = pXmlNodeApiParameters->PFindElement('s');
	if (pXmlNodeSent != NULL)
		{
		TIMESTAMP tsEventID = pXmlNodeSent->TsGetAttributeValueTimestamp_ML('i');
		// Return all the timestamps of sent events since tsEventID
		BinAppendText_VE("<s o='$t'/>", tsEventID);
		}
	BinXmlAppendXcpElementForApiRequest_ElementClose();
	}

void
CBinXcpStanza::BinXmlAppendXcpApiSynchronize_OnReply(const CXmlNode * pXmlNodeApiParameters)
	{

	}
*/


//	Before allocating a CDataXmlLargeEvent, check if it is not already there.
//	This is important because if we are sending a message to a group, then the same CDataXmlLargeEvent is reused, otherwise
//	there will be a copy of CDataXmlLargeEvent for each group member.  Just imagine sending a large stanza of 100 KiB to a group of 1000 people.
CDataXmlLargeEvent *
CVaultEvents::PFindOrAllocateDataXmlLargeEvent_NZ(TIMESTAMP tsEventID, IN_MOD_TMP CBinXcpStanza * pbinXcpStanza)
	{
	Assert(tsEventID > d_ts_zNULL);
	Assert(pbinXcpStanza != NULL);
	Assert(pbinXcpStanza->m_pContact != NULL);
	Assert(pbinXcpStanza->m_pContact->EGetRuntimeClass() == RTI(TContact));
	Assert(m_pParent != NULL);
	Assert(m_pParent->EGetRuntimeClass() == RTI(TContact) || m_pParent->EGetRuntimeClass() == RTI(TGroup));
	IEvent * pEvent;

	// First, search the list of large stanza
	CListaDataXmlLargeEvents * plistaDataXmlLargeEvents = &m_pParent->m_pAccount->m_listaDataXmlLargeEvents;
	CDataXmlLargeEvent * pDataXmlLargeEvent = (CDataXmlLargeEvent *)plistaDataXmlLargeEvents->pHead;
	while (pDataXmlLargeEvent != NULL)
		{
		if (pDataXmlLargeEvent->m_tsEventID == tsEventID)
			{
			MessageLog_AppendTextFormatSev(eSeverityComment, "PFindOrAllocateDataXmlLargeEvent_NZ() - Found existing CDataXmlLargeEvent $t  ($I bytes)\n", tsEventID, pDataXmlLargeEvent->m_binXmlData.CbGetData());
			plistaDataXmlLargeEvents->MoveNodeToHead(INOUT pDataXmlLargeEvent);	// Move the node at the beginning of the list so next time its access is faster (this is a form of caching, in case there are many pending CDataXmlLargeEvent)
			goto Done;
			}
		pDataXmlLargeEvent = (CDataXmlLargeEvent *)pDataXmlLargeEvent->pNext;
		} // while
	// We have been unable to find a CDataXmlLargeEvent matching tsEventID, therefore we need to allocate a new one
	pDataXmlLargeEvent = new CDataXmlLargeEvent;
	pDataXmlLargeEvent->m_tsEventID = tsEventID;

	// We need to serialize the event, and then save the blob
	pEvent = PFindEventByID(tsEventID);
	if (pEvent != NULL)
		{
		const int ibXmlDataStart = pbinXcpStanza->CbGetData();
		//Assert(ibXmlDataStart > 0);
		/*
		//pbinXcpStanza->BinAppendText_VE("<$U" _tsO, pEvent->EGetEventClassForXCP(pbinXcpStanza->m_pContact), pEvent->m_tsEventID);	// Do not serialize m_tsOther, as the CDownloader will have it.  Same for the group identifier as well as m_pContactGroupSender_YZ
		pbinXcpStanza->BinAppendText_VE("<$U", pEvent->EGetEventClassForXCP(pbinXcpStanza->m_pContact));	// No need to serialize the timestamps, group identifier, or m_pContactGroupSender_YZ, because the CEventDownloader has this information
		pEvent->XmlSerializeCore(IOUT pbinXcpStanza);
		pbinXcpStanza->BinAppendXmlForSelfClosingElement();
		*/
		pbinXcpStanza->BinXmlSerializeEventForXcpCore(pEvent, d_ts_zNULL);
		Assert(pbinXcpStanza->m_pContact != NULL);	// Sometimes XmlSerializeCore() may modify m_pContact, however it should be only for contacts not supporting XCP (which should not be the case here!)
		int cbXmlData = pbinXcpStanza->CbGetData() - ibXmlDataStart;
		pDataXmlLargeEvent->m_binXmlData.BinInitFromBinaryData(IN pbinXcpStanza->DataTruncateAtOffsetPv(ibXmlDataStart), cbXmlData);	// Make a copy of the serialized data, and restore CBinXcpStanza to its original state
		MessageLog_AppendTextFormatSev(eSeverityComment, "PFindOrAllocateDataXmlLargeEvent_NZ() - Initializing cache by serializing tsEventID $t (tsOther $t) -> $I bytes of data:\n$B\n", tsEventID, pEvent->m_tsOther, cbXmlData, &pDataXmlLargeEvent->m_binXmlData);
		}
	else
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "PFindOrAllocateDataXmlLargeEvent_NZ() - Unable to find tsEventID $t ({tL}), therefore initializing an empty cache entry!\n", tsEventID, tsEventID);
		}
	HashSha1_CalculateFromCBin(OUT &pDataXmlLargeEvent->m_hashXmlData, IN pDataXmlLargeEvent->m_binXmlData);	// Calculate the checksum for the data
	plistaDataXmlLargeEvents->InsertNodeAtHead(PA_CHILD pDataXmlLargeEvent);	// Insert at the head, as the event is most likely to be accessed within the next minutes

	Done:
	pDataXmlLargeEvent->m_tsmLastAccessed = g_tsmMinutesSinceApplicationStarted;
	MessageLog_AppendTextFormatSev(eSeverityNoise, "PFindOrAllocateDataXmlLargeEvent_NZ() returning CDataXmlLargeEvent $t of $I bytes (m_tsmLastAccessed=$I)\n", tsEventID, pDataXmlLargeEvent->m_binXmlData.CbGetData(), pDataXmlLargeEvent->m_tsmLastAccessed);
	return pDataXmlLargeEvent;
	} // PFindOrAllocateDataXmlLargeEvent_NZ()


CListaDataXmlLargeEvents::~CListaDataXmlLargeEvents()
	{
	// Destroy the list
	CDataXmlLargeEvent * pDataXmlLargeEvent = (CDataXmlLargeEvent *)pHead;
	while (pDataXmlLargeEvent != NULL)
		{
		CDataXmlLargeEvent * pDataXmlLargeEventNext = (CDataXmlLargeEvent *)pDataXmlLargeEvent->pNext;
		delete pDataXmlLargeEvent;
		pDataXmlLargeEvent = pDataXmlLargeEventNext;
		} // while
	}

void
CListaDataXmlLargeEvents::DeleteIdleNodes()
	{
	// Flush any cache entry idle for more than a number of minutes
	#define d_cMinutesIdleForCDataXmlLargeEvent		0		// For debugging, use zero, however for a real world situation, 10 minutes would be a good guess

	CDataXmlLargeEvent * pDataXmlLargeEvent = (CDataXmlLargeEvent *)pHead;
	while (pDataXmlLargeEvent != NULL)
		{
		CDataXmlLargeEvent * pDataXmlLargeEventNext = (CDataXmlLargeEvent *)pDataXmlLargeEvent->pNext;
		if (pDataXmlLargeEvent->m_tsmLastAccessed + d_cMinutesIdleForCDataXmlLargeEvent < g_tsmMinutesSinceApplicationStarted)
			{
			MessageLog_AppendTextFormatSev(eSeverityNoise, "[$@] CListaDataXmlLargeEvents::DeleteIdleNodes() - Deleting cache entry for tsEventID $t\n", pDataXmlLargeEvent->m_tsEventID);	// This is not an error, however good to draw attention
			DetachNode(INOUT pDataXmlLargeEvent);
			delete pDataXmlLargeEvent;
			}
		pDataXmlLargeEvent = pDataXmlLargeEventNext;
		} // while
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
CEventDownloader::CEventDownloader(const TIMESTAMP * ptsEventID) : IEvent(ptsEventID)
	{
	m_tsForwarded = d_ts_zNA;
	m_cbDataToDownload = 0;
	m_paEvent = NULL;
	}

CEventDownloader::~CEventDownloader()
	{
	delete m_paEvent;
	}

//	Return the class of the downloader.
//	It is important to return the true event class because there are many instances of the code where a typecast is made based on the event class.
EEventClass
CEventDownloader::EGetEventClass() const
	{
	return c_eEventClass;
	}

//	Return the event class to serialize the event via XCP.
//	In this virtual method, it is appropriate to substitute the class by the downloaded event if available because no typecast is made based on this method.
EEventClass
CEventDownloader::EGetEventClassForXCP() const
	{
	if (m_paEvent != NULL)
		return m_paEvent->EGetEventClassForXCP();
	return c_eEventClass;	// This value does not serialize via XCP
	}

//	CEventDownloader::IEvent::XmlSerializeCoreE()
EXml
CEventDownloader::XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const
	{
	if (m_paEvent != NULL)
		{
		return m_paEvent->XmlSerializeCoreE(IOUT pbinXmlAttributes);
		}
	pbinXmlAttributes->BinAppendXmlAttributeTimestamp(d_chXCPa_CEventDownloader_tsForwarded, m_tsForwarded);
	pbinXmlAttributes->BinAppendXmlAttributeInt(d_chXCPa_CEventDownloader_cblDataToDownload, m_cbDataToDownload);
	pbinXmlAttributes->BinAppendXmlAttributeCBin(d_chXCPa_CEventDownloader_bin85DataReceived, m_binDataDownloaded);
	return eXml_zAttributesOnly;
	}

//	CEventDownloader::IEvent::XmlUnserializeCore()
void
CEventDownloader::XmlUnserializeCore(const CXmlNode * pXmlNodeElement)
	{
	if (m_paEvent != NULL)
		{
		m_paEvent->XmlUnserializeCore(IN pXmlNodeElement);	// This happens when synchronizing
		return;
		}
	pXmlNodeElement->UpdateAttributeValueTimestamp(d_chXCPa_CEventDownloader_tsForwarded, OUT_F_UNCH &m_tsForwarded);
	pXmlNodeElement->UpdateAttributeValueInt(d_chXCPa_CEventDownloader_cblDataToDownload, OUT_F_UNCH &m_cbDataToDownload);
	pXmlNodeElement->UpdateAttributeValueCBin(d_chXCPa_CEventDownloader_bin85DataReceived, OUT_F_UNCH &m_binDataDownloaded);
	}

void
CEventDownloader::XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanza * pbinXcpStanzaReply)
	{
	if (m_paEvent != NULL)
		m_paEvent->XcpExtraDataRequest(pXmlNodeExtraData, pbinXcpStanzaReply);
	}

void
CEventDownloader::XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanza * pbinXcpStanzaReply)
	{
	if (m_paEvent != NULL)
		m_paEvent->XcpExtraDataArrived(pXmlNodeExtraData, pbinXcpStanzaReply);
	}

void
CEventDownloader::ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE
	{
	Assert(m_tsEventID > d_tsOther_kmReserved);
	if (m_paEvent == NULL)
		{
		// Show progress of the download
		int cbDataDownloaded = m_binDataDownloaded.CbGetData();
		if (cbDataDownloaded != m_cbDataToDownload)
			{
			_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
			g_strScratchBufferStatusBar.BinAppendText_VE("receiving large data... {kT}", cbDataDownloaded, m_cbDataToDownload);
			//g_strScratchBufferStatusBar.BinAppendTextBytesKiBPercentProgress(cbDataDownloaded, m_cbDataToDownload);
			poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, QBrush(d_coSilver));

			// Request the next block of data
			MessageLog_AppendTextFormatSev(eSeverityComment, "CEventDownloader::ChatLogUpdateTextBlock() - Event tsOther $t is requesting data at offset $I (until $I)\n", m_tsOther, cbDataDownloaded, m_cbDataToDownload);
			CBinXcpStanzaTypeInfo binXcpStanza(this);
			binXcpStanza.BinAppendText_VE("<" d_szXCPe_EventSplitDataRequest_tsI_i "/>", m_tsOther, cbDataDownloaded);
			binXcpStanza.XcpSendStanza();
			return;
			}
		// We are done downloading, therefore allocate the event by unserializing the downloaded XML data
		MessageLog_AppendTextFormatSev(eSeverityComment, "CEventDownloader::ChatLogUpdateTextBlock() - Event tsOther $t completed with $I bytes:\n$B\n", m_tsOther, cbDataDownloaded, &m_binDataDownloaded);
		m_pVaultParent_NZ->SetModified();	// Make sure whatever we do next will be saved to disk

		CXmlTree oXmlTree;
		if (oXmlTree.EParseFileDataToXmlNodesCopy_ML(IN m_binDataDownloaded) == errSuccess)
			{
			m_paEvent = IEvent::S_PaAllocateEvent_YZ(IN &oXmlTree, IN &m_tsEventID);
			if (m_paEvent != NULL)
				{
				// We have successfully re-created a blank event class matching the blueprint of the downloader.  Now, we need to initialize the event variables.
				m_paEvent->m_pVaultParent_NZ = m_pVaultParent_NZ;		// The new event uses the same vault as the downloader
				Assert(m_paEvent->m_pContactGroupSender_YZ == NULL);	// New allocated events do not have any contact group sender yet
				m_paEvent->m_pContactGroupSender_YZ = PGetAccount_NZ()->Contact_PFindByIdentifierGroupSender_YZ(IN &oXmlTree);	// The contact who created the event may be different than the contact who transmitted the event, as the [large] event may be have been forwarded
				if (m_paEvent->m_pContactGroupSender_YZ == NULL)
					m_paEvent->m_pContactGroupSender_YZ = m_pContactGroupSender_YZ;	// If the contact sender was not present in the downloaded XML, use the contact of the downloader (if any)
				Assert(m_paEvent->m_tsOther == d_ts_zNULL);
				//oXmlTree.UpdateAttributeValueTimestamp(d_chEvent_Attribute_tsOther, OUT_F_UNCH &m_paEvent->m_tsOther);	// Make sure m_tsOther is always unserialized.  This line is somewhat similar as EventsUnserializeFromDisk()
				//Assert(oXmlTree.TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther) == d_ts_zNA);
				/*
				m_paEvent->m_tsOther = m_tsOther;	// Since tsOther is never serialized in the cache, just initialize its value from the downloader
				Assert(m_paEvent->m_tsOther > d_tsOther_kmReserved);
				*/
				MessageLog_AppendTextFormatCo(d_coOrange, "CEventDownloader::m_tsOther $t, m_tsForwarded $t\n", m_tsOther, m_tsForwarded);
				m_paEvent->m_tsOther = m_tsForwarded;
				if (m_paEvent->m_tsOther == d_ts_zNULL)
					m_paEvent->m_tsOther = m_tsOther;
				Assert(m_paEvent->m_tsOther > d_tsOther_kmReserved);
				m_paEvent->XmlUnserializeCore(IN &oXmlTree);	// Finally, unserialize the data specific to the event
				goto EventUpdateToGUI;
				}
			else
				{
				if (m_uFlagsEvent & FE_kfEventProtocolWarning)
					m_uFlagsEvent |= FE_kfEventProtocolError;	// Upgrade the 'warning' to an 'error'
				m_uFlagsEvent |= FE_kfEventProtocolWarning;
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "[$s] Unable to allocate event '$s' $t (tsOther = $t) from XML data of CEventDownloader (m_cbDataToDownload = $I): ^N",
					(m_uFlagsEvent & FE_kfEventProtocolError) ? "Error" : "Warnign", oXmlTree.m_pszuTagName, m_tsEventID, m_tsOther, m_cbDataToDownload, &oXmlTree);
				goto EventDisplayError;
				}
			}
		// The data is not good (probably corrupted), so display a notification and flush the data
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "CEventDownloader::ChatLogUpdateTextBlock() - Data corrupted for event tsOther $t, therefore flushing data.\n", m_tsOther);
		EventDisplayError:
		_BinHtmlInitWithTime(OUT &g_strScratchBufferStatusBar);
		g_strScratchBufferStatusBar.BinAppendText_VE("<span style='color:red'>Error allocating event $s of {kK}", oXmlTree.m_pszuTagName, cbDataDownloaded);
		poCursorTextBlock->InsertHtmlBin(g_strScratchBufferStatusBar, QBrush(d_coSilver));
		m_binDataDownloaded.Empty();
		m_cbDataToDownload = c_cbDataToDownload_Error;
		}
	else
		{
		EventUpdateToGUI:
		m_paEvent->ChatLogUpdateTextBlock(INOUT poCursorTextBlock);
		} // if...else
	} // ChatLogUpdateTextBlock()

void
CEventDownloader::XcpDownloadedDataArrived(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXcpStanzaReply, QTextEdit * pwEditChatLog)
	{
	const int ibDataNew = pXmlNodeData->LFindAttributeXcpOffset();
	const int ibDataRequested = m_binDataDownloaded.CbGetData();
	if (ibDataNew == ibDataRequested)
		{
		m_binDataDownloaded.BinAppendBinaryDataFromBase85Szv_ML(pXmlNodeData->PszuFindAttributeValue(d_chXCPa_EventExtraData_bin85Payload));	// Append new binary data
		int cbDataNew = m_binDataDownloaded.CbGetData();
		SHashSha1 hashDataChecksum;
		if (pXmlNodeData->UpdateAttributeValueHashSha1(d_chXCPa_EventExtraData_shaData, OUT_F_UNCH &hashDataChecksum))
			{
			// If the hash is present, it means we have received everyting.  All we need is to compare if the value is good
			if (m_binDataDownloaded.FCompareFingerprint(IN hashDataChecksum))
				{
				// We have the correct fingerprint, meaning the data is valid
				if (cbDataNew != m_cbDataToDownload)
					{
					if ((m_uFlagsEvent & FE_kfEventProtocolWarning) == 0)
						{
						m_uFlagsEvent |= FE_kfEventProtocolWarning;
						MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "[Warning] CEventDownloader tsEventID $t: Adjusting m_cbDataToDownload from $I to $I.\n", m_tsEventID, m_cbDataToDownload, cbDataNew);
						m_cbDataToDownload = cbDataNew;
						}
					}
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "CEventDownloader tsEventID $t: Fingerprint mismatch for $I bytes of data\n", m_tsEventID, cbDataNew);
				if ((m_uFlagsEvent & FE_kfEventProtocolWarning) == 0)
					{
					m_uFlagsEvent |= FE_kfEventProtocolWarning;
					m_binDataDownloaded.Empty();	// Flush what we downloaded, and try again
					cbDataNew = 0;
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t [Warning]: CEventDownloader tsEventID $t: Retrying to download again...\n", m_tsEventID);
					}
				} // if...else
			goto UpdateGUI;	// Always update the GUI when the hash is present
			} // if
		if (cbDataNew > ibDataRequested)
			{
			// The content m_binDataDownloaded grew (which means received new data), therefore update the GUI if available, otherwise request another chunk
			UpdateGUI:
			if (pwEditChatLog != NULL)
				ChatLog_UpdateEventWithinWidget(pwEditChatLog);	// Update the UI (which will requests another chunk)
			else
				pbinXcpStanzaReply->BinAppendText_VE("<" d_szXCPe_EventSplitDataRequest_tsI_i "/>", m_tsOther, cbDataNew);
			m_pVaultParent_NZ->SetModified();	// Make sure the new data gets saved to disk
			}
		else
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "CEventDownloader tsEventID $t: Ignoring empty stanza (cbDataDownloaded=$I, m_cbDataToDownload=$I)\n", m_tsEventID, ibDataRequested, m_cbDataToDownload);
		}
	else
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "CEventDownloader tsEventID $t: Ignoring data because its offset $I does not match the requested at $I\n", m_tsEventID, ibDataNew, ibDataRequested);
	} // XcpDownloadedDataArrived()

void
CEventDownloader::HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText)
	{
	if (m_paEvent != NULL)
		m_paEvent->HyperlinkGetTooltipText(pszActionOfHyperlink, IOUT pstrTooltipText);
	}

void
CEventDownloader::HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock)
	{
	if (m_paEvent != NULL)
		m_paEvent->HyperlinkClicked(pszActionOfHyperlink, INOUT poCursorTextBlock);
	}


CEventDownloader *
CVaultEvents::PFindEventDownloaderMatchingEvent(const IEvent * pEvent) const
	{
	Assert(pEvent != NULL);
	Assert(pEvent->EGetEventClass() != CEventDownloader::c_eEventClass);
	IEvent ** ppEventStop;
	IEvent ** ppEvent = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		CEventDownloader * pEventDownloader = (CEventDownloader *)*--ppEventStop;
		if (pEventDownloader->EGetEventClass() == CEventDownloader::c_eEventClass)
			{
			if (pEventDownloader->FIsDownloaderMatchingEvent(pEvent))
				return pEventDownloader;
			}
		}
	return NULL;
	}


/*
#define d_chXCPe_EventPrevious								'p'	// Send the timestamp of the previous event, as well as the synchronization
#define d_szXCPe_EventPrevious_tsO_tsI				d_szXCP_"p"	_tsO _tsI
	#define d_szXCPa_EventPrevious_cEventsMissing				" c='$i'"
	#define d_chXCPa_EventPrevious_cEventsMissing				'c'
#define d_chXCPe_EventsMissing								'M'	// Notification to the remote client some events are missing, and therefore should synchronize
#define d_szXCPe_EventsMissing						d_szXCP_"M"

#define d_chXCPe_EventNextRequest							'n'	// Ask the remote client to send its next event after tsEventID
#define d_szXCPe_EventNextRequest_tsI				d_szXCP_"n"	_tsI
#define d_chXCPe_EventNextReply								'N'	// Reply to the sender regarding the request of tsEventID and tsOther (the following xcp-stanzas contains serialized IEvents)
#define d_szXCPe_EventNextReply_tsO					d_szXCP_"N" _tsO
#define d_chXCPe_EventsOutstandingReply						'O'	// Any message reply 'N' must end with 'O' to indicate hoe many outstanding IEvents remain.
#define d_szXCPe_EventsOutstandingReply_i_tsO		d_szXCP_"O c='$i'" _tsO
	#define d_chXCPa_EventsOutstandingReply_cEvents				'c'

#define d_chXCPe_EventConfirmation							'C'	// Confirmation an event was processed successfully (the confirmation only includes tsEventID of the sender)
#define d_szXCPe_EventConfirmation_tsI				d_szXCP_"C" _tsI
#define d_chXCPe_EventError									'E'	// The event was received, however could not be processed because its class/type is unknown. 'C' and 'E' are mutually exclusive.
#define d_szXCPe_EventError_tsI_s					d_szXCP_"E" _tsI " e='^s'"	// e=event, c=code of the error (if any), t=friendly text to display to the user (if any)

#define d_chXCPe_EventForwardRequest						'f'	// Ask the remote client to forward a group event sent by someone else.  This is somewhat similar to d_chXCPe_EventNextRequest, however since forwarded events are not synchronized, Cambrian cannot use d_chXCPe_EventNextRequest.
#define d_szXCPe_EventForwardRequest_tsI			d_szXCP_"f"	_tsI
#define d_chXCPe_EventForwardReply							'F'
#define d_szXCPe_EventForwardReply_tsO				d_szXCP_"F" _tsO
#define d_chXCPe_EventsForwardRemaining						'R'
#define d_szXCPe_EventsForwardRemaining_i_tsO		d_szXCP_"R c='$i'" _tsO
	#define d_chXCPa_EventsForwardRemaining_cEvents				'c'

#define d_chXCPe_EventExtraDataRequest						'x'	// Request an event to send extra data (for instance, downloading the content of a file)
#define d_szXCPe_EventExtraDataRequest_tsI			d_szXCP_"x" _tsI
#define d_chXCPe_EventExtraDataReply						'X'	// Reply from the event with the data payload
#define d_szXCPe_EventExtraDataReply				d_szXCP_"X"
#define d_szXCPe_EventExtraDataReply_tsO			d_szXCP_"X" _tsO

	#define d_chXCPa_EventExtraData_strxIdentifier				'I'		// Identifier of the extra data (this is optional, as sometimes there is only one extra data)
	#define d_chXCPa_EventExtraData_iblOffset					'O'		// Offset of the extra data (this is important, because the extra data is sent by smaller pieced of about 4 KiB)
	#define d_szXCPa_EventExtraData_iblOffset_i					" O='$i'"	// 32 bit offset
	#define d_szXCPa_EventExtraData_iblOffset_l					" O='$l'"	// 64 bit offset
	#define d_chXCPa_EventExtraData_bin85Payload				'b'			// Binary payload
	#define d_szXCPa_EventExtraData_bin85Payload_pvcb			" b='{p|}'"
	#define d_chXCPa_EventExtraData_shaData						'h'	// SHA-1 of the data (to ensure its integrity)
	#define d_szXCPa_EventExtraData_shaData_h					" h='{h|}'"

#define d_chXCPe_EventSplitDataRequest						's'
#define d_szXCPe_EventSplitDataRequest_tsI_i		d_szXCP_"s" _tsI d_szXCPa_EventExtraData_iblOffset_i
#define d_chXCPe_EventSplitDataReply						'S'
#define d_szXCPe_EventSplitDataReply_tsO_i_pvcb		d_szXCP_"S" _tsO d_szXCPa_EventExtraData_iblOffset_i d_szXCPa_EventExtraData_bin85Payload_pvcb

//	Tasks have only one identifier (there is no tsOther) for tasks
#define d_chXCPe_TaskDownloading							't'
#define d_szXCPe_TaskDownloading_tsI				d_szXCP_"t" _tsI
#define d_chXCPe_TaskSending								'T'
#define d_szXCPe_TaskSending_tsI					d_szXCP_"T" _tsI

	#define d_chXCPa_TaskDataSizeTotal							's'			// Total size of the task's data
	#define d_szXCPa_TaskDataSizeTotal_i							" s='$i'"
	#define d_chXCPa_TaskDataOffset									'O'
	#define d_szXCPa_TaskDataOffset_i								" O='$i'"
	#define d_chXCPa_TaskDataBinary									'b'			// Chunk of data
	#define d_szXCPa_TaskDataBinary_Bii								" b='{o|}'"
//	#define d_szXCPa_TaskDataBinary_pvcb							" b='{p|}'"

*/

#if 0
	// API call
	<_a r='identifier' n='function'/>parameters</_a>
	<_A r='identifier' e='error (if any)'>result</_A>

#endif
#define d_chXCPe_zNA									'\0'	// Not applicable

/*
void
CBinXcpStanza::BinXmlAppendXcpElementForApiRequest_ElementOpen(PSZUC pszApiName)
	{
	Assert(m_ibXmlApiReply == d_zNA);
	BinAppendText_VE("<" d_szXCPe_ApiReply_s ">", pszApiName);	// Open the XML tag
	m_ibXmlApiReply = m_paData->cbData;
	Assert(XcpApi_FIsXmlElementOpened());
	}

void
CBinXcpStanza::BinXmlAppendXcpElementForApiRequest_ElementClose()
	{
	if (m_ibXmlApiReply <= 0)
		return;
	BinAppendText("</" d_szXCPe_ApiReply_close ">");	// Close the XML tag
	m_ibXmlApiReply = d_zNA;
	}
*/


#if 0
///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core method of the Cambrian Protocol (Version 1).
//
//	This method is related to a contact receiving an XCP stanza.
//	Depending on the context, the events may be assigned to the contact, or the group where the contact is member.
//
//	THIS METHOD HAS BEEN REPLACED WITH CBinXcpStanza::XcpApi_ExecuteList().
//	THIS CODE IS KEPT FOR BACKWARD COMPATIBILITY UNTIL EVERYONE USES PROTOCOL VERSION 2.
void
TContact::Xcp_ProcessStanzasAndUnserializeEvents(const CXmlNode * pXmlNodeXcpEvent)
	{
	Assert(pXmlNodeXcpEvent != NULL);
	PSZUC pszNameDisplay = TreeItem_PszGetNameDisplay();		// To help debugging
	ITreeItemChatLogEvents * pChatLogEvents = this;				// Which Chat Log to display the new events (by default, it is the contact receiving the stanza, however it may be the group)
	WChatLog * pwChatLog = ChatLog_PwGet_YZ();
	CVaultEvents * pVault = Vault_PGet_NZ();					// By default, use the vault of the contact.  If the message is for a group, then change the vault and the synchronization timestamps.
	TIMESTAMP * ptsOtherLastSynchronized = &m_tsOtherLastSynchronized;	// When the data was last synchronized with the remote contact
	MessageLog_AppendTextFormatCo(d_coBlack, "Xcp_ProcessStanzasAndUnserializeEvents(Account: $s, Peer: $s - $S)\n\t m_tsOtherLastSynchronized=$t, m_tsEventIdLastSentCached=$t\n",
		m_pAccount->TreeItem_PszGetNameDisplay(), pszNameDisplay, &m_strJidBare, m_tsOtherLastSynchronized, m_tsEventIdLastSentCached);
	CBinXcpStanzaTypeInfo binXcpStanzaReply;	// What stanza to send to the remote client as a response to pXmlNodeXcpEvent
	binXcpStanzaReply.m_pContact = this;
	int cbXcpStanzaReplyEmpty = 0;				// What bytes are considered 'empty' within the binXcpStanzaReply.  This variable is necessary because of group chat, where the group identifier must be store within the reply, however if nothing else was stored, then the reply is considered empty.
	int cEventsRemaining;
	PSZUC pszEventValue;

	BOOL fEventsOutOfSyncInChatLog = FALSE;			// The events in the Chat Log are out of sync and therefore display a special icon
	TIMESTAMP_DELTA dtsOtherSynchronization = 1;	// By default, assume the events are NOT synchronized, therefore assign to the variable a non-zero value.
	CHS chXCPe = d_chXCPe_zNA;			// Last known XCP element
	IEvent * pEvent = NULL;
	TGroup * pGroup = NULL;				// Pointer to the selected group
	TGroupMember * pMember = NULL;		// Pointer to the contact within the selected group
	while (pXmlNodeXcpEvent != NULL)
		{
		Report(pXmlNodeXcpEvent->m_pszuTagName != NULL);
		MessageLog_AppendTextFormatCo((pXmlNodeXcpEvent->m_pszuTagName[0] == d_chXCP_ && pXmlNodeXcpEvent->m_pszuTagName[1] == d_chXCPe_EventError) ? COX_MakeBold(d_coRed) : d_coBlack, "\t Processing $s ^N", fEventsOutOfSyncInChatLog ? "(out of sync)" : NULL, pXmlNodeXcpEvent);
		// Get both timestamps, as most xcp-stanzas have both
		TIMESTAMP tsEventID = pXmlNodeXcpEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsEventID);
		TIMESTAMP tsOther = pXmlNodeXcpEvent->TsGetAttributeValueTimestamp_ML(d_chXCPa_tsOther);
		PSZUC pszEventName = pXmlNodeXcpEvent->m_pszuTagName;
		CHS chEventName0 = pszEventName[0];
		if (chEventName0 == d_chXCP_)
			{
			// We have a XCP control node which contains directives how to interpret the remaining xcp-stanzas/event(s). Some xcp-stanzas (control nodes) are at the beginning of the xmpp-stanza, while others are at the end.
			chXCPe = pszEventName[1];
			switch (chXCPe)
				{
			case d_chXCPe_GroupSelector:
				// The events belong to a group, so attempt to find the group
				Report(pGroup == NULL);
				Report(pMember == NULL);
				pszEventValue = pXmlNodeXcpEvent->m_pszuTagValue;
				binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_GroupSelector ">^s</" d_szXCPe_GroupSelector ">", pszEventValue);
				cbXcpStanzaReplyEmpty = binXcpStanzaReply.CbGetData();
				pGroup = m_pAccount->Group_PFindByIdentifier_YZ(IN pszEventValue, INOUT &binXcpStanzaReply, TAccountXmpp::eFindGroupCreate);	// Find the group matching the identifier, and if not there create it
				if (pGroup == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Group identifier '$s' is not valid\n", pszEventValue);
					break;
					}
				pChatLogEvents = pGroup;
				pMember = pGroup->Member_PFindOrAddContact_NZ(this);
				Assert(pMember != NULL);
				Assert(pMember->m_pContact == this);
				ptsOtherLastSynchronized = &pMember->m_tsOtherLastSynchronized;
				pVault = pChatLogEvents->Vault_PGet_NZ();
				pwChatLog = pChatLogEvents->ChatLog_PwGet_YZ();
				MessageLog_AppendTextFormatCo(d_coGrayDark, "\t\t Selecting group '$s' (m_tsOtherLastSynchronized=$t, m_tsEventIdLastSentCached=$t)\n", pGroup->TreeItem_PszGetNameDisplay(), *ptsOtherLastSynchronized, pGroup->m_tsEventIdLastSentCached);
				break;
			case d_chXCPe_EventPrevious:	// We are receiving information regarding the previous message
				dtsOtherSynchronization = tsOther - *ptsOtherLastSynchronized;
				if (dtsOtherSynchronization > 0)
					{
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t m_tsOtherLastSynchronized = $t however its value should be $t\n"
						"\t\t\t therefore requesting $s to resend its next event after $t\n", *ptsOtherLastSynchronized, tsOther, pszNameDisplay, *ptsOtherLastSynchronized);
					binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventNextRequest_tsI "/>", *ptsOtherLastSynchronized);
					}
				else if (dtsOtherSynchronization < 0)
					{
					Assert(tsOther < *ptsOtherLastSynchronized);
					// The most common scenario for this code path is when the Chat Log has been deleted.  The timestamps from config.xml remain, however those timestamps do no longer represent the actual events.
					// Before notifying the contact it is missing its own messages, atempt to verify the integrity of our own timestamps.
					pEvent = pVault->PFindEventReceivedByTimestampOther(*ptsOtherLastSynchronized, this);
					if (pEvent != NULL)
						{
						MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t tsOther $t < m_tsOtherLastSynchronized $t\n"
							"\t\t\t therefore $s is missing ITS OWN messages!  As a result, send the necessary info to notify $s about the situation.\n", tsOther, *ptsOtherLastSynchronized, pszNameDisplay, pszNameDisplay);
						binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventsMissing "/>");
						}
					else
						{
						MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t tsOther $t < m_tsOtherLastSynchronized $t\n"
							"\t\t\t however since there is no event matching $t, we adjust m_tsOtherLastSynchronized of $s from $t to $t\n", tsOther, *ptsOtherLastSynchronized, *ptsOtherLastSynchronized, pszNameDisplay, *ptsOtherLastSynchronized, tsOther);
						*ptsOtherLastSynchronized = tsOther;
						dtsOtherSynchronization = 0;	// We are now synchronized!
						}
					}
				else
					{
					Assert(dtsOtherSynchronization == 0);	// The timestamps do synchronize
					// For group chat, check if we are not missing messages from other group members
					int cEventsReceivedByOthers = pXmlNodeXcpEvent->UFindAttributeValueDecimal_ZZR(d_chXCPa_EventPrevious_cEventsMissing);
					if (cEventsReceivedByOthers > 0)
						{
						Assert(tsOther > d_tsOther_kmReserved);
						int cEventsReceivedInChatLog = pVault->UCountEventsReceivedByOtherGroupMembersSinceTimestampOther(tsOther);
						if (cEventsReceivedInChatLog < cEventsReceivedByOthers)
							{
							MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Apparently I am missing $I events written by other group members\n"
								"\t\t\t therefore asking $s to forward other's messages since tsOther $t\n", cEventsReceivedByOthers - cEventsReceivedInChatLog, pszNameDisplay, tsOther);
							binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventForwardRequest_tsI "/>", tsOther);
							break;
							}
						}

					if (tsEventID < pChatLogEvents->m_tsEventIdLastSentCached)
						{
						MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t tsEventID $t < m_tsEventIdLastSentCached $t\n"
							"\t\t\t therefore $s is missing my messages!\n", tsEventID, pChatLogEvents->m_tsEventIdLastSentCached, pszNameDisplay);
						binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventsMissing "/>");
						}
					else if (tsEventID > pChatLogEvents->m_tsEventIdLastSentCached)
						{
						// I have no idea what this situation means, so I am reporting it on the Message Log
						MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t tsEventID $t > m_tsEventIdLastSentCached $t\n\t\t Apparently $s has messages from ME that I don't have.\n", tsEventID, pChatLogEvents->m_tsEventIdLastSentCached, pszNameDisplay);
						}
					} // if...else
				break;
			case d_chXCPe_EventsMissing:
				Assert(tsOther == d_ts_zNA);
				MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Apparently I am missing events (I am out of sync)\n\t\t therefore requesting $s to resend the next event(s) after $t\n", pszNameDisplay, *ptsOtherLastSynchronized);
				binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventNextRequest_tsI "/>", *ptsOtherLastSynchronized);
				break;
			case d_chXCPe_EventNextRequest:
				// We are receiving a request to resend the next event following tsEventID
				Endorse(tsEventID == d_ts_zNULL);	// Get the first event
				Assert(tsOther == d_ts_zNA);
				pEvent = pVault->PFindEventNext(tsEventID, OUT &cEventsRemaining);
				Assert(pEvent == NULL || pEvent->EGetEventClass() != eEventClass_eNull);
				MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Resending next event following tsEventID $t (cEventsRemaining = $i)\n", tsEventID, cEventsRemaining);
				if (pEvent != NULL)
					{
					binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventNextReply_tsO "/>", tsEventID);
					binXcpStanzaReply.BinXmlSerializeEventForXcp_ObsoleteProtocolVersion1(pEvent);
					binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventsOutstandingReply_i_tsO "/>", cEventsRemaining, pEvent->m_tsEventID);
					}
				break;
			case d_chXCPe_EventNextReply:	// Although d_chXCPe_EventNextReply may appear as useless, the variable chXCPe is set to d_chXCPe_EventNextReply which will be useful when unserializing events.
				Report(tsEventID == d_ts_zNA);
				dtsOtherSynchronization = tsOther - *ptsOtherLastSynchronized;
				if (dtsOtherSynchronization == 0)
					{
					MessageLog_AppendTextFormatCo(d_coGrayDark, "\t\t chXCPe = d_chXCPe_EventNextReply;\n");
					break;
					}
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t chXCPe = d_chXCPe_EventNextReply; (out of sync by $T)\n\t\t therefore ignoring remaining stanza(s): ^N\n", dtsOtherSynchronization, pXmlNodeXcpEvent->m_pNextSibling);
				goto Exit;	// If we are out of sync, it is pointless to continue the synchronization
			case d_chXCPe_EventsOutstandingReply:
				Report(tsEventID == d_ts_zNA);
				Report(tsOther != d_ts_zNULL);	// Typically tsOther is the same as pEvent->m_tsOther (the last event we processed), however not always the case as events may have been received out-of-sync
				if (pEvent == NULL)
					{
					// This is not necessary, however it is good documentation to indicate there was a previous event processed, although the event may unknown, and therefore pEvent may be NULL without being an error.
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t d_chXCPe_EventsOutstandingReply with pEvent == NULL\n");
					}
				cEventsRemaining = pXmlNodeXcpEvent->UFindAttributeValueDecimal_ZZR(d_chXCPa_EventsOutstandingReply_cEvents);
				if (cEventsRemaining > 0)
					{
					StatusBar_SetTextFormat("Downloading messages from $s... $I remaining...", pszNameDisplay, cEventsRemaining);
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Requesting $s to resend the next $I events after $t\n", pszNameDisplay, cEventsRemaining, tsOther);
					binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventNextRequest_tsI "/>", tsOther);
					break;
					}
				// There are no more outstanding events to download, so do cleanup and send the last known timestamps to give an opportunity to the remote client to download its missing messages (if any)
				StatusBar_ClearText();
				if (pMember != NULL)
					binXcpStanzaReply.BinXmlAppendTimestampsToSynchronizeWithGroupMember(pMember);
				else
					binXcpStanzaReply.BinXmlAppendTimestampsToSynchronizeWithContact(this);
				break;
			case d_chXCPe_EventForwardRequest:
				// We are receiving a request to forward an event written by another contact
				pEvent = pVault->PFindEventNextReceivedByOtherGroupMembers(tsEventID, this, OUT &cEventsRemaining);
				MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Forwarding the next event following tsEventID $t (cEventsRemaining = $i)\n", tsEventID, cEventsRemaining);
				if (pEvent != NULL)
					{
					binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventForwardReply_tsO "/>", tsEventID);
					binXcpStanzaReply.BinXmlSerializeEventForXcp_ObsoleteProtocolVersion1(pEvent);
					binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventsForwardRemaining_i_tsO "/>", cEventsRemaining, pEvent->m_tsEventID);
					}
				break;
			case d_chXCPe_EventForwardReply:
				// There is nothing to do  here, except set chXCPe = d_chXCPe_EventForwardReply
				MessageLog_AppendTextFormatCo(d_coGrayDark, "\t\t chXCPe = d_chXCPe_EventForwardReply;\n");
				break;
			case d_chXCPe_EventsForwardRemaining:
				cEventsRemaining = pXmlNodeXcpEvent->UFindAttributeValueDecimal_ZZR(d_chXCPa_EventsForwardRemaining_cEvents);
				if (cEventsRemaining > 0)
					{
					StatusBar_SetTextFormat("Downloading messages from $s... $I remaining...", pszNameDisplay, cEventsRemaining);
					MessageLog_AppendTextFormatSev(eSeverityWarning, "\t\t Requesting $s to resend the next $I events after $t\n", pszNameDisplay, cEventsRemaining, tsOther);
					binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventForwardRequest_tsI "/>", tsOther);
					break;
					}
				// There are no more outstanding events to download, so do cleanup and send the last known timestamps to give an opportunity to the remote client to download its missing messages (if any)
				StatusBar_ClearText();
				break;
			case d_chXCPe_EventConfirmation:
			case d_chXCPe_EventError:
				Report(tsEventID != d_ts_zNULL);
				Report(tsOther == d_ts_zNA);
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					if (chXCPe == d_chXCPe_EventError)
						pEvent->Event_SetFlagErrorProtocol();
					pEvent->Event_SetCompletedAndUpdateChatLog(pwChatLog);	// This will typically display a green checkmark at the right of the screen where the event is located
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unrecognized confirmation for EventID $t\n", tsEventID);
				break;
			case d_chXCPe_EventExtraDataRequest:
				Report(tsEventID > d_tsOther_kmReserved);
				Report(tsOther == d_ts_zNA);
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					pEvent->XcpExtraDataRequest(IN pXmlNodeXcpEvent, INOUT &binXcpStanzaReply);
					pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);	// This will typically show a progress bar
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unable to find EventID $t for d_chXCPe_EventExtraDataRequest (group $s)\n", tsEventID, (pGroup == NULL) ? NULL : pGroup->TreeItem_PszGetNameDisplay());
				break;
			case d_chXCPe_EventExtraDataReply:
				// A 'data reply' may occur if there is no 'request', therefore we need to check for tsEventID as well as tsOther
				if (tsEventID != d_ts_zNULL)
					{
					Assert(tsEventID > d_tsOther_kmReserved);
					Assert(tsOther == d_ts_zNA);
					pEvent = pVault->PFindEventByID(tsEventID);
					}
				else
					{
					Assert(tsOther > d_tsOther_kmReserved);
					pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pMember);
					}
				if (pEvent != NULL)
					{
					pEvent->XcpExtraDataArrived(IN pXmlNodeXcpEvent, INOUT &binXcpStanzaReply);
					pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);	// This will typically show a progress bar
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unable to find i=$t, o=$t for d_chXCPe_EventExtraDataReply\n", tsEventID, tsOther);
				break;
			case d_chXCPe_EventSplitDataRequest:
				// Send a chunk of a large data
				Assert(tsEventID > d_tsOther_kmReserved);
				Assert(tsOther == d_ts_zNA);
				{
				int ibDataSource = pXmlNodeXcpEvent->LFindAttributeXcpOffset();
				CDataXmlLargeEvent * pDataXmlLargeEvent = pVault->PFindOrAllocateDataXmlLargeEvent_NZ(tsEventID, IN_MOD_TMP &binXcpStanzaReply);
				int cbDataRemaining;
				#if 1
					int cbStanzaMaxBinary = 1 + pDataXmlLargeEvent->m_binXmlData.CbGetData() / 4;	// At the moment, send only 1 byte + 25% at the time (rather than c_cbStanzaMaxBinary), so we can test the code transmitting large events
					if (cbStanzaMaxBinary > CBinXcpStanza::c_cbStanzaMaxBinary)
						cbStanzaMaxBinary = CBinXcpStanza::c_cbStanzaMaxBinary;
				#else
					#define cbStanzaMaxBinary	CBinXcpStanza::c_cbStanzaMaxBinary
				#endif
				int cbData = pDataXmlLargeEvent->m_binXmlData.CbGetDataAfterOffset(ibDataSource, cbStanzaMaxBinary, OUT &cbDataRemaining);
				MessageLog_AppendTextFormatCo(d_coOrange, "Sending $i/$i bytes data from offset $i for tsEventID $t, cbDataRemaining = $i\n", cbData, cbStanzaMaxBinary, ibDataSource, tsEventID, cbDataRemaining);
				binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventSplitDataReply_tsO_i_pvcb , tsEventID, ibDataSource, pDataXmlLargeEvent->m_binXmlData.PvGetDataAtOffset(ibDataSource), cbData);
				if (cbDataRemaining <= 0)
					{
					// We reached the end of the XML data, therefore send the hash
					//pDataXmlLargeEvent->m_hashXmlData.rgbData[0] = 0;	// Corrupt the data to see how the protocol handles it
					binXcpStanzaReply.BinAppendText_VE(d_szXCPa_EventExtraData_shaData_h, IN &pDataXmlLargeEvent->m_hashXmlData);
					}
				binXcpStanzaReply.BinAppendXmlForSelfClosingElement();
				#if 0
				List_DetachNode(INOUT pDataXmlLargeEvent);
				delete pDataXmlLargeEvent;	// Destroy the cache, so we can test how stable is the code
				#endif
				}
				break;
			case d_chXCPe_EventSplitDataReply:
				// New data has arrived
				Assert(tsOther > d_tsOther_kmReserved);
				Assert(tsEventID == d_ts_zNA);
				pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pMember);
				if (pEvent != NULL && pEvent->EGetEventClass() == CEventDownloader::c_eEventClass)
					((CEventDownloader *)pEvent)->XcpDownloadedDataArrived(IN pXmlNodeXcpEvent, INOUT &binXcpStanzaReply, pwChatLog);
				else
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unable to find CEventDownloader matching tsOther $t for d_chXCPe_EventSplitDataReply\n", tsOther);
				break;
			case d_chXCPe_ApiRequest:
			case d_chXCPe_ApiReply:
				// This code is for backward compatibility.
				Assert(binXcpStanzaReply.m_pContact == this);
				if (binXcpStanzaReply.m_pContact == this)
					{
					binXcpStanzaReply.XcpApi_ExecuteApiList(IN pXmlNodeXcpEvent);
					binXcpStanzaReply.XcpSendStanza();
					}
				break;
			default:
				MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t Unknown XCP directive $s: chXCPe = $i ($b)\n", pszEventName, chXCPe, chXCPe);
				} // switch (chXCPe)
			}
		else
			{
			// The XML node contains an event of type EEventClass
			const BOOL fEventClassReceived = FEventClassReceived(chEventName0);
			EEventClass eEventClass = EEventClassFromPsz(pszEventName);
			switch (eEventClass)
				{
			case eEventClass_eMessageTextComposing:	// The contact is typing something
				pChatLogEvents->ChatLog_ChatStateIconUpdate((pXmlNodeXcpEvent->PFindAttribute(d_chXCPa_MessageTextComposing_State) == NULL) ? eChatState_zComposing : eChatState_fPaused, this);
				goto EventNext;
			case eEventClass_ePing:
				// The XCP ping returns the timestamp (in UTC) of the contact.  This way, it is possible to calculate the clock difference between the two devices.  Ideally the clock difference should be less than one minute.
				binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventExtraDataReply _tsI d_szXCPa_PingTime_t "/>", tsOther, Timestamp_GetCurrentDateTime());
				goto EventNext;
			case eEventClass_eVersion:
				binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventExtraDataReply _tsI d_szXCPa_eVersion_Version d_szXCPa_eVersion_Platform d_szXCPa_eVersion_Client "/>", tsOther);
				goto EventNext;
			#ifdef SUPPORT_XCP_VERSION_1
			case eEventClass_eDownloader:	// The downloader is a hybrid event, part of the Cambrian Protocol, and another part a regular event.
				/*
				if (pwChatLog == NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t No Chat Log present for CEventDownloader tsOther $t, therefore initiating download without GUI...\n", tsOther);
					binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventSplitDataRequest_tsI_i "/>", tsOther, 0);	// If there is no Chat Log present, then request the data to start a download.
					}
				*/
				break;
			#endif
			default:
				;		// Keep the compiler happy to prevent a warning the switch() statement does not handle all cases
				} // switch
			Assert(tsOther > d_tsOther_kmReserved);		// Any received event should have a valid tsOther
			BOOL fSwapTimestamps = FALSE;				// Swap tsEventID and tsOther.  This happens when a group event is forwarded by another contact
			TContact * pContactGroupSender = m_pAccount->Contact_PFindByIdentifierGroupSender_YZ(pXmlNodeXcpEvent);
			Endorse(pContactGroupSender == NULL);	// The message is for a 1-to-1 conversation
			if (pContactGroupSender == NULL)
				{
				if (fEventClassReceived && pGroup != NULL)
					pContactGroupSender = this;
				}
			else if (pContactGroupSender != this)
				{
				Assert(fEventClassReceived);
				Assert(tsEventID > d_tsOther_kmReserved);
				fSwapTimestamps = TRUE;
				pEvent = pVault->PFindEventReceivedByTimestampOther(tsEventID, pContactGroupSender);	// Search the event using swapped timestamps
				Assert(pGroup != NULL);
				if (pGroup != NULL)
					(void)pGroup->Member_PFindOrAddContact_NZ(pContactGroupSender);		// Make sure the contact is part of the group
				goto EventBegin;
				}
			pEvent = pVault->PFindEventReceivedByTimestampOther(tsOther, pContactGroupSender);	// Regardless of what type of event we received, always attempt to find it the current vault (which may be the vault of the contact or the vault of the group) and matching the contact of the group sender (if any)
			Endorse(pEvent == NULL);
			EventBegin:
			MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t pContactGroupSender = ^j, fSwapTimestamps = $s\n", pContactGroupSender, fSwapTimestamps ? "TRUE" : "FALSE");
			if (chXCPe != d_chXCPe_EventNextReply && chXCPe != d_chXCPe_EventForwardReply)
				{
				// We are receiving new events from the remote client
				Assert(tsEventID == d_ts_zNULL);						// New events should not have a tsEventID
				binXcpStanzaReply.m_eStanzaType = eStanzaType_eMessage;	// Any event confirmation (or error) shall be cached as an 'XMPP message' so the remote client may know about it
				if (pwChatLog != NULL)
					//pwChatLog->ChatLog_ChatStateComposerRemove(this);	// Make sure the text "<user> is typing..." will be no longer displayed
					pwChatLog->ChatLog_ChatStateIconUpdate(this, eChatState_fPaused);	// Make sure the text "<user> is typing..." will be no longer displayed
				if (pEvent != NULL)
					{
					// The new event is already in the Chag Log.  This is the case when the event is a re-send.
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Updating existing tsEventID $t matching tsOther $t written by ^j\n", pEvent->m_tsEventID, tsOther, pContactGroupSender);
					goto EventUnserialize;
					}
				}
			else
				{
				// We are synchronizing by receiving previous events
				Assert(tsEventID == d_ts_zNULL || tsEventID > d_tsOther_kmReserved);	// The EventID may be zero for the case of receiving an event which was never delivered before
				if (pEvent != NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t [Sync] Updating existing tsEventID $t matching tsOther $t written by ^j\n", pEvent->m_tsEventID, tsOther, pContactGroupSender);
					goto EventUnserialize;
					}
				// If we are unable to find the event by tsOther, attempt to find it by its identifier
				pEvent = pVault->PFindEventByID(tsEventID);
				if (pEvent != NULL)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t [Sync] Updating existing tsEventID $t written by ^j\n", tsEventID, pContactGroupSender);
					EventUnserialize:
					Assert(pEvent->m_tsEventID > d_tsOther_kmReserved);
					if (fEventClassReceived)
						{
						// Only unserialize existing 'received' events.  This is a security feature, so someone does not attempt to hijack our history log by pretending to resend our own events.
						Assert(pEvent->Event_FIsEventTypeReceived());
						pEvent->XmlUnserializeCore(IN pXmlNodeXcpEvent);	// TODO: Need to provide a mechanism if an event has been updated
						pEvent->ChatLog_UpdateEventWithinWidget(pwChatLog);
						}
					goto EventConfirmation;
					}
				} // if...else

			// We have a new event to allocate and to display into the Chat Log.
			Assert(pEvent == NULL);
			MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t Allocating event '$U' tsEventID $t written by ^j\n", eEventClass, tsEventID, pContactGroupSender);
			pEvent = IEvent::S_PaAllocateEvent_YZ(eEventClass, (tsEventID == d_ts_zNULL) ? NULL : &tsEventID);
			if (pEvent == NULL)
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "\t\t Unable to allocate event tsOther $t from stanza because its class ($i) is unrecognized: '$s'\n", tsOther, eEventClass, pszEventName);
				binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventError_tsI_s "/>", tsOther, pszEventName);	// Report the error to the remote client
				goto EventSynchronize;
				}
			pEvent->m_uFlagsEvent |= pXmlNodeXcpEvent->UFindAttributeValueHexadecimal_ZZR(d_chXCPa_IEvent_uFlagsEvent);
			Assert(pEvent->m_pVaultParent_NZ == NULL);
			pEvent->m_pVaultParent_NZ = pVault;	// Assign the parent vault right away
			#ifdef d_szEventDebug_strContactSource
			if (pGroup != NULL)
				pEvent->m_strDebugContactSource = m_strJidBare;
			#endif
			#ifdef d_szEventDebug_strVersion
			pEvent->m_strDebugVersion = (PSZUC)d_szApplicationVersion;
			#endif
			#ifdef DEBUG
			if (USZU_from_USZUF(pEvent->EGetEventClass()) != eEventClass)
				MessageLog_AppendTextFormatCo(d_coBlueDark, "\t\t\t Event tsOther $t was allocated as '$U' however its runtime class is '$U'\n", tsOther, eEventClass, pEvent->EGetEventClass());
			if (pEvent->Event_FIsEventTypeSent() && pContactGroupSender != NULL)
				MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t\t tsEventID $t, tsOther $t should NOT have a pContactGroupSender ^j\n", pEvent->m_tsEventID, tsOther, pContactGroupSender);
			#endif
			if (!fSwapTimestamps)
				pEvent->m_tsOther = tsOther;
			else
				{
				pEvent->m_tsOther = pEvent->m_tsEventID;
				pEvent->m_tsEventID = tsOther;
				}
			pEvent->XmlUnserializeCore(IN pXmlNodeXcpEvent);
			Assert(pEvent->m_pContactGroupSender_YZ == NULL);
			if (fEventClassReceived)
				{
				if (!pEvent->Event_FIsEventTypeReceived())
					{
					MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "\t\t\t Event tsEventID $t, tsOther $t of class '$U' should be RECEIVED\n", pEvent->m_tsEventID, pEvent->m_tsOther, eEventClass);
					}
				if (pContactGroupSender != NULL)
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t\t Assigning m_pContactGroupSender_YZ ^j to tsEventID $t, tsOther $t\n", pContactGroupSender, pEvent->m_tsEventID, pEvent->m_tsOther);
				pEvent->m_pContactGroupSender_YZ = pContactGroupSender;
				}
			fEventsOutOfSyncInChatLog |= pVault->m_arraypaEvents.Event_FoosAddSorted(PA_CHILD pEvent);
			if (fEventsOutOfSyncInChatLog)
				pEvent->Event_SetFlagOutOfSync();
			if (pwChatLog != NULL)
				pwChatLog->ChatLog_EventDisplay(IN pEvent);

			// Update the GUI about the new event
			pChatLogEvents->TreeItemChatLog_IconUpdateOnNewMessageArrivedFromContact(IN pEvent->PszGetTextOfEventForSystemTray(OUT_IGNORED &g_strScratchBufferStatusBar), this, pMember);
			TreeItem_IconUpdate();		// Update the icon of the contact, which will in turn update the icon(s) of all its aliases, including the member contact of the group.  It is important to update the icon of the contact because it is likely to be displaying the pencil icon indicating the user was composing/typing text.

			if (chXCPe == d_chXCPe_EventForwardReply)
				goto EventNext;		// Don't attempt to synchronize a forwarded event

			EventConfirmation:
			binXcpStanzaReply.BinAppendText_VE("<" d_szXCPe_EventConfirmation_tsI "/>", tsOther);	// Acknowledge to the remote client we received (and processed) the event
			EventSynchronize:
			if (dtsOtherSynchronization == 0)
				{
				//Assert(tsOther > *ptsOtherLastSynchronized);
				if (tsOther > *ptsOtherLastSynchronized)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t\t Updating m_tsOtherLastSynchronized from $t to $t\n", *ptsOtherLastSynchronized, tsOther);
					*ptsOtherLastSynchronized = tsOther;
					}
				else if (tsOther < *ptsOtherLastSynchronized)
					{
					// I am not sure what this situation means (perhaps editing an existing event)
					MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "tsOther $t  <  m_tsOtherLastSynchronized $t\n", tsOther, *ptsOtherLastSynchronized);
					}
				}
			else
				{
				MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Out of sync by $T\n", dtsOtherSynchronization);
				}
			Endorse(pEvent == NULL);	// This happens when an event cannot becreated, typically because its class is unknown
			//Assert(pEvent != NULL);
			if (pEvent != NULL && pEvent->Event_FIsEventTypeSent())
				{
				// Adjust the timestamp if the event we processed 'was' our own event.  This happens when synchronizing where the remote client re-send all the events, including our own
				if (tsEventID > pChatLogEvents->m_tsEventIdLastSentCached)
					{
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t\t Updating m_tsEventIdLastSentCached from $t to $t\n", pChatLogEvents->m_tsEventIdLastSentCached, tsEventID);
					pChatLogEvents->m_tsEventIdLastSentCached = tsEventID;
					}
				else
					MessageLog_AppendTextFormatSev(eSeverityComment, "\t\t\t tsEventID $t  <=  m_tsEventIdLastSentCached $t\n", tsEventID, pChatLogEvents->m_tsEventIdLastSentCached);
				}
			} // if...else

		EventNext:
		pXmlNodeXcpEvent = pXmlNodeXcpEvent->m_pNextSibling;	// Get the next event/xcp-stanza
		} // while
	Exit:
	if (binXcpStanzaReply.CbGetData() > cbXcpStanzaReplyEmpty)
		binXcpStanzaReply.XcpSendStanzaToContact(IN this);
	Assert(pVault->m_arraypaEvents.FEventsSortedByIDs());
	} // Xcp_ProcessStanzasAndUnserializeEvents()
#endif

/*
//	Method to serialize an event to be transmitted through the Cambrian Protocol.
void
CBinXcpStanza::BinXmlSerializeEventForXcp_ObsoleteProtocolVersion1(const IEvent * pEvent)
	{
	Assert(pEvent != NULL);
	Assert(pEvent->m_pVaultParent_NZ != NULL);
	Assert(m_pContact != NULL);
	Assert(m_pContact->EGetRuntimeClass() == RTI(TContact));
	Assert(m_paData	!= NULL);
	EEventClass eEventClassXcp = pEvent->EGetEventClassForXCP();
	if ((eEventClassXcp & eEventClass_kfNeverSerializeToXCP) == 0)
		BinXmlSerializeEventForXcpCore(pEvent, pEvent->m_tsOther);
	}

//	Send the event through XMPP.  If the contact can understand the Cambrian Protocol, then the event will be serialized for XCP, ortherwise will be sent throught the standard XMPP.
void
IEvent::Event_WriteToSocket()
	{
	Assert(m_pVaultParent_NZ != NULL);
	Assert(m_tsOther == d_tsOther_ezEventNeverSent);
	if (m_tsOther != d_tsOther_ezEventNeverSent)
		return;
	m_tsOther = d_tsOther_eEventSentOnce;
	CBinXcpStanzaTypeMessage binXcpStanza;			// By default, events are sent as XMPP 'messages'
	ITreeItemChatLogEvents * pContactOrGroup = m_pVaultParent_NZ->m_pParent;
	Assert(pContactOrGroup != NULL);
	TGroup * pGroup = (TGroup *)pContactOrGroup;
	TContact * pContact = (TContact *)pContactOrGroup;
	if (pContact->EGetRuntimeClass() == RTI(TContact))
		{
		// Send the message to a contact
		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGrayDark), "\t Event_WriteToSocket() - Sending message to $S\n\t\t m_tsEventIdLastSentCached $t, m_tsOtherLastSynchronized $t\n", &pContact->m_strJidBare, pContact->m_tsEventIdLastSentCached, pContact->m_tsOtherLastSynchronized);
		binXcpStanza.m_pContact = pContact;
		binXcpStanza.BinXmlAppendTimestampsToSynchronizeWithContact(pContact);
		binXcpStanza.BinXmlSerializeEventForXcp_ObsoleteProtocolVersion1(IN this);
		binXcpStanza.XcpSendStanza();
		}
	else
		{
		// Broadcast the message to every [active] group member
		Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
		TGroupMember ** ppMemberStop;
		TGroupMember ** ppMember = pGroup->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
		while (ppMember != ppMemberStop)
			{
			TGroupMember * pMember = *ppMember++;
			Assert(pMember != NULL);
			Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
			binXcpStanza.m_pContact = pMember->m_pContact;
			if (binXcpStanza.m_pContact->m_cVersionXCP <= 0)
				{
				MessageLog_AppendTextFormatSev(eSeverityNoise, "Event_WriteToSocket() - Skipping group peer $S because its client does not support XCP (probably because it is offline)\n", &pMember->m_pContact->m_strJidBare);
				continue;
				}
			MessageLog_AppendTextFormatSev(eSeverityNoise, "Event_WriteToSocket() - Sending message to group member $S\n", &pMember->m_pContact->m_strJidBare);
			binXcpStanza.BinXmlInitStanzaWithGroupSelector(pGroup);	// This line could be removed out of the loop
			binXcpStanza.BinXmlAppendTimestampsToSynchronizeWithGroupMember(pMember);
			binXcpStanza.BinXmlSerializeEventForXcp_ObsoleteProtocolVersion1(IN this);
			binXcpStanza.XcpSendStanza();	// Send the XCP stanza to the contact
			} // while
		} // if...else
	if ((EGetEventClass() & eEventClass_kfNeverSerializeToDisk) == 0)
		{
		MessageLog_AppendTextFormatCo(COX_MakeBold(d_coGrayDark), "\t\t Updating m_tsEventIdLastSentCached from $t to $t\n", pContactOrGroup->m_tsEventIdLastSentCached, m_tsEventID);
		pContactOrGroup->m_tsEventIdLastSentCached = m_tsEventID;
		}
	m_pVaultParent_NZ->SetModified();	// This line is important so m_tsOther (as well as other timestamps) are saved to disk
	} // Event_WriteToSocket()

//	Send the event if the socket is ready
void
IEvent::Event_WriteToSocketIfReady()
	{
	if (PGetSocketOnlyIfReady() != NULL)
		Event_WriteToSocket();
	}
*/
#if 0
//	The enumeration ETaskClass is used to serialize and unserialize tasks and determine what type of task it is.
enum ETaskClass
	{
	eTaskClass_CTaskSend		= _USZU1('s'),
	eTaskClass_CTaskReceive		= _USZU1('r')
	};


class ITask : public SListNode	// (task)
{
public:
	TIMESTAMP m_tsTaskID;			// Identifier of the task
	/*
	TContact * m_pContact;			// Which contact the task is for
	CBin m_binData;					// Data to send to the contact
	int m_cbTransmitted;
	*/

public:
	ITask(const TIMESTAMP * ptsTaskID = d_ts_pNULL_AssignToNow);
	virtual ~ITask();
	virtual ETaskClass EGetTaskClass() const  = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Linked list of ITask.
//	Since ITask inherits SListNode, there is no need to remove (or delete) nodes from CListTasks as the destructor of ITask will take care of removing nodes from the CListTasks.
//	Also, each CListTasks is mutually exclusive, as each ITask may belong only to one list.
class CListTasks : public CList
{
public:
	void DeleteTask(PA_DELETING ITask * paTask);
	void DeleteAllTasks();
	ITask * PFindTaskByID(TIMESTAMP tsTaskID, ETaskClass eTaskClass) const;

	void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);
};

class CTaskSend : public ITask
{
public:
	CBin m_binData;					// Data to transmit
public:
	CTaskSend(const TIMESTAMP * ptsTaskID = d_ts_pNULL_AssignToNow);
	virtual ETaskClass EGetTaskClass() const { return eTaskClass_CTaskSend; }
};

//	Receiving data is similar to sending data, except we need to know what is the total data available
class CTaskReceive : public CTaskSend
{
public:
	int m_cbTotal;	// Total expected amount of data to receive

public:
	CTaskReceive(const TIMESTAMP * ptsTaskID);
	virtual ETaskClass EGetTaskClass() const { return eTaskClass_CTaskReceive; }
};
#endif
/*
//	All task to be sent are appended at the end of the list, so the order is preserved
void
CListTasksSendReceive::AddTaskSend(INOUT CTaskSendReceive * pTaskSent)
	{
	Assert(pTaskSent != NULL);
	Assert(pTaskSent->m_pNext == NULL);
	pTaskSent->m_pNext = m_plistTasks;	// Need to fix this (to append at the end)
	m_plistTasks = pTaskSent;
	}

//	A new task to be downloaded is inserted at the beginning of the list so it may be accessed quickly
void
CListTasksSendReceive::AddTaskReceive(INOUT CTaskSendReceive * pTaskReceive)
	{
	Assert(pTaskReceive != NULL);
	Assert(pTaskReceive->m_pNext == NULL);
	pTaskReceive->m_pNext = m_plistTasks;
	m_plistTasks = pTaskReceive;
	}
*/

/*
void
CListTasksSendReceive::AppendNextTaskToStanza(INOUT CBinXcpStanza * pbinXcpStanza)
	{
	Assert(pbinXcpStanza != NULL);
	if (m_plistTasks == NULL)
		return;	// Nothing to do
	const int cbTotal = m_plistTasks->m_cbTotal;
	if (cbTotal == CTaskSendReceive::c_cbTotal_SendRetry)
		{
		// Attempt to send as much data as possible
		m_plistTasks->m_cbTotal = CTaskSendReceive::c_cbTotal_SentOnce;
		BinAppendText_VE("<" d_szAPIe_TaskDownloading_tsI d_szAPIa_TaskDataOffset_i d_szAPIa_TaskDataBinary_Bii "/>", m_plistTasks->m_tsTaskID, ibData, &m_plistTasks->m_binData, ibData, cbStanzaMaxBinary);
		return;
		}
	if (cbTotal == CTaskSendReceive::c_cbTotal_SendOnlyOnce)
		DeleteTask(m_plistTasks);
	}
void
CListTasksSendReceive::SentTasksToContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	// Find the first task to send
	CTaskSendReceive * pTask = m_plistTasks;
	while (pTask != NULL)
		{
		if (pTask->FIsTaskSend())
			{
			CBinXcpStanza binXcpStanza;
			binXcpStanza.XcpSendTaskDataToContact(pContact, pTask);
			MessageLog_AppendTextFormatSev(eSeverityWarningToErrorLog, "SentTasksToContact(^j) - Task ID $t of $I bytes:\n$B\n", pContact, pTask->m_tsTaskID, pTask->m_binData.CbGetData(), &pTask->m_binData);
			return;
			}
		pTask = pTask->m_pNext;
		}
	}
*/

void
CBinXcpStanza::XcpSendTaskDataToContact(TContact * pContact, const CTaskSendReceive * pTaskUpload, int ibData)
	{
	Assert(pContact != NULL);
	Assert(pTaskUpload != NULL);
	Assert(ibData >= 0);
	#ifdef DEBUG_XCP_TASKS
	m_cbStanzaThresholdBeforeSplittingIntoTasks = c_cbStanzaThresholdBeforeSplittingIntoTasks;	// Increase the threshold to its maximum value so the task data may be transmitted as fast as possible
	int cbStanzaMaxBinary = 1 + pTaskUpload->m_binXmlData.CbGetData() / 4;	// At the moment, send 1 byte + 25% at the time (rather than c_cbStanzaMaxBinary), so we can test the code transmitting tasks
	if (cbStanzaMaxBinary > c_cbStanzaMaxBinary)
		cbStanzaMaxBinary = c_cbStanzaMaxBinary;
	#else
		#define cbStanzaMaxBinary	c_cbStanzaMaxBinary
	#endif
	BinAppendText_VE("<" d_szXOSPe_TaskDownloading_ts d_szAPIa_TaskDataOffset_i d_szAPIa_TaskDataBinary_Bii "/>", pTaskUpload->m_tsTaskID, ibData, &pTaskUpload->m_binXmlData, ibData, cbStanzaMaxBinary);
	Assert(m_paData->cbData < c_cbStanzaThresholdBeforeSplittingIntoTasks);
	m_uFlags |= F_kfTaskIncluded;
	XospSendStanzaToContactAndEmpty(pContact);	// Send the data immediately, as the data from the task will fill the entire XMPP stanza
	}

	#ifdef SUPPORT_XCP_VERSION_1
	eEventClass_eDownloader							= _USZU2('d', 'l'),
	eEventClass_eDownloader_class					= _USZU2('d', 'l') | eEventClass_kfNeverSerializeToXCP | eEventClass_kfReceivedByRemoteClient,	// The downloader is saved to disk and its class never serialized for XCP.
		#define d_szXCPe_CEventDownloader_i_tsO		"dl s='$i'" _tsO		// At the moment, the downloader class will only send 32 bit of data, hence $i
		#define d_chXCPa_CEventDownloader_cblDataToDownload		's'	// Size of the data to download
		#define d_chXCPa_CEventDownloader_bin85DataReceived		'd'	// Data was received so far.  This value is used when saving the downloader to disk to resume the download later.
		#define d_chXCPa_CEventDownloader_tsForwarded			'f'	// Timestamt for a forwarded event
		#define d_szXCPa_CEventDownloader_tsForwarded_t			" f='$t'"
	#endif
#ifdef SUPPORT_XCP_VERSION_1
///////////////////////////////////////////////////////////////////////////////////////////////////
//	Class to download a large event.
//	This class is a hybrid between an event and the core of the XCP protocol.
//	The CEventDownloader is never allocated by the sender, however it is always unserialized and allocated by the receiver.
//	Until the large event has been fully downloaded, the downloader will have its own class, and later will emulate the behavior of the downloaded event.
class CEventDownloader : public IEvent
{
protected:
	TIMESTAMP m_tsForwarded;				// Forwarded timestamp (this happens when a group event is forwarded by another group member)
	enum { c_cbDataToDownload_Error = -1 }; // Use variable m_cbDataToDownload to store an arror code
	int m_cbDataToDownload;			// How much data needs to be downloaded (this variable is good for a progress bar).  Since the 'event downloader' is not to transfer files, but large text messages, a 32-bit integer is sufficient for storing such a message.
	CBin m_binDataDownloaded;		// Data downloaded (so far)
	IEvent * m_paEvent;				// Allocated event (when the download is complete)
public:
	CEventDownloader(const TIMESTAMP * ptsEventID);
	virtual ~CEventDownloader();
	virtual EEventClass EGetEventClass() const;
	virtual EEventClass EGetEventClassForXCP() const;
	virtual EXml XmlSerializeCoreE(IOUT CBinXcpStanza * pbinXmlAttributes) const;
	virtual void XmlUnserializeCore(const CXmlNode * pXmlNodeElement);
	virtual void XcpExtraDataRequest(const CXmlNode * pXmlNodeExtraData, INOUT CBinXcpStanza * pbinXcpStanzaReply);
	virtual void XcpExtraDataArrived(const CXmlNode * pXmlNodeExtraData, CBinXcpStanza * pbinXcpStanzaReply);
	virtual void ChatLogUpdateTextBlock(INOUT OCursor * poCursorTextBlock) CONST_MAY_CREATE_CACHE;
	virtual void HyperlinkGetTooltipText(PSZUC pszActionOfHyperlink, IOUT CStr * pstrTooltipText);
	virtual void HyperlinkClicked(PSZUC pszActionOfHyperlink, INOUT OCursor * poCursorTextBlock);

	void XcpDownloadedDataArrived(const CXmlNode * pXmlNodeData, INOUT CBinXcpStanza * pbinXcpStanzaReply, QTextEdit * pwEditChatLog);
	inline BOOL FIsDownloaderMatchingEvent(const IEvent * pEvent) const { return (pEvent == m_paEvent); }

	static const EEventClass c_eEventClass = eEventClass_eDownloader_class;
	friend class CBinXcpStanza;
}; // CEventDownloader

class CDataXmlLargeEvent;	// Object holding the data to send to CEventDownloader
class CListaDataXmlLargeEvents : public CList
{
public:
	~CListaDataXmlLargeEvents();
	void DeleteIdleNodes();
};
#endif
#ifdef SUPPORT_XCP_VERSION_1
//	Class holding a the data for an event requiring a large amount of XML data to transmit.
class CDataXmlLargeEvent : public SListNode
{
public:
	TIMESTAMP m_tsEventID;			// Which event the stanza is associated
	CBin m_binXmlData;				// XML data to transmit.  This data is the caching of IEvent::XmlSerializeCore() and will be unserialized with XmlUnserializeCore().
	TIMESTAMP_MINUTES m_tsmLastAccessed;	// Remember when this stanza was last used (after about 15 minutes of inactivity, the object gets deleted because it is assumed the stanza is now idle)
	SHashSha1 m_hashXmlData;				// Reliable checksum of the data
};
#endif


//	The XCP protocol uses lowercase and uppercase letters:
//	A lowercase letter designates an xcp-stanza (a stanza within an xmpp-stanza) where there could be a reply.
//	An uppercase letter designates a reply to an xcp-stanza.  There is never a reply to a reply, otherwise it may cause an infinite loop.
//	The tsOther and tsEventID 'make sense' for the receiver of the xcp-stanza; it is the responsibility of the sender to swap those values accordingly.

#define d_chXCPe_GroupSelector								'g'	// Which group to assign the events.  The body of the XML element is the identifier of the group.
#define d_szXCPe_GroupSelector						d_szXCP_"g"

/*
CBinXcpStanzaTypeInfo::CBinXcpStanzaTypeInfo(IEvent * pEvent) : CBinXcpStanza(eStanzaType_zInformation)
	{
	Assert(pEvent != NULL);
	Assert(pEvent->m_pVaultParent_NZ != NULL);
	TGroup * pContactOrGroup = (TGroup *)pEvent->m_pVaultParent_NZ->m_pParent;
	if (pContactOrGroup->EGetRuntimeClass() == RTI(TContact))
		m_pContact = (TContact *)pContactOrGroup;
	else
		{
		Assert(pContactOrGroup->EGetRuntimeClass() == RTI(TGroup));
		m_pContact = pEvent->m_pContactGroupSender_YZ;
		Report(m_pContact != NULL && "Invalid peer group sender");	// This pointer may actually be NULL if the event is the group sender
		// We have a group, therefore add the group selector to the XCP stanza
		BinXmlInitStanzaWithGroupSelector(pContactOrGroup);
		}
	}
void
CBinXcpStanza::BinXmlInitStanzaWithGroupSelector(TGroup * pGroup)
	{
	Assert(pGroup != NULL);
	Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
	BinInitFromTextSzv_VE("<" d_szXCPe_GroupSelector ">{h|}</" d_szXCPe_GroupSelector ">", &pGroup->m_hashGroupIdentifier);
	}
*/

/*
//	XCP is an abbreviation for "eXtensible Cambrian Protocol".
//	In a nutshell, this protocol is a layer taking care of end-to-end encryption between clients, as well as automatically
//	splitting large pieces of data into smaller xmpp-stanzas when the data does not fit in the recommended xmpp-stanza size of 4 KiB.
#define d_chXCP_					'_'
#define d_szXCP_					"_"

//	The following #define are for the XCP APIs which are shared among multiple .cpp files
#define d_chXCPe_ApiRequest									'a'
#define d_szXCPe_ApiRequest_s						d_szXCP_"a n='$s'"
#define d_szXCPe_ApiRequestGroup_s_h				d_szXCP_"a n='$s' g='{h|}'"
#define d_szXCPe_ApiRequest_close					d_szXCP_"a"
	#define d_chXCPa_Api_strName								'n'		// Name of the function (API) to call
	#define d_chXCPa_Api_shaGroupID								'g'		// Group (if any) where the API is related
#define d_chXCPe_ApiReply									'A'
#define d_szXCPe_ApiReply_s							d_szXCP_"A n='$s'"
#define d_szXCPe_ApiReply_close						d_szXCP_"A"
	#define d_chXCPa_Api_eErrorCode								'e'	// Store the error code EErrorXcpApi
	#define d_chXCPa_Api_strxErrorData							'd'
	#define d_szXCPa_Api_ErrorCodeAndErrorData_i_s				" e='$i' d='^s'"
*/

/*
//	Find the next event after tsEventID, and return how many events are remaining.
//	Since this method is for XCP, the method will skip returning non-XCP events and only return events which may be transmitted throught XCP.
//	For instance, a 'ping' event will not be returned by this method.
//
IEvent *
CArrayPtrEvents::PFindEventNextForXcp(TIMESTAMP tsEventID, OUT int * pcEventsRemaining) const
	{
	Endorse(tsEventID == d_ts_zNULL);	// Return the first event in the array
	Assert(pcEventsRemaining != NULL);
	if (m_paArrayHdr != NULL)
		{
		int cEvents = m_paArrayHdr->cElements;
		if (cEvents > 0)
			{
			IEvent ** ppEventFirst = (IEvent **)m_paArrayHdr->rgpvData;
			IEvent ** ppEventStop = ppEventFirst + cEvents;
			IEvent ** ppEventCompare = ppEventStop;
			// Search from the end until we find an event smaller or equal than tsEventID.  This means the next event (if any) in the array is the one to return;
			while (--ppEventCompare >= ppEventFirst)
				{
				IEvent * pEvent = *ppEventCompare;
				AssertValidEvent(pEvent);
				Assert(pEvent->m_tsEventID != d_ts_zNULL);
				if (pEvent->m_tsEventID <= tsEventID)
					break;
				} // while

			// Loop until we have a valid event for XCP
			while (++ppEventCompare < ppEventStop)
				{
				IEvent * pEvent = *ppEventCompare;	// Get the next event
				if ((pEvent->EGetEventClass() & eEventClass_kfNeverSerializeToXCP) == 0)
					{
					*pcEventsRemaining = (ppEventStop - ppEventCompare) - 1;
					Assert(*pcEventsRemaining >= 0);
					return pEvent;
					}
				} // while
			} // if
		} // if
	*pcEventsRemaining = 0;
	return NULL;
	} // PFindEventNextForXcp()

IEvent *
CVaultEvents::PFindEventNext(TIMESTAMP tsEventID, OUT int * pcEventsRemaining) CONST_MCC
	{
	return m_arraypaEvents.PFindEventNextForXcp(tsEventID, OUT pcEventsRemaining);
	}


//	Very similar as PFindEventNext()
IEvent *
CVaultEvents::PFindEventNextReceivedByOtherGroupMembers(TIMESTAMP tsEventID, TContact * pContactExclude, OUT int * pcEventsRemaining) CONST_MCC
	{
	Assert(pContactExclude != NULL);
	int cEventsRemaining = -1;
	IEvent * pEventNext = NULL;
	IEvent ** ppEventStop;
	IEvent ** ppEventFirst = m_arraypaEvents.PrgpGetEventsStop(OUT &ppEventStop);
	IEvent ** ppEventCompare = ppEventStop;
	// Search from the end until we find an event smaller or equal than tsEventID.  This means the next event (if any) in the array is the one to return;
	while (--ppEventCompare >= ppEventFirst)
		{
		IEvent * pEvent = *ppEventCompare;
		AssertValidEvent(pEvent);
		Assert(pEvent->m_tsEventID != d_ts_zNULL);
		if (pEvent->m_tsEventID <= tsEventID)
			break;
		if (pEvent->m_pContactGroupSender_YZ == pContactExclude)
			continue;
		if ((pEvent->EGetEventClass() & (eEventClass_kfReceivedByRemoteClient | eEventClass_kfNeverSerializeToXCP)) == eEventClass_kfReceivedByRemoteClient)
			{
			pEventNext = pEvent;
			cEventsRemaining++;
			}
		} // while
	*pcEventsRemaining = cEventsRemaining;
	return pEventNext;
	} // PFindEventNextReceivedByOtherGroupMembers()
*/


class CDashboardSectionItem_ITreeItem : public IDashboardSectionItem
{
public:
	ITreeItem * m_piTreeItem;
public:
	CDashboardSectionItem_ITreeItem(ITreeItem * piTreeItem) { m_piTreeItem = piTreeItem; }
	virtual void DrawItemText(CPainterCell * pPainter);
	virtual int DrawItemIcons(CPainterCell * pPainter);
};

class CDashboardSectionItem_TGroup : public IDashboardSectionItem
{
public:
	TGroup * m_pGroup;

public:
	CDashboardSectionItem_TGroup(TGroup * pGroup) { m_pGroup = pGroup; }
	virtual void DrawItemText(CPainterCell * pPainter);
	virtual int DrawItemIcons(CPainterCell * pPainter);
};

class CDashboardSectionItem_TContact : public IDashboardSectionItem
{
public:
	TContact * m_pContact;

public:
	CDashboardSectionItem_TContact(TContact * pContact) { m_pContact = pContact; }
	virtual void DrawItemText(CPainterCell * pPainter);
	virtual int DrawItemIcons(CPainterCell * pPainter);
};


class CDashboardSectionItem_IEventBallot : public IDashboardSectionItem
{
public:
	IEventBallot * m_pBallot;

public:
	CDashboardSectionItem_IEventBallot(IEventBallot * pBallot) { m_pBallot = pBallot; }
	virtual void DrawItemText(CPainterCell * pPainter);
	virtual int DrawItemIcons(CPainterCell * pPainter);
};


#if 0
class TApplicationBallotmaster : public IApplication
{
protected:
	CStr m_strUrlAddress;				// Address to start the application (of course, this solution is not 100% portable, because the HTML files for the application are stored into the "user folder", however remembering the last URL is better than nothing)
	WLayoutBrowser * m_pawLayoutBrowser;
	CVaultEvents * m_paVaultBallots;	// Ballots are 'events' because they may be sent to other users, and therefore require a vault for their storage.
private:
	TContact * m_paContactDummy;		// Temporary hack to have a dummy contact as the 'parent' of the vault.  This is necessary because the vault was designed to have a contact as its parent, and need to be refactored.

public:
	TApplicationBallotmaster(TProfile * pProfileParent);
	~TApplicationBallotmaster();
	virtual PSZAC PszGetClassNameApplication() { return c_szaApplicationClass_Ballotmaster; }	// From IApplication
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);			// From IXmlExchange
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem

	CVaultEvents * PGetVault_NZ();

	CEventBallotPoll * PAllocateBallot(const IEventBallot * pEventBallotTemplate = NULL);
	void EventBallotAddAsTemplate(IEventBallot * pEventBallot);
	/*
	void ApiBallotSave(IN PSZUC pszXmlBallot);
	void ApiBallotsList(OUT CBin * pbinXmlBallots);
	*/
	RTI_IMPLEMENTATION(TApplicationBallotmaster)
};
#endif
/*
//	This function must have the same interface as PFn_PaAllocateApplication()
IApplication *
PaAllocateApplicationBallotmaster(TProfile * pProfileParent)
	{
	return new TApplicationBallotmaster(pProfileParent);
	}
*/

/*
TApplicationBallotmaster *
TProfile::PGetApplicationBallotmaster_NZ()
	{
	TApplicationBallotmaster * pApplication = (TApplicationBallotmaster *)m_arraypaApplications.PFindRuntimeObject(RTI(TApplicationBallotmaster));
	if (pApplication == NULL)
		{
		pApplication = (TApplicationBallotmaster *)PaAllocateApplicationBallotmaster(this);
		m_arraypaApplications.Add(PA_CHILD pApplication);
		pApplication->TreeItemApplication_DisplayWithinNavigationTree();
		}
	return pApplication;
	}
*/
/*
TApplicationBallotmaster::TApplicationBallotmaster(TProfile * pProfileParent) : IApplication(pProfileParent, eMenuIconVote)
	{
	m_paVaultBallots = NULL;
	m_paContactDummy = NULL;
	m_pawLayoutBrowser = NULL;
	}

TApplicationBallotmaster::~TApplicationBallotmaster()
	{
	delete m_pawLayoutBrowser;
	delete m_paContactDummy;
	}

void
TApplicationBallotmaster::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	IApplication::XmlExchange(INOUT pXmlExchanger);
	pXmlExchanger->XmlExchangeStr("URL", INOUT &m_strUrlAddress);
	CVaultEvents * pVault = PGetVault_NZ();
	pVault->XmlExchange("Ballots", INOUT pXmlExchanger);
	}

const EMenuActionByte c_rgzeActionsMenuApplication[] =
	{
	eMenuAction_ApplicationHide,
	ezMenuActionNone
	};

//	TApplicationBallotmaster::ITreeItem::TreeItem_MenuAppendActions()
void
TApplicationBallotmaster::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuApplication);
	}

//	TApplicationBallotmaster::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TApplicationBallotmaster::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_ApplicationHide:
		TreeItemW_Hide();
		return ezMenuActionNone;
	default:
		return IApplication::TreeItem_EDoMenuAction(eMenuAction);
		}
	}

//	TApplicationBallotmaster::ITreeItem::TreeItem_GotFocus()
void
TApplicationBallotmaster::TreeItem_GotFocus()
	{
	if (m_strUrlAddress.FIsEmptyString())
		m_strUrlAddress = "file:///" + m_pProfileParent->m_pConfigurationParent->SGetPathOfFileName("Apps/Ballotmaster/default.htm");
	if (m_pawLayoutBrowser == NULL)
		m_pawLayoutBrowser = new WLayoutBrowser(m_pProfileParent, INOUT_LATER &m_strUrlAddress);
	MainWindow_SetCurrentLayout(IN m_pawLayoutBrowser);
	}
