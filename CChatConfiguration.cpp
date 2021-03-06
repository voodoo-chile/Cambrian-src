///////////////////////////////////////////////////////////////////////////////////////////////////
//	CChatConfiguration.cpp
//
//	Class responsible holding the data regarding the configuration of the application.
//	This class is also responsible of saving and loading the configuration file Config.xml.
//	This class is a complement of widget WChatNavigation who is responsible of displaying the configuration to the user and
//	handling user input to add, remove and rename accounts.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
//#include "WNavigationTree.h"
#include "TMarketplace.h"

CChatConfiguration g_oConfiguration;
CArrayPtrAccountsXmpp g_arraypAccounts;
UINT g_uPreferences;	// Preferences of the user which are serialized in the configuration

CStr g_strPathFolderDefault;		// Default path to save the files
QString g_sPathConfigXmlDefault;	// Path where the config.xml should be located

///////////////////////////////////////////////////////////////////////////////////////////////////
CChatConfiguration::CChatConfiguration()
	{
	InitToZeroes(OUT &m_hashMd5LastSave, sizeof(m_hashMd5LastSave));
	HashSha1_InitRandom(OUT &m_hashSalt);
	}

CChatConfiguration::~CChatConfiguration()
	{
	/*
	NoticeListRoaming_FlushAndCauseMemoryLeak();
	m_oTreeItemCertificates.FlushDataAndCauseMemoryLeak();	// This line is important, otherwise the destructor of class TCertificates will attempt to delete the widget twice (at this time, the widget has already been destroyed)
	*/
	}

//	Destroy the configuration and free all its object.  This method is used by the destructor of the WMainWindow, because waiting for g_oConfiguration's destructor is too late and cause the application to crash
void
CChatConfiguration::Destroy()
	{
	g_arraypAccounts.RemoveAllElements();
	m_arraypaProfiles.DeleteAllTreeItems();
	m_oTreeItemCertificates.FlushDataAndCauseMemoryLeak();
	}

//	May return NULL if there is no profile in the configuration, or there is no profile selected while there is multiple profiles available.
TProfile *
CChatConfiguration::PGetProfileSelectedUnique_YZ() const
	{
	return (m_pProfileSelected != NULL) ? m_pProfileSelected : (TProfile *)m_arraypaProfiles.PvGetElementUnique_YZ();
	}

//	Register an encryption certificate.
//	If the certificate is not already registered, allocate one.
TCertificate *
CChatConfiguration::PCertificateRegister(TCertificate * pCertificateParent, const CSslCertificate & oCertificateSsl)
	{
	Endorse(pCertificateParent == NULL);		// We have a root certificate
	const QByteArray arraybCertificate = oCertificateSsl.toDer();	// Get the certificate in binary data

	ICertificate * piCertificateParent = pCertificateParent;
	if (piCertificateParent == NULL)
		piCertificateParent = &m_oTreeItemCertificates;
	CArrayPtrCertificates * parraypaCertificates = &piCertificateParent->m_arraypaCertificates;
	TCertificate * pCertificate = NULL;
	TCertificate ** ppCertificateStop;
	TCertificate ** ppCertificate = parraypaCertificates->PrgpGetCertificatesStop(OUT &ppCertificateStop);
	while (ppCertificate != ppCertificateStop)
		{
		pCertificate = *ppCertificate++;
		Assert(pCertificate != NULL);
		Assert(pCertificate->m_pCertificateParentIssuer == pCertificateParent);
		if (pCertificate->m_binDataBinary.FCompareBinary(arraybCertificate))
			{
			MessageLog_AppendTextFormatSev(eSeverityNoise, "Certificate '$S' already registered!\n", &pCertificate->m_strCertificateName);
			goto Done;
			}
		}
	// Register a new certificate
	MessageLog_AppendTextFormatSev(eSeverityNoise, "Registering new certificate...\n");
	pCertificate = new TCertificate(pCertificateParent);
	parraypaCertificates->Add(PA_CHILD pCertificate);
	pCertificate->m_binDataBinary = arraybCertificate;
	oCertificateSsl.GetCertificateName(OUT &pCertificate->m_strCertificateName);
	if (m_oTreeItemCertificates.m_paTreeItemW_YZ != NULL)
		{
		pCertificate->NavigationTree_DisplayCertificates(piCertificateParent);	// If the Tree Item "Certificates" is visible within the Navigation Tree, then display also the newly added certificate.
		pCertificate->TreeItemW_EnsureVisible();
		}
	Done:
	return pCertificate;
	} // PCertificateRegister()

