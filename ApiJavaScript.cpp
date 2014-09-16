//	ApiJavaScript.cpp

#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "ApiJavaScript.h"
#include "TApplicationBallotmaster.h"
#include "IEventBallot.h"


OJapiApps::OJapiApps(OJapiCambrian * poCambrian)
	{
    m_poCambrian = poCambrian;
	}

POJapiAppBallotmaster
OJapiApps::ballotmaster()
	{
	//MessageLog_AppendTextFormatCo(d_coGreen, "apps()");
	return m_poCambrian->polls();
	}

POJapiApps
OJapiCambrian::apps()
	{
	return &m_oApps;
	}




///////////////////////////////////////////////////////////////////////////////////////////////////
OJapiMe::OJapiMe(OJapiCambrian * poCambrian)
	{
	m_poCambrian = poCambrian;
	}

OJapiList
OJapiMe::groups()
	{
	CListVariants oList(m_poCambrian);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		oList.AddGroupsMatchingType(IN pAccount->m_arraypaGroups, eGroupType_Open);
		oList.AddGroupsMatchingType(IN pAccount->m_arraypaGroups, eGroupType_Audience);
		/*
		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
			//oList.append(pGroup->m_strNameDisplayTyped.ToQString());
			//oList.append(QVariant::fromValue(new OJapiGroup(pGroup)));
			oList.append(QVariant::fromValue(pGroup->POJapiGet()));
			} // while
		*/
		} // while
	//MessageLog_AppendTextFormatCo(d_coRed, "OJapiMe::groups() (length = $i)\n", oList.length());
	return oList;
	}
/*
OJapiList
OJapiMe::peerLists()
	{
	CListVariants oList(m_poCambrian);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		oList.AddGroupsMatchingType(IN pAccount->m_arraypaGroups, eGroupType_Audience);
		} // while
	MessageLog_AppendTextFormatCo(d_coBlack, "Groups List length end = $i\n", oList.length());
	return oList;
	}
*/
OJapiList
OJapiMe::peers()
	{
	CListVariants oList(m_poCambrian);
	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;
		oList.AddContacts(IN pAccount->m_arraypaContacts);
		}
		/*
		TContact ** ppContactStop;
		TContact ** ppContact = pAccount->m_arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);

		while (ppContact != ppContactStop)
			{
			TContact * pContact = *ppContact++;
			Assert(pContact != NULL);
			Assert(pContact->EGetRuntimeClass() == RTI(TContact));
			//oList.append(QVariant::fromValue(new OJapiContact(pContact)));
			oList.append(QVariant::fromValue(pContact->POJapiGet()));
			} // while
		} // while
		*/
	return oList;
}

/*
POJapiGroup
OJapiMe::newPeerList()
	{
	TAccountXmpp *pAccount = (TAccountXmpp*) m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
	if ( pAccount == NULL)
		return NULL;
	TGroup *paGroup = pAccount->Group_PaAllocateTemp(eGroupType_Audience);
	m_poCambrian->m_arraypaTemp.Add(PA_CHILD paGroup);
	return paGroup->POJapiGet(m_poCambrian);
	}

POJapiGroup
OJapiMe::newGroup()
{
TAccountXmpp *pAccount = (TAccountXmpp*) m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
	if ( pAccount == NULL)
		return NULL;
	TGroup *paGroup = pAccount->Group_PaAllocateTemp(eGroupType_Open);
	m_poCambrian->m_arraypaTemp.Add(PA_CHILD paGroup);
	return paGroup->POJapiGet(m_poCambrian);
}
*/

POJapiGroup
OJapiMe::newGroup(const QString &type)
	{
	TAccountXmpp *pAccount = (TAccountXmpp*) m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PvGetElementFirst_YZ();
	if ( pAccount == NULL)
		return NULL;

	EGroupType eGroupType = eGroupType_Open;
	if ( type.compare("Open", Qt::CaseInsensitive) == 0)
		eGroupType = eGroupType_Open;
	else if ( type.compare("Broadcast", Qt::CaseInsensitive) == 0 )
		eGroupType = eGroupType_Audience;
	else
		return NULL;

	TGroup *paGroup = pAccount->Group_PaAllocateTemp(eGroupType);
	m_poCambrian->m_arraypaTemp.Add(PA_CHILD paGroup);
	return paGroup->POJapiGet(m_poCambrian);
	}

