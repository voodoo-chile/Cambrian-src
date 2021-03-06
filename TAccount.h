///////////////////////////////////////////////////////////////////////////////////////////////////
//	TAccount.h
//
//	Class holding an account for communication purpose, either instant messaging (XMPP), email or something else.
//	Each account maintains a list of contacts, and other relevant information such as groups, circles and marketplace.
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TACCOUNT_H
#define TACCOUNT_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
//#ifdef WANT_TREE_NODE_NEW_CONTACT		// Code to add the node <New Contact...>

//	Base class for a chat account.
class TAccountCore : public ITreeItemChatLog
{
public:
	TProfile * m_pProfileParent;		// Pointer to the profile where the account is associated with.  The profile contains the pointer of the configuration where the data is saved on disk.
	CStr m_strServerName;					// Name of the server, such as example.com, jabber.example.com, chat.cambrian.org.  The server name could be an IP address, such as "192.168.1.2"
	quint16 m_uServerPort;					// Port to connect to the server
	enum
		{
		FAS_kmStatusPresenceMask	= 0x000F,	// Status/presence value from eMenuAction_Presence*
		FAS_kfMessageUnread			= 0x0080,	// The account has at least one unread message (and therefore a special icon should be displayed in the Navigation Tree)
		FAS_kfAccountAutoGenerated	= 0x0100	// The account was automatically generated
		};
	quint16 m_uFlagsAccountStatus;			// Flags regarding the account status (this variable is serialized)

protected:
	TAccountCore(TProfile * pProfileParent);
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);	// From IXmlExchange
public:
	PSZUC PszGetPresenceStatus() const;
	friend class CArrayPtrAccounts;
};

class CArrayPtrAccountsCore : public CArrayPtrTreeItems
{
public:
	inline TAccountCore ** PrgpGetAccountsStop(OUT TAccountCore *** pppAccountStop) const { return (TAccountCore **)PrgpvGetElementsStop(OUT (void ***)pppAccountStop); }

};

enum EFindContact
	{
	eFindContact_zDefault					= 0x00,	// Return NULL if there is no contact matching the JID.  This parameter will also return an invisible contact
	eFindContact_kfMakeVisible				= 0x01,	// If the contact is invisible, then make it appear in the Navigation Tree
	eFindContact_kmMakeVisibleOrCreateNew	= 0x03,	// Combination of eFindContact_kfMakeVisible and eFindContact_kfCreateNew
	eFindContact_kfCreateNew				= 0x02,	// Create a new contact if there is no contact matching the JID
	eFindContact_kfCreateAsUnsolicited		= 0x04,	// Create an unsolicited contact if there is no contact matching the JID
	eFindContact_kmCreateAsUnsolicited		= eFindContact_kfCreateAsUnsolicited | eFindContact_kfCreateNew
	};