//	Register all the certificates involved with the socket encryption.
//	Return the pointer of the certificate used to encrypt the socket connection
TCertificate *
CChatConfiguration::PCertificatesRegister(QSslSocket * pSocket)
	{
	TCertificate * pCertificateParent = NULL;	// A NULL pointer means a root certificate
	QList<QSslCertificate> listCertificatesSsl = pSocket->peerCertificateChain();	// Get all the SSL certificates involved with the socket
	int cCertificates = listCertificatesSsl.length();	// Reverse the list because we want to register the root certificates first.
//	Assert(cCertificates > 0);	// There should be at least one certificate in the chain
	while (--cCertificates >= 0)
		{
		const QSslCertificate & oCertificateSsl = listCertificatesSsl.at(cCertificates);	// The method at() is O(n). This is not efficient because the loop is O(n^2), however since the certificate chain is typically 2 or 3 items, perhaps 4, it is not a big deal.  Besides, it is probably faster than using the iterator which creates an object with a memory allocation.
		#if 0
		QString sCertificate = oCertificateSsl.subjectInfo(QSslCertificate::Organization).join(QLatin1Char(' '));
		MessageLog_AppendTextFormatCo(d_coBlue, "Registering certificate $Q\n", &sCertificate);
		#endif
		#if 0
		QFile file("e:/" + sCertificate + ".cer");
		if (file.open(QIODevice::WriteOnly))
			{
			file.write(oCertificateSsl.toDer());
			file.close();
			}
		#endif
		pCertificateParent = PCertificateRegister(pCertificateParent, IN (CSslCertificate &)oCertificateSsl);
		}
	//Assert(pCertificateParent != NULL);
	return pCertificateParent;	// This pointer is no longer a parent, but the last certificate registered which is the current 'peer' certificate of the socket.
	} // PCertificatesRegister()


int g_iWallet;
void
CChatConfiguration::NavigationTree_DisplayWallet()
	{
	TWallet::S_FileOpen_MB((g_iWallet++ == 0) ? (PSZUC)"My Wallet" : g_strScratchBufferStatusBar.Format("My Wallet #$i", g_iWallet));
	}

void
ITreeItem::TreeItemW_DisplayTransactionsBitcoin()
	{
	if (m_paTreeItemW_YZ->childCount() == 0)
		{
		new TWalletView(this, "Bitcoins Sent", eWalletViewFlag_kfDisplayTransactionsSent);
		new TWalletView(this, "Bitcoins Received", eWalletViewFlag_kfDisplayTransactionsReceived);
		m_paTreeItemW_YZ->setExpanded(true);
		}
	}


/*
void
CChatConfiguration::NavigationTree_AddCertificates()
	{
	CTreeItemW * poTreeItemRoot = m_oTreeItemCertificates.m_paTreeWidgetItem;
	if (poTreeItemRoot != NULL)
		{
		// The certificates are already visible
		poTreeItemRoot->setVisible(poTreeItemRoot->isHidden());	// Toggle the visibility
		return;
		}
	}
*/

QString
CChatConfiguration::SGetPathOfFileName(PSZAC pszFileNameRelative)
	{
	return m_sPathFileSave.mid(0, m_sPathFileSave.lastIndexOf('/')) + '/' + pszFileNameRelative;
	}

QString
CChatConfiguration::SGetPathOfFileName(IN const SHashSha1 * pHashFileName)
	{
	CHU szFileName[40];
	Base41_EncodeToText(OUT szFileName, IN (const BYTE *)pHashFileName, sizeof(*pHashFileName));
	strcpy(OUT (char *)szFileName + sizeof(*pHashFileName) + sizeof(*pHashFileName) / 2, ".dat");
	return SGetPathOfFileName(IN (PSZAC)szFileName);
	}