/*
POJapiGroup
OJapiMe::getPeerList(const QString & sId)
	{
	return getGroup(sId);
	}
*/

POJapiGroup
OJapiMe::getGroup(const QString & sId)
	{
	SHashSha1 hashId ;
	CStr strId = sId;
	HashSha1_FInitFromStringBase85_ZZR_ML(OUT &hashId, strId);

	TAccountXmpp ** ppAccountStop;
	TAccountXmpp ** ppAccount = m_poCambrian->m_pProfile->m_arraypaAccountsXmpp.PrgpGetAccountsStop(OUT &ppAccountStop);
	while (ppAccount != ppAccountStop)
		{
		TAccountXmpp * pAccount = *ppAccount++;

		TGroup ** ppGroupStop;
		TGroup ** ppGroup = pAccount->m_arraypaGroups.PrgpGetGroupsStop(OUT &ppGroupStop);
		while (ppGroup != ppGroupStop)
			{
			TGroup * pGroup = *ppGroup++;
			Assert(pGroup != NULL);
			Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));

			if ( HashSha1_FCompareEqual( &pGroup->m_hashGroupIdentifier, &hashId  ))
				return pGroup->POJapiGet(m_poCambrian);
			} // while

		} // while
	return NULL;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
POJapiGroup
TGroup::POJapiGet(OJapiCambrian * poCambrian) CONST_MCC
	{
	if (m_paoJapiGroup == NULL)
		m_paoJapiGroup = new OJapiGroup(this, poCambrian);
	return m_paoJapiGroup;
	}

OJapiGroup::OJapiGroup(TGroup * pGroup, OJapiCambrian * poCambrian)
	{
	m_pGroup = pGroup;
	m_poCambrian = poCambrian;
	}

QString
OJapiGroup::id()
	{
	/*
	CStr str;
	str.Format("$p", this);
	return str;
	*/
	return HashSha1_ToQStringBase85(IN &m_pGroup->m_hashGroupIdentifier);
	}

QString
OJapiGroup::name()
	{
	return m_pGroup->m_strNameDisplayTyped;
	}

void
OJapiGroup::name(const QString &sName)
	{
	MessageLog_AppendTextFormatCo(d_coBlue, "OJapiGroup.name($Q)\n", &sName);
	m_pGroup->m_strNameDisplayTyped = sName;
	}

int
OJapiGroup::count()
	{
	return m_pGroup->m_arraypaMembers.GetSize();
	}

OJapiList
OJapiGroup::members()
	{
	CListVariants oList(m_poCambrian);
	TGroupMember ** ppGroupMemberStop;
	TGroupMember ** ppGroupMember = m_pGroup->m_arraypaMembers.PrgpGetMembersStop(OUT &ppGroupMemberStop);
	while ( ppGroupMember != ppGroupMemberStop )
		{
		TGroupMember * pGroupMember = *ppGroupMember++;
		oList.AddContact(pGroupMember->m_pContact);
		}
	return oList;
	}

QString OJapiGroup::type()
	{
	if ( m_pGroup->m_eGroupType == eGroupType_Audience)
		return "broadcast";
	else if (m_pGroup->m_eGroupType == eGroupType_Open)
		return "open";
	return c_sEmpty;
	}

void
OJapiGroup::addPeer(QObject *pContactAdd)
	{
	OJapiContact * pContact = qobject_cast<OJapiContact *>(pContactAdd); // Make sure we received an object of proper type
	MessageLog_AppendTextFormatCo(d_coBlue, "addPeer($S)\n", &pContact->m_pContact->m_strJidBare);
	m_pGroup->Member_PFindOrAddContact_NZ(pContact->m_pContact);
	}

void
OJapiGroup::removePeer(QObject *pContactRemove)
	{
	OJapiContact * pContact = qobject_cast<OJapiContact *>(pContactRemove); // Make sure we received an object of proper type
	TGroupMember **ppGroupMemberStop;
	TGroupMember **ppGroupMember = m_pGroup->m_arraypaMembers.PrgpGetMembersStop(&ppGroupMemberStop);
	while ( ppGroupMember != ppGroupMemberStop )
		{
		TGroupMember *pGroupMember = *ppGroupMember++;
		if ( pGroupMember->m_pContact == pContact->m_pContact)
			{
			//MessageLog_AppendTextFormatCo(d_coBlue, "removing Peer ");
			m_pGroup->Member_Remove_UI(pGroupMember);
			}
		}
	}