///////////////////////////////////////////////////////////////////////////////////////////////////
class TAccountXmpp : public TAccountCore
{
	RTI_IMPLEMENTATION(TAccountXmpp)
public:
	CStr m_strJID;			// Bare Jabber Identifier (this is similar as an email address format: username@servername).  The content of this variable MUST be in lowercase.
	CStr m_strJIDwithResource;		// The JID with the resource (username@server/resource).  This JID is never empty: it is either initialized from the Bare JID or from the stanza <bind>.
	CStr m_strUsername;		// The username is typically the JID (in lowercase) until the @, however because XMPP authentication requires the an exact username with proper case, it is more reliable to store the username in a separate field rather than extracting it from the JID, as the user may type his/her JID with different case than how the username is stored on the server.
	CStr m_strPassword;		// Password for the XMPP account.  This password will be hashed with the username during XMPP authentication.
	CArrayPtrContacts m_arraypaContacts;	// Contacts related to the XMPP account.
	CArrayPtrGroups m_arraypaGroups;		// Groups and channels related to the account
	CArrayPtrContacts m_arraypContactsComposing;		// All contacts currently composing (this list is important for optimization)

protected:
	TAccountAlias * m_paAlias;	// Each account has a corresponding alias under the "Profile" node
	CArrayPtrContacts m_arraypContactsMessagesUnread;	// List of contacts with unread messages.  When this list is empty, then the icon 'new message' is removed from the Navigation Tree.
	CSocketXmpp * m_paSocket;							// Socket to send and receive XMPP messages.
	CBin m_binFingerprintCertificate;					// The fingerprint (typically SHA-1) of the pinned certificate encrypting the connection with the server.  Any change to this value will cutoff the socket communication until manually approved by the user.
	TCertificateServerName * m_pCertificateServerName;	// Pinned certificate for the account.  Any change of certificate automatically notifies the user, and often require manual approval by the user.
	#ifdef WANT_TREE_NODE_NEW_CONTACT
	TContactNew * m_pTreeItemContactNew;				// Pointer to the node <New Contact...>   (this pointer is necessary so we may insert new contact(s) and group(s) before it)
	#endif

public:
	TAccountXmpp(TProfile * pProfileParent);
private:
	~TAccountXmpp();
public:
	void Account_MarkForDeletion();
	void RemoveAllReferencesToObjectsAboutBeingDeleted();
	CChatConfiguration * PGetConfiguration() const;
	const SHashSha1 * PGetSaltOfConfiguration() const;

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;					// From IRuntimeObject
	virtual void XmlExchange(INOUT CXmlExchanger * pXmlExchanger);				// From IXmlExchange
	virtual BOOL TreeItem_FContainsMatchingText(PSZUC pszTextSearchLowercase) CONST_MCC;		// From ITreeItem
	virtual PSZUC TreeItem_PszGetNameDisplay() CONST_MCC;								// From ITreeItem
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);					// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);			// From ITreeItem
	virtual void TreeItem_GotFocus();												// From ITreeItem
	PSZUC TreeItemAccount_PszGetNameDisplay() const;
	void TreeItemAccount_DisplayWithinNavigationTree();
	void TreeItemAccount_DisplayWithinNavigationTreeInit(PSZUC pszServerName, UINT uServerPort);
	void TreeItemAccount_DeleteFromNavigationTree_MB(PA_DELETING);
	void TreeItemAccount_UpdateIcon();
	void TreeItemAccount_UpdateIconOfAllContacts();
	PSZUC TreeItemAccount_SetIconConnectingToServer_Gsb();
	void TreeItemAccount_SetIconOnline();
	void TreeItemAccount_SetIconOffline();
	void TreeItemAccount_SetIconDisconnected();
	TContact * TreeItemAccount_PContactAllocateNewToNavigationTree_NZ(PSZUC pszContactJID, PSZUC pszContactNameDisplay = NULL);
	TContact * TreeItemAccount_PContactAllocateNewToNavigationTreeInvited_NZ(PSZUC pszContactJID);
	TContact * TreeItemAccount_PContactAllocateNewTemporary(int nContact);
	void TreeItemAccount_RecommendationAdd(PA_CHILD ITreeItem * paTreeItemRecommendationAdd);
	void TreeItemAccount_RecommendationRemove(ITreeItem * pTreeItemRecommendationRemove);

	void IconUpdate_MessageArrivedFromContact(TContact * pContact);
	void IconUpdate_MessagesReadForContact(TContact * pContact);
	void IconUpdate_NetworkOnlineStateChanged();
	void PresenceUpdateFromGlobalSettings(EMenuAction eMenuAction_Presence);
	void PresenceUpdateFromContextMenu(EMenuAction eMenuAction_Presence);
	void PresenceUnsubscribe();

	void Socket_Connect();
	void Socket_ConnectUI(ISocketUI * piSocketUI, BOOL fCreateAccount = d_zNA);
	void Socket_ConnectUnsecured();
	void Socket_ReconnectIfDisconnected();
	BOOL Socket_FIsConnected() const;
	void Socket_DisconnectUI();
	void Socket_WriteXmlPresence();
	void OnTimerNetworkIdle();
	CSocketXmpp * Socket_PGetOnlyIfReadyToSendMessages() const;
	CSocketXmpp * PGetSocket_YZ() const { return m_paSocket; }

	void Certificate_RegisterCertificateChainFromSocket();
	BOOL Certificate_FRegisterCertificateChainFromSocketAndAcceptAllErrors(const QList<QSslError> & listErrorsSsl);
	BOOL Certificate_FIsCertificateAuthenticated();
	BOOL Certificate_FCompareFingerprints(const TCertificate * pCertificate);
	void Certificate_PinFingerprintSha1(const TCertificate * pCertificate);
	void Certificate_PinFingerprint(const CBin & binFingerprint) { m_binFingerprintCertificate = binFingerprint; }
	const CBin * Certificate_PGetBinaryFingerprint() const { return &m_binFingerprintCertificate; }
	TCertificate * Certificate_PGet_YZ() const;
	inline TCertificateServerName * Certificate_PGetServerName() const { return m_pCertificateServerName; }
	void NoticeListRoaming_AddNoticeCertificateNotAuthenticated();

	void SetJIDwithResource(PSZUC pszJIDwithResource);

	PSZUC ChatLog_PszGetInvitationLink(OUT_IGNORE CStr * pstrInvitationLink) const;
	PSZUC ChatLog_PszGetPathFolderDownload();
	void ChatLog_DisplayStanza(const CXmlNode * pXmlNodeMessageStanza);

	TAccountAlias * PGetAlias_NZ() CONST_MCC;

	inline int Contacts_UGetCount() const { return m_arraypaContacts.GetSize(); }
	inline TContact ** PrgpGetContactsStop(OUT TContact *** pppContactStop) const { return (TContact **)m_arraypaContacts.PrgpvGetElementsStop(OUT (void ***)pppContactStop); }
	void Contacts_Append(IOUT CArrayPtrContacts * parraypContacts) const { parraypContacts->Append(IN &m_arraypaContacts); }
	void Contacts_SetFlagAboutBeingDeleted();
	void Contacts_BroadcastAboutBeingDeleted();
	void Contact_RosterSubscribe(INOUT TContact * pContact);
	void Contact_RosterUnsubscribe(INOUT TContact * pContact);
	PSZUC Contact_RosterUpdateItem(const CXmlNode * pXmlNodeItemRoster);
	void Contacts_RosterUpdate(const CXmlNode * pXmlNodeQuery);
	void Contacts_RosterDisplayDebug(PSZAC pszName);
	void Contact_PresenceUpdate(const CXmlNode * pXmlNodeStanzaPresence);
	void Contact_AllocateNewFromGlobalInvitation();
	TContact * Contact_PFindByJID(PSZUC pszContactJID, EFindContact eFindContact);
	TContact * Contact_PFindByJIDorAllocate(PSZUC pszContactJID, PSZUC pszContactName);
	TContact * Contact_PFindByIdentifierGroupSender_YZ(const CXmlNode * pXmlNodeEvent) CONST_MCC;
	TContact * Contact_PFindByIdentifierOrCreate_YZ(const CXmlNode * pXmlNodeEvent, CHS chAttributeName, INOUT CBinXcpStanza * pbinXcpApiExtraRequest) CONST_MCC;
	void Contact_AddToNavigationTree(PA_CHILD TContact * paContact);


	static const int c_iGroupNew = -1;
	TGroup * Group_PAllocate();
	TGroup * Group_PaAllocateTemp(EGroupType eGroupType);
	void Group_AddNewMember_UI(TContact * pContact, int iGroup);
	enum EFindGroup
		{
		eFindGroupOnly,		// Return NULL if there is no group matching the identifier
		eFindGroupCreate,		// Create a new group if not present
		};
	TGroup * Group_PFindByIdentifier_YZ(PSZUC pszGroupIdentifier, INOUT CBinXcpStanza * pbinXcpApiExtraRequest, EFindGroup eFindGroup);
	TGroup * Group_PFindByIdentifier_YZ(PSZUC pszGroupIdentifier);
	TGroup * GroupChannel_PFindByNameOrCreate_YZ(PSZUC pszChannelName, INOUT CBinXcpStanza * pbinXcpApiExtraRequest);

	void Group_AddToNavigationTree(PA_CHILD TGroup * paGroup);

	void DisplayDialogProperties();
	void DebugDumpContacts();