void
CChatConfiguration::XmlConfigurationExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	if (!pXmlExchanger->XmlExchangeNodeRootF("r"))
		return;	// If there is no root node, there there is nothign to do!

	m_arraypaProfiles.ForEach_UAssignObjectIds();	// Assign a unique identifier for each profile.  This is necessary to serialize the pointer of the selected profile

	//g_uPreferences = 123; // |= P_kfDontPlaySoundWhenNewMessageArrive;
	pXmlExchanger->XmlExchangeUInt("Preferences", INOUT &g_uPreferences);	// TODO: This should not be there, but in the Registry
	#if 0
	MessageLog_AppendTextFormatSev(eSeverityComment, "CChatConfiguration::XmlConfigurationExchange() - Preferences = 0x$x\n", g_uPreferences);
	#endif

	pXmlExchanger->XmlExchangeObjects('P', INOUT &m_arraypaProfiles, TProfile::S_PaAllocateProfile, this);
	if (!pXmlExchanger->m_fSerializing && m_arraypaProfiles.FIsEmpty())
		pXmlExchanger->XmlExchangeObjects('I', INOUT &m_arraypaProfiles, TProfile::S_PaAllocateProfile, this);	// Load old 'Identities' <I> which have been replaced by <P>
	m_oTreeItemCertificates.XmlExchange(INOUT pXmlExchanger);
	m_oTreeItemCertificates.InitializeAsRootCertificates();

	// Save the variables related to the configuration.  Those variables are saved last because they are sometimes modified by the serialization of other objects.
	pXmlExchanger->XmlExchangeElementBegin("d");	// Config Data
		pXmlExchanger->XmlExchangeSha1("s", INOUT &m_hashSalt);
		pXmlExchanger->XmlExchangePointer('p', PPX &m_pProfileSelected, IN &m_arraypaProfiles);
	pXmlExchanger->XmlExchangeElementEnd("d");
	}

#ifdef DEBUG_IMPORT_OLD_CONFIG_XML
//	This method should be eventually be deleted
void
TProfile::UnserializeContactsFromOldConfigXml()
	{
	Assert(g_arraypAccounts.FIsEmpty());
	Assert(m_arraypaAccountsXmpp.FIsEmpty());
	const QString sPathConfigurationBase = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.HavenChat";
	const QString sPathConfigurationXml = sPathConfigurationBase + "/Config.xml";
	CXmlExchanger oXmlExchanger(FALSE);
	EError err = oXmlExchanger.ELoadFileAndParseToXmlNodes(sPathConfigurationXml);
	if (err != errSuccess)
		return;
	MessageLog_AppendTextFormatSev(eSeverityWarning, "Importing XMPP accounts and peers from file '$Q'\n", &sPathConfigurationXml);
	if (!oXmlExchanger.XmlExchangeNodeRootF("HavenChat"))
		return;	// If there is no root node, there there is nothing to do!
	SHashSha1 * pHashSalt = const_cast<SHashSha1 *>(m_pConfigurationParent->PGetSalt());	// This is a hack to reuse the same salt as the previous configuration
	oXmlExchanger.XmlExchangeElementBegin("Settings");
		oXmlExchanger.XmlExchangeSha1("Salt", INOUT pHashSalt);
	oXmlExchanger.XmlExchangeElementEnd("Settings");
	oXmlExchanger.XmlExchangeObjects("Accounts", "Account", INOUT &m_arraypaAccountsXmpp, TAccountXmpp::S_PaAllocateAccount, this);
	// Do not import the certificates, as they will be automatically added to the configuration as Cambrian connects to the servers.
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		// Copy the chat history for every contact
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->TreeItemAccount_DisplayWithinNavigationTree();
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);
		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			pContact->_ClearFlags();
			pContact->m_strComment.Empty();
			const QString sPathFileNameContactHistoryDst = pContact->Vault_SGetPath();
			if (QFile::exists(sPathFileNameContactHistoryDst))
				continue;	// We already have a chat history, so don't overwrite
			SHashSha1 hashFileNameEvents;
			pContact->Vault_GetHashFileName(OUT &hashFileNameEvents);
			CHU szFileName[40];
			Base41_EncodeToText(OUT szFileName, IN (const BYTE *)&hashFileNameEvents, sizeof(hashFileNameEvents));
			strcpy(OUT (char *)szFileName + sizeof(hashFileNameEvents) + sizeof(hashFileNameEvents) / 2, ".dat");
			const QString sPathFileNameContactHistorySrc = sPathConfigurationBase + "/" + (const char *)szFileName;
			MessageLog_AppendTextFormatSev(eSeverityWarning, "\t Copying '$Q' to '$Q'...\n", &sPathFileNameContactHistorySrc, &sPathFileNameContactHistoryDst);
			QFile::copy(sPathFileNameContactHistorySrc, sPathFileNameContactHistoryDst);
			} // while
		} // while
	g_pTreeItemCommunication->TreeItemWidget_EnsureVisible();
	g_pwNavigationTree->NavigationTree_ExpandAllRootTreeItems();
	} // UnserializeContactsFromOldConfigXml()