void
OJapiGroup::save()
	{
	Assert(m_pGroup != NULL);
	m_pGroup->TreeItemFlags_SerializeToDisk_Yes();
	m_pGroup->m_pAccount->m_arraypaGroups.ElementTransferFrom(m_pGroup, INOUT &m_poCambrian->m_arraypaTemp);	// Transfer the group from 'temp' to the account

	// for normal groups
	if ( m_pGroup->m_eGroupType == eGroupType_Open && m_pGroup->m_paTreeItemW_YZ == NULL )
		m_pGroup->TreeItemW_DisplayWithinNavigationTree(m_pGroup->m_pAccount, eMenuAction_Group );

	/*
	if (m_poCambrian->m_arraypaTemp.RemoveElementFastF(m_pGroup))
		m_pGroup->m_pAccount->m_arraypaGroups.Add(m_pGroup);
	*/
	}

void
OJapiGroup::destroy()
	{
	Assert(m_pGroup != NULL);
	//if (m_pGroup->m_eGroupType != eGroupType_Audience)
	//	return;	// Don't allow a JavaScript to delete a regular group; only a 'peerlist'

	m_pGroup->TreeItemGroup_RemoveFromNavigationTree();
	m_pGroup->Group_MarkForDeletion();
	m_poCambrian->m_arraypaTemp.ElementTransferFrom(m_pGroup, INOUT &m_pGroup->m_pAccount->m_arraypaGroups);	// Transfer the group back to the 'temp' array


	/*
	if ( !m_pGroup || m_pGroup->m_eGroupType != eGroupType_Audience)
		return;
	if ( m_poCambrian->m_arraypaTemp.DeleteRuntimeObjectF(m_pGroup))
		return;
	m_pGroup->m_pAccount->m_arraypaGroups.DeleteRuntimeObject(m_pGroup);
	m_pGroup = NULL;

	*/
	}



///////////////////////////////////////////////////////////////////////////////////////////////////
POJapiContact
TContact::POJapiGet()
	{
	if (m_paoJapiContact == NULL)
		m_paoJapiContact = new OJapiContact(this);
	return m_paoJapiContact;
	}

OJapiContact::OJapiContact(TContact * pContact)
	{
	m_pContact = pContact;
	}

QString
OJapiContact::id()
	{
	return m_pContact->m_strJidBare;
	}

QString
OJapiContact::name()
	{
	return m_pContact->m_strNameDisplayTyped;
	}



///////////////////////////////////////////////////////////////////////////////////////////////////

CListVariants::CListVariants(OJapiCambrian *poCambrian)
	{
	m_poCambrian = poCambrian;
	}

void
CListVariants::AddContact(TContact *pContact)
	{
	this->append(QVariant::fromValue(pContact->POJapiGet()));
	}

void
CListVariants::AddContacts(const CArrayPtrContacts &arraypaContacts)
	{
	TContact ** ppContactStop;
	TContact ** ppContact = arraypaContacts.PrgpGetContactsStop(OUT &ppContactStop);

	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		Assert(pContact != NULL);
		Assert(pContact->EGetRuntimeClass() == RTI(TContact));
		AddContact(pContact);
		} // while
	}

void
CListVariants::AddGroupMatchingType(TGroup *pGroup, EGroupType eGroupType)
	{
	Assert(pGroup != NULL);
	if ( pGroup->m_eGroupType == eGroupType && !pGroup->TreeItemFlags_FuIsInvisible() )
	{
		append(QVariant::fromValue(pGroup->POJapiGet(m_poCambrian)));
	}
}

void
CListVariants::AddGroupsMatchingType(const CArrayPtrGroups &arraypGroups, EGroupType eGroupType)
	{
	TGroup **ppGroupsStop;
	TGroup **ppGroup = arraypGroups.PrgpGetGroupsStop(&ppGroupsStop);
		while( ppGroup != ppGroupsStop)
		{
		TGroup *pGroup = *ppGroup++;
		AddGroupMatchingType(pGroup, eGroupType);
		}
	}

void
CListVariants::AddGroup(TGroup *pGroup)
	{
	AddGroupMatchingType(pGroup, eGroupType_Open);
	}