public:
	static IXmlExchange * S_PaAllocateAccount(PVOID pvContextAllocate);	// This static method must be compatible with interface PFn_PaAllocateXmlObject()
	friend class CArrayPtrAccountsXmpp;
	friend class WLayoutAccount;
}; // TAccountXmpp


class CArrayPtrAccountsXmpp : public CArrayPtrAccountsCore
{
public:
	inline TAccountXmpp ** PrgpGetAccountsStop(OUT TAccountXmpp *** pppAccountStop) const { return (TAccountXmpp **)PrgpvGetElementsStop(OUT (void ***)pppAccountStop); }
	inline TAccountXmpp * PGetAccountFirst_YZ() const { return (TAccountXmpp *)PvGetElementFirst_YZ(); }
	TAccountXmpp * PFindAccountByJID(PSZUC pszuAccountJID) const;
	TAccountXmpp * PFindAccountByNameDisplay(PSZUC pszAccountNameDisplay) const;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	This class displays an alias to an account under the "Profile" node.
//	In summary, this class is a shortcut to an account.
class TAccountAlias : public ITreeItem	// (alias)
{
public:
	TAccountXmpp * m_pAccount;
public:
	TAccountAlias(TAccountXmpp * pAccount);

	virtual POBJECT PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const;			// From IRuntimeObject
	virtual void TreeItem_MenuAppendActions(IOUT WMenu * pMenu);			// From ITreeItem
	virtual EMenuAction TreeItem_EDoMenuAction(EMenuAction eMenuAction);	// From ITreeItem
	virtual void TreeItem_GotFocus();										// From ITreeItem

	RTI_IMPLEMENTATION(TAccountAlias)
}; // TAccountAlias


#endif // TACCOUNT_H