#endif // DEBUG_IMPORT_OLD_CONFIG_XML

void
CChatConfiguration::XmlConfigurationLoadFromFile(const QString * psPathFileOpen)
	{
	BOOL fOpenConfigurationFile = TRUE;
	if (g_sPathConfigXmlDefault.isEmpty())
		{
		g_sPathConfigXmlDefault = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
		//MessageLog_AppendTextFormatCo(d_coGreen, "QStandardPaths::writableLocation(QStandardPaths::HomeLocation) returned '$Q'\n", &g_sPathConfigXmlDefault);
		g_sPathConfigXmlDefault += "/.Cambrian";
		QDir oDir(g_sPathConfigXmlDefault);
		if (!oDir.exists())
			{
			oDir.mkpath(".");	// Create the folder if not exist
			fOpenConfigurationFile = FALSE;	// If the folder was just created, don't attempt to open the configuration file and/or report an error if the file is missing.
			}
		g_strPathFolderDefault = g_sPathConfigXmlDefault;
		g_sPathConfigXmlDefault += "/Config.xml";
		}

	m_sPathFileSave = (psPathFileOpen != NULL) ? *psPathFileOpen : g_sPathConfigXmlDefault;
	MessageLog_AppendTextFormatSev(eSeverityNoise,  d_szApplicationName " v" d_szApplicationVersion ": Opening configuration file '$Q'...\n", &m_sPathFileSave);
	CXmlExchanger oXmlExchanger(FALSE);
	oXmlExchanger.m_pFileSizeAndMd5 = INOUT &m_hashMd5LastSave;
	EError err = oXmlExchanger.ELoadFileAndParseToXmlNodes(m_sPathFileSave);
	if (err != errSuccess && fOpenConfigurationFile)
		{
		// This is not really an error, as the first time the software is ever installed, there is no configuration file
		MessageLog_AppendTextFormatSev(eSeverityWarning, "Unable to load XML configuration file $Q\n", &m_sPathFileSave);
		}
	else
		{
		if (psPathFileOpen != NULL)
			{
			// Since we are restoring a configuration, we have to make sure we remove anything in the current configuration
			Destroy();
			g_strScratchBufferStatusBar.Format(d_szApplicationName " v" d_szApplicationVersion " - $Q", psPathFileOpen);
			g_pwMainWindow->setWindowTitle(g_strScratchBufferStatusBar);
			}
		//MessageLog_AppendTextFormatCo(coRed, "content of $Q:\n^N\n", &sPathConfiguration, &oXmlExchanger);
		XmlConfigurationExchange(INOUT &oXmlExchanger);		// Unserialize the XML configuration
		}

	// We are done unserializing, populate the Navigation Tree
	NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(m_pProfileSelected);
	} // XmlConfigurationLoadFromFile()