void
CListVariants::AddAudience(TGroup *pGroup)
	{
	AddGroupMatchingType(pGroup, eGroupType_Audience);
	}

//////////////////////////////////////////////////////////////////////////////////


const QString
OJapiBrowserTab::title()
	{
	return m_pTab->m_strNameDisplayTyped.ToQString();
	}

OJapiBrowserTab::OJapiBrowserTab(TBrowserTab *pTab, OJapiBrowsersList *pBrowsersListParent)
	{
	Assert(pTab != NULL);
	Assert(pBrowsersListParent != NULL);
	m_pTab = pTab;
	m_pBrowsersListParent = pBrowsersListParent;
	}

void
OJapiBrowserTab::back()
	{
	Assert(m_pTab != NULL);
	m_pTab->NavigateBack();
	}

void
OJapiBrowserTab::forward()
	{
	Assert(m_pTab != NULL);
	m_pTab->NavigateForward();
	}

void
OJapiBrowserTab::reload()
	{
	Assert(m_pTab != NULL);
	m_pTab->NavigateReload();
}

void
OJapiBrowserTab::close()
	{
	// TODO: close
	}

void
OJapiBrowserTab::openApp(const QString & appName)
	{
	CStr sAppName(appName);
	const SApplicationHtmlInfo *pInfo = PGetApplicationHtmlInfo(sAppName.PszaGetUtf8NZ());
	if ( pInfo != NULL )
		{
		/*???*/
		CStr url = "file:///" + m_pBrowsersListParent->m_poJapiProfileParent_NZ->m_pProfile->m_pConfigurationParent->SGetPathOfFileName(pInfo->pszLocation);
		m_pTab->SetUrl(url);
		}
	}

void
OJapiBrowserTab::openUrl(const QString & url)
	{
	CStr sUrl(url);
	m_pTab->SetUrl(sUrl);
	}

POJapiBrowserTab
TBrowserTab::POJapiGet(OJapiBrowsersList *pBrowsersList)
	{
	if ( m_paoJapiBrowser == NULL)
		m_paoJapiBrowser = new OJapiBrowserTab(this, pBrowsersList);

	return m_paoJapiBrowser;
	}





OJapiBrowsersList::OJapiBrowsersList(OJapiProfile *poProfile)
	{
	Assert(poProfile != NULL);
	m_poJapiProfileParent_NZ = poProfile;
	}

POJapiBrowserTab
OJapiBrowsersList::PGetCurrentTab_YZ()
	{
	TBrowserTabs *pBrowserTabs = PGetBrowser_YZ();
	if ( pBrowserTabs != NULL)
		{
		TBrowserTab *pTab = pBrowserTabs->PGetCurrentBrowserTab_YZ();
		if ( pTab != NULL )
			return pTab->POJapiGet(this);
		}

	return NULL;
	}

TBrowserTabs*
OJapiBrowsersList::PGetBrowser_YZ()
	{
	return (TBrowserTabs*) m_poJapiProfileParent_NZ->m_pProfile->m_arraypaBrowsersTabbed.PvGetElementFirst_YZ();
	}

QVariantList
OJapiBrowsersList::listBrowsers()
	{
	QVariantList list;
	TBrowserTabs *pBrowserTabs = PGetBrowser_YZ();
	if ( pBrowserTabs )
		{
		TBrowserTab **ppBrowserTabStop;
		TBrowserTab **ppBrowserTab = pBrowserTabs->m_arraypaTabs.PrgpGetBrowserTabStop(&ppBrowserTabStop);
		while( ppBrowserTab != ppBrowserTabStop )
			{
			TBrowserTab *pBrowser = *ppBrowserTab++;
			list.append(QVariant::fromValue(pBrowser->POJapiGet(this) ));
			}
		}
	return list;
	}

POJapiBrowserTab
OJapiBrowsersList::newBrowser()
	{
	TBrowserTabs *pBrowserTabs = PGetBrowser_YZ();
	TProfile *pProfile = m_poJapiProfileParent_NZ->m_pProfile;

	if ( !pBrowserTabs )
		{
		// create browser
		CStr sTreeItemName("Web Browser");
		pBrowserTabs = new TBrowserTabs(pProfile);
		pBrowserTabs->SetIconAndName(eMenuAction_DisplaySecureWebBrowsing, sTreeItemName);
		pProfile->m_arraypaBrowsersTabbed.Add(PA_CHILD pBrowserTabs);
		pBrowserTabs->TreeItemBrowser_DisplayWithinNavigationTree();
		}

	// add a new tab
	TBrowserTab *pTab = pBrowserTabs->AddTab();
	pBrowserTabs->TreeItemW_SelectWithinNavigationTree();

	return pTab->POJapiGet(this);
	}





POJapiProfile
TProfile::POJapiGet()
	{
	if ( m_paoJapiProfile == NULL)
		m_paoJapiProfile = new OJapiProfile(this);

	return m_paoJapiProfile;
}

OJapiCambrian *TProfile::POJapiGetCambrian()
{
			return new OJapiCambrian(this, NULL);
}

OJapiProfile::OJapiProfile(TProfile *pProfile) : m_oBrowsersList(this)
	{
	m_pProfile = pProfile;
	}

QString
OJapiProfile::id()
	{
	return m_pProfile->m_binKeyPublic.ToQString();
	}

QString
OJapiProfile::name()
	{
	return m_pProfile->m_strNameProfile.ToQString();
	}

POJapiBrowsersList
OJapiProfile::browsers()
	{
	return &m_oBrowsersList;
	}

void
OJapiProfile::destroy()
	{
	// TODO
	}





OJapiProfilesList::OJapiProfilesList(OCapiRootGUI *pRootGui)
	{
	m_pRootGui = pRootGui;
	}

TProfile*
OJapiProfilesList::PGetCurrentProfile()
	{
	return g_oConfiguration.m_pProfileSelected;
	}

POJapiProfile
OJapiProfilesList::currentProfile()
	{
	return PGetCurrentProfile()->POJapiGet();
	}

void
OJapiProfilesList::setCurrentProfile(POJapiProfile poJapiProfile)
	{
	OJapiProfile *pProfile = qobject_cast<OJapiProfile*>(poJapiProfile);
	//MessageLog_AppendTextFormatCo(d_coRed, "setCurrentProfile $p\n", pProfile);

	if ( pProfile != NULL )
		{
		NavigationTree_PopulateTreeItemsAccordingToSelectedProfile(pProfile->m_pProfile);
		roleChanged();
		}
	}

QVariantList
OJapiProfilesList::list()
	{
	QVariantList list;
	//MessageLog_AppendTextFormatCo(d_coRed, "OJapiProfilesList::list() \n");

	TProfile **ppProfilesStop;
	TProfile **ppProfiles = g_oConfiguration.m_arraypaProfiles.PrgpGetProfilesStop(&ppProfilesStop);
	while(ppProfiles != ppProfilesStop)
		{
		TProfile *pProfile = *ppProfiles++;
		list.append( QVariant::fromValue(pProfile->POJapiGet()) );
		//list.append( QVariant::fromValue(pProfile->m_strNameProfile.ToQString()) );
		}

	return list;
	}

POJapiProfile
OJapiProfilesList::create(const QString &name)
	{
	return NULL; // TODO
	}


OCapiRootGUI::OCapiRootGUI() : m_oProfiles(this)
	{
	}

POJapiProfilesList
OCapiRootGUI::roles()
	{
	return &m_oProfiles;
}

// this enum must be in sync with g_rgApplicationHtmlInfo[]
enum EApplicationHtmlinfo
	{
	eApplicationHtmlInfoBallotmaster = 9,
	};

SApplicationHtmlInfo g_rgApplicationHtmlInfo[] =
{
	{"Navshell Peers"	 , "Apps/navshell-contacts/index.html"		, PaAllocateJapiGeneric, NULL },
	{"Navshell Sidebar"  , "Apps/navshell-stack/index.html"			, PaAllocateJapiGeneric, NULL },
	{"Navshell Header"   , "Apps/navshell-header/index.html"		, PaAllocateJapiGeneric, NULL },
	{"Office Kingpin"    , "Apps/html5-office-kingpin/index.html"	, PaAllocateJapiGeneric, NULL },
	{"Pomodoro"          , "Apps/html5-pomodoro/index.html"			, PaAllocateJapiGeneric, NULL },
	{"JAPI Tests"        , "Apps/japi/test/test.html"				, PaAllocateJapiGeneric, NULL },
	{"Scratch"           , "Apps/html5-scratch/index.html"			, PaAllocateJapiGeneric, NULL },
	{"HTML5 xik"         , "Apps/html5-xik/index.html"				, PaAllocateJapiGeneric, NULL },
	{"Group Manager"	 , "Apps/html5-group-manager/index.html"	, PaAllocateJapiGeneric, NULL },
	{"Ballotmaster"		 , "Apps/html5-pollmaster/index.html"		, PaAllocateJapiGeneric, NULL },
	{"Home"		         , "Apps/html5-scratch/index.html"			, PaAllocateJapiGeneric, NULL },
};