//	This method will not write the XML unless its content changed, or if psPathFileSaveAs is not NULL.
void
CChatConfiguration::XmlConfigurationSaveToFile(const QString * psPathFileSaveAs)
	{
	TRACE1("[$@] XmlConfigurationSaveToFile($Q)", psPathFileSaveAs);
	MessageLog_AppendTextFormatSev(eSeverityNoise, "[$@] XmlConfigurationSaveToFile($Q)\n", psPathFileSaveAs);
	AssertIsNotGarbageInitialized(IN &m_hashMd5LastSave, sizeof(m_hashMd5LastSave));	// The structure m_hashMd5LastSave should have been initialized by XmlConfigurationLoadFromFile()
	Report(m_hashMd5LastSave.cbFileSize < 1000*1000);	// Ideally, the config file should be less than 1 MB, however if it is bigger, it is not a problem, however it is a good idea to report the situation to the user
	CXmlExchanger oXmlExchanger;
	XmlConfigurationExchange(INOUT &oXmlExchanger);
	oXmlExchanger.ReverseLinkedListsElementsAndAttributes();	// For performance, the XML nodes are appended at the beginning of each list.  To preserve the order from which they were unserialized, we need to reverse all elements and attributes
	Endorse(oXmlExchanger.m_binXmlFileData.CbGetData() == 0);	// The buffer may have been temporary used
	(void)oXmlExchanger.m_binXmlFileData.PbbAllocateMemoryAndEmpty_YZ(m_hashMd5LastSave.cbFileSize + 2000);		// Empty any previous value and pre-allocate an extra 2 KB to prevent unnecessary memory re-allocations when serializing
	oXmlExchanger.SerializeTreeToXml();
	// Save only if there is an explicit path, or if the content of the XML changed.
	if (psPathFileSaveAs != NULL || oXmlExchanger.m_binXmlFileData.FAssignFileSizeAndMd5(INOUT_F_UNCH &m_hashMd5LastSave))
		{
		const QString sPathConfiguration = (psPathFileSaveAs != NULL) ? *psPathFileSaveAs : m_sPathFileSave;
		EError err = oXmlExchanger.m_binXmlFileData.BinFileWriteE(sPathConfiguration);
		if (err != errSuccess)
			{
			MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to save XML configuration file $Q\n", &sPathConfiguration);
			return;
			}
		TRACE1("Configuration saved to $Q", &sPathConfiguration);
		if (psPathFileSaveAs != NULL)
			StatusBar_SetTextFormat("Configuration saved to $Q", psPathFileSaveAs);
		}
	} // XmlConfigurationSaveToFile()


///////////////////////////////////////////////////////////////////////////////////////////////////
const QString c_sCambrianOrg("Cambrian.org");
const QString c_sChat("Chat");

OSettingsRegistry::OSettingsRegistry() : QSettings(c_sCambrianOrg, c_sChat)
	{
	}


//	Return TRUE if the configuration does not contain any accounts
BOOL
Configuration_FIsWithoutAccounts()
	{
	return (g_arraypAccounts.FIsEmpty());
	}

//	Return pointer to the selected account (if any) or the first account in the configuration.
TAccountXmpp *
Configuration_PGetAccountSelectedOrFirstAccount()
	{
	TAccountXmpp * pAccount = NavigationTree_PGetSelectedTreeItemMatchingInterfaceTAccount();
	if (pAccount != NULL)
		return pAccount;
	return (TAccountXmpp *)g_arraypAccounts.PvGetElementFirst_YZ();	// May return NULL
	}

EError
Configuration_EFileRead(IN const SHashSha1 * pHashFileName, OUT CBin * pbinFile)
	{
	Assert(pHashFileName != NULL);
	Assert(pbinFile != NULL);
	QString sFileName = g_oConfiguration.SGetPathOfFileName(IN pHashFileName);
	return pbinFile->BinFileReadE(sFileName);
	}

EError
Configuration_EFileWrite_ML(IN const SHashSha1 * pHashFileName, IN const CBin * pbinFile)
	{
	QString sFileName = g_oConfiguration.SGetPathOfFileName(IN pHashFileName);
	EError err = pbinFile->BinFileWriteE(sFileName);
	if (err != errSuccess)
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to write file $Q\n", &sFileName);
	return err;
	}

void
Configuration_FileAppend(IN const SHashSha1 * pHashFileName, const CBin * pbinAppend)
	{
	Assert(pHashFileName != NULL);
//	Guid_GenerateIfEmpty(INOUT pHashFileName);	// Create a GUID for the filename (if not already assigned)
	QString sFileName =	g_oConfiguration.SGetPathOfFileName(IN pHashFileName);
	EError err = pbinAppend->BinFileWriteE(sFileName, QIODevice::Append);
	if (err != errSuccess)
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to append to file $Q\n", &sFileName);
		}
	}

void
Configuration_FileAppend(IN const SHashSha1 * pHashFileName, INOUT CMemoryAccumulator * pAccumulator)
	{
	Assert(pHashFileName != NULL);
	Assert(pAccumulator != NULL);
	if (pAccumulator->FIsEmpty())
		return;		// Nothing to do
//	Guid_GenerateIfEmpty(INOUT pHashFileName);	// Create a GUID for the filename (if not already assigned)
	QString sFileName =	g_oConfiguration.SGetPathOfFileName(IN pHashFileName);
	EError err = pAccumulator->EAppendToFileAndEmpty(sFileName);
	if (err != errSuccess)
		{
		MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to append to file $Q\n", &sFileName);
		}
	}

#pragma GCC diagnostic ignored "-Wswitch"
void
Configuration_GlobalSettingsPresenceUpdate(EMenuAction eMenuAction)
	{
	Assert(eMenuAction < eMenuAction_PresenceLast);
	g_uPreferences &= ~P_kmPresenceMask;	// Remove the previous presence
	switch (eMenuAction)
		{
	case eMenuAction_PresenceAway:
	case eMenuAction_PresenceBusy:
	case eMenuAction_PresenceInvisible:
		g_uPreferences |= eMenuAction;	// Remember only those presences (the other presences are dynamic depending on the context of each account)
		}
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->PresenceUpdateFromGlobalSettings(eMenuAction);
		}
	NavigationTree_UpdatePresenceIcon(eMenuAction);
	/*
	extern QToolButton * g_pwButtonStatusOfNavigationTree;
	Widget_SetIconButton(INOUT g_pwButtonStatusOfNavigationTree, eMenuAction);
	g_pwButtonStatusOfNavigationTree->setText(PGetMenuAction(eMenuAction)->text());
	*/
	}

QString c_sFilter = "SocietyPro Configuration (*.xml)";	// This string should be 'const' however the function getSaveFileName() accepts only a QString* without 'const'
/*
void
Configuration_SetPathDefault(const QString & strPath)
	{
	g_oConfiguration.m_sPathFileSave = strPath;
	}
*/
void
Configuration_Save()
	{
	g_oConfiguration.XmlConfigurationSaveToFile();
	TWallet::S_SaveAll();
	}

void
Configuration_SaveAs()
	{
	Configuration_Save();	// Save the current configuration to its usual place (it is always a good idea to regularly save the configuration, and this is a good opportunity).  The idea is also to have identical XML files if one day there is a need to compare different configurations.

	// Prompt the user for the filename to save the backup configuration
	QString sFileSave = QFileDialog::getSaveFileName(g_pwMainWindow, "Backup Configuration", c_sEmpty, c_sFilter, IN &c_sFilter);
	if (!sFileSave.isEmpty())
		{
		//MessageLog_AppendTextFormatCo(d_coGreen, "Saving backup configuration to $Q...\n", &sFileSave);
		g_oConfiguration.XmlConfigurationSaveToFile(IN &sFileSave);
		}
	}

void
Configuration_Restore()
	{
	QString sFileOpen = QFileDialog::getOpenFileName(g_pwMainWindow, "Restore Configuration", c_sEmpty, c_sFilter, IN &c_sFilter);
	if (!sFileOpen.isEmpty())
		{
		g_oConfiguration.XmlConfigurationLoadFromFile(IN &sFileOpen);
		}
	}

//	Loop through every account to make sure they are connected
//	This function is peridiocally called by a timer to make sure the accounts are permanently connected
void
Configuration_NetworkReconnectIfDisconnected()
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->Socket_ReconnectIfDisconnected();
		}
	}

void
Configuration_OnTimerNetworkIdle()
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_arraypAccounts.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		pAccount->OnTimerNetworkIdle();
		}
	}

/*
//	Global function to delete a message.
//	Since the scope of those messages has not been defined, and after being deleted they
//	may remain in some history until the application closes, it is better to have a globalfunction to handle them.
void
Configuration_RemoveMessage(PA_DELETING WNotice * pawMessage)
	{
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = g_oConfiguration.m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		if (pAccount->WNotice_FRemoveFromList(pawMessage))
			{
			delete pawMessage;
			return;
			}
		}
	Assert(FALSE && "Message not found");
	}
*/