SApplicationHtmlInfo * PGetApplicationHtmlInfoBallotmaster() { return &g_rgApplicationHtmlInfo[9]; }

// This function was moved to make the code compile
const SApplicationHtmlInfo *
PGetApplicationHtmlInfo(PSZAC pszNameApplication)
	{
	//MessageLog_AppendTextFormatCo(d_coRed, "sizeof=$i\n", sizeof(c_rgApplicationHtmlInfo)/sizeof(SApplicationHtmlInfo) );
	const SApplicationHtmlInfo * pInfo = g_rgApplicationHtmlInfo;
	while (pInfo != g_rgApplicationHtmlInfo + LENGTH(g_rgApplicationHtmlInfo))
		{
		if (FCompareStringsNoCase((PSZUC) pInfo->pszName, (PSZUC)pszNameApplication))
			return pInfo;
		pInfo++;
		}
	return NULL;
	}


QVariantList OCapiRootGUI::apps()
	{
	QVariantList list;
	for (SApplicationHtmlInfo * pInfo = &g_rgApplicationHtmlInfo[0]; pInfo != g_rgApplicationHtmlInfo + LENGTH(g_rgApplicationHtmlInfo); pInfo++)
		{
		if (pInfo->paoJapi == NULL)
			pInfo->paoJapi = pInfo->pfnPaAllocateJapi(pInfo);

		list.append( QVariant::fromValue(pInfo->paoJapi) );
		}

	return list;
	}

POJapiNotificationsList
OCapiRootGUI::notifications()
	{
	return &m_oNotificationsList;
	}

POJapiPeerRequestsList
OCapiRootGUI::peerRequests()
	{
	return &m_oPeerRequestsList;
	}

OCapiImageProvider::OCapiImageProvider()  : QQuickImageProvider(QQuickImageProvider::Pixmap)
	{
	}

QPixmap OCapiImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
	{
	/*
	 * image://sopro/[:id:]
	 *
	 * expected id values
	 *  - roles/current
	 *	- roles/[:roleName:]
	 *	- appInfo/[:appName:]
	 *
	 * TODO: return actual images
	 */

	QStringList srgParts = id.split("/", QString::SkipEmptyParts);
	QString sFirst = srgParts.first();
	QPixmap profilepic;

	if ( sFirst.compare("roles") == 0 )
		{
		profilepic.load(":/ico/IconHaven");
		}
	else if ( sFirst.compare("appInfo") == 0 )
		{
		profilepic.load(":/ico/IconHaven");
		}

	return profilepic;
	}




///////////////////////////////////////////////////////////////////////////////////

OJapiGroupList::OJapiGroupList(OJapiCambrian *poCambrian)
	{
	m_poCambrian = poCambrian;
	}


POJapiGroup
OJapiGroupList::build(const QString &type)
	{
	return m_poCambrian->m_oMe.newGroup(type);
	}

OJapiList
OJapiGroupList::list()
	{
	return m_poCambrian->m_oMe.groups();
	}


POJapiGroup
OJapiGroupList::get(const QString & sId)
	{
	return m_poCambrian->m_oMe.getGroup(sId);
	}


////////////////////////////////////////// Utilities /////////////////////////////////////

QString
OJapiUtil::base64encode(const QString & sText)
	{
	CStr strBase64;
	strBase64.InitFromTextEncodedInBase64(sText);
	return strBase64;
	}

QString
OJapiUtil::base64decode(const QString &sBase64)
	{
	return QByteArray::fromBase64(sBase64.toUtf8());
}

///////////////////////////////////////////////////////
QString
OJapiPollAttatchment::name()
	{
	return m_pBallotAttatchment->m_strName;
	}

QString
OJapiPollAttatchment::mimeType()
	{
	return m_pBallotAttatchment->m_strMimeType;
	}

QString
OJapiPollAttatchment::content()
	{
	MessageLog_AppendTextFormatCo(d_coRed, "PollAttatchment::contetn()\n");
	CStr strBase64;
	strBase64.BinAppendStringBase64FromBinaryData(&m_pBallotAttatchment->m_binContent);
	return strBase64;
	}

OJapiPollAttatchment::OJapiPollAttatchment(CEventBallotAttatchment *pBallotAttatchment)
	{
	Assert(pBallotAttatchment != NULL);
	m_pBallotAttatchment = pBallotAttatchment;
	}

void
OJapiPollAttatchment::destroy()
	{
	MessageLog_AppendTextFormatCo(d_coBlack, "OJapiPollAttatchment::destroy\n");
	CArrayPtrPollAttatchments * parraypaAtattchments = &m_pBallotAttatchment->m_pPollParent->m_arraypaAtattchments;
	CEventBallotAttatchment **ppBallotAttatchmentStop;
	CEventBallotAttatchment **ppBallotAttatchment = parraypaAtattchments->PrgpGetAttatchmentsStop(&ppBallotAttatchmentStop);
	while ( ppBallotAttatchment != ppBallotAttatchmentStop)
		{
		CEventBallotAttatchment * pBallotAttatchment = *ppBallotAttatchment++;
		if (pBallotAttatchment == m_pBallotAttatchment)
			{
			parraypaAtattchments->RemoveElementI(pBallotAttatchment);
			delete pBallotAttatchment;
			return;
			}
		}
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unable to destroy attatchment $S\n", &m_pBallotAttatchment->m_strName);
	}





OJapiAppInfo:: OJapiAppInfo(const SApplicationHtmlInfo *pApplicationInfo)
	{
	Assert ( pApplicationInfo != NULL );
	m_pApplicationInfo = pApplicationInfo;
	}

QString OJapiAppInfo::name()
	{
	Assert ( m_pApplicationInfo != NULL );
	return QString(m_pApplicationInfo->pszName);
	}

QString OJapiAppInfo::tooltip()
	{
	return "SoPro Application";
	}

QString OJapiAppInfo::launchUrl()
	{
	Assert ( m_pApplicationInfo != NULL );
	return QString(m_pApplicationInfo->pszLocation);
	}

QString OJapiAppInfo::iconUrl()
	{
	Assert ( m_pApplicationInfo != NULL );
	return "image://application/" + QString(m_pApplicationInfo->pszName);
}


/////////////////////////////////////////////////////

POJapi
IEvent::POJapiGet()
	{
	if ( m_paoJapiEvent == NULL)
		m_paoJapiEvent = new OJapiNotification();

	return m_paoJapiEvent;
	}


OJapiNotification::OJapiNotification(IEvent *pEvent)
	{
	}

QString
OJapiNotification::title()
	{
	return "Incoming File Transfer";
	}

QString
OJapiNotification::text()
	{
	return "corp2014.pdf 4.3 Mb";
	}

QDateTime
OJapiNotification::date()
	{
	return QDateTime::currentDateTime();
	}

QString
OJapiNotification::cardLink()
	{
	return "cardLink";
	}

QString
OJapiNotification::actionLabel()
	{
	return "ACCEPT";
	}

QString
OJapiNotification::actionLink()
	{
	return "actionLink";
	}

void
OJapiNotification::clear()
	{
	// TODO: remove this notification from the list
	}

QVariantList
OJapiNotificationsList::recent(int nMax)
	{
	QVariantList list;
	list.append(QVariant::fromValue(new OJapiNotification()));/*??? memory leak */
	list.append(QVariant::fromValue(new OJapiNotification()));
	list.append(QVariant::fromValue(new OJapiNotification()));

	return list;
	}

void
OJapiNotificationsList::clearRecent()
	{
	// TODO: remove all notifications from the list
	}





OJapiPeerRequestsList::OJapiPeerRequestsList()
	{
	}

OJapiPeerRequestsList::~OJapiPeerRequestsList()
	{
	}

QVariantList
OJapiPeerRequestsList::list(int nMax)
	{
	QVariantList list;
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	list.append(QVariant::fromValue(new OJapiPeerRequest()));
	return list;
	}



QString
OJapiPeerRequest::id()
	{
	return "plato@xmpp.cambrian.org";
	}

QString
OJapiPeerRequest::name()
	{
	return "Plato";
	}
