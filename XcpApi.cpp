///////////////////////////////////////////////////////////////////////////////////////////////////
//	XcpApi.cpp
//
//	Implementation of "Personal APIs" (PAPI) available through the Cambrian Protocol.
//
//	The content of this file could be within Xcp.cpp, however since file Xcp.cpp already has 1500 lines,
//	it is easier to understand the code related to the Cambrian APIs in a dedicated file.
//
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "XcpApi.h"

#if 0
	Group.Profile.Get [idGroup]
	Group.Profile.GetPicture [idGroup]

	Contact.Profile.Get [idContact]
	Contact.Profile.GetPicture [idContact]
	Contact.Profile.GetPicturePanoramic [idContact]
	Contact.Recommendations.Get							- Return all the recommendations from the contact (other contacts, groups, links, etc)
	Contact.Wallet.GetAddress [idContact], strCurrency, strInvoiceNumber		// Return an address for payment

	Me.Profile.Get
	Me.Wallet.GetAddress
#endif

const CHU c_szaApi_Group_Profile_Get[] = "Group.Profile.Get";

#define d_chAPIe_TGroup_					'G'
#define d_szAPIe_TGroup_					"G"
#define d_szAPIe_TGroup_h_str				"G i='{h|}' n='^S'"
	#define d_chAPIa_TGroup_shaIdentifier	'i'
	#define d_chAPIa_TGroup_strName			'n'
#define d_chAPIe_TGroupMember_				'M'
#define d_szAPIe_TGroupMember_				"M"
	#define d_chAPIa_TGroupMember_idContact	'c'	// Perhaps rename to 'i'

const CHU c_szaApi_Contact_Recommendations_Get[] = "Contact.Recommendations.Get";

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Core method to serialize the data for an XCP API request.
void
CBinXcpStanzaType::BinXmlAppendXcpElementForApiRequest_AppendApiParameterData(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
	{
	Assert(pszApiName != NULL);
	Assert(pXmlNodeApiParameters != NULL);
	Assert(XcpApi_FIsXmlElementOpened());
	Assert(m_pContact != NULL);
	TAccountXmpp * pAccount = m_pContact->m_pAccount;
	Assert(pAccount->EGetRuntimeClass() == RTI(TAccountXmpp));
	MessageLog_AppendTextFormatSev(eSeverityComment, "Processing API '$s' with the following parameters: ^N\n", pszApiName, pXmlNodeApiParameters);

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
	} // BinXmlAppendXcpElementForApiRequest_AppendApiParameterData()


//	Method to unserialize the data from BinXmlAppendXcpElementForApiRequest_AppendApiParameterData().
//	Although the reply typically does not need the object CBinXcpStanzaType, the reply may necessitate the CBinXcpStanzaType to make additional request.
//	For instance, when unserializing a group, the method may need to query information about new group members to fetch the contact profile.
void
CBinXcpStanzaType::BinXmlAppendXcpElementForApiReply(PSZUC pszApiName, const CXmlNode * pXmlNodeApiParameters)
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
	MessageLog_AppendTextFormatSev(eSeverityErrorWarning, "Unknown API: $s\n^N", pszApiName, pXmlNodeApiParameters);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
CBinXcpStanzaType::BinXmlAppendXcpApiRequest_ProfileGet(PSZUC pszGroupIdentifier)
	{
	BinXmlAppendXcpApiRequest((PSZAC)c_szaApi_Group_Profile_Get, pszGroupIdentifier);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
ITreeItemChatLogEvents::XcpApi_Invoke_RecommendationsGet()
	{
	XcpApi_Invoke(c_szaApi_Contact_Recommendations_Get, d_zNA, d_zNA);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
//	Serialize all recommendations related to a profile

#define d_chAPIe_Recommendations_					'R'
#define d_szAPIe_Recommendations_					"R"
#define d_chAPIe_Recommendations_TContacts			'C'
#define d_szAPIe_Recommendations_TContacts			"C"
#define d_chAPIe_Recommendations_TGroups			'G'
#define d_szAPIe_Recommendations_TGroups			"G"
	#define d_chAPIa_Recommendations_shaIdentifier	'i'
	#define d_chAPIa_Recommendations_strName		'n'
	#define d_chAPIa_Recommendations_strDescription	'd'	// NYI
	#define d_szAPIe_RecommendationContact_p_str	"C i='^i' n='^S'"
	#define d_szAPIe_RecommendationGroup_h_str		"G i='{h|}' n='^S'"

void
TProfile::XcpApiProfile_RecommendationsSerialize(INOUT CBinXcpStanzaType * pbinXcpStanzaReply) const
	{
	Assert(pbinXcpStanzaReply != NULL);
	pbinXcpStanzaReply->BinAppendText("<" d_szAPIe_Recommendations_ "><" d_szAPIe_Recommendations_TContacts ">");
	CArrayPtrContacts arraypaContactsRecommended;
	GetRecommendations_Contacts(OUT &arraypaContactsRecommended);
	TContact ** ppContactStop;
	TContact ** ppContact = arraypaContactsRecommended.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_RecommendationContact_p_str "/>", pContact, &pContact->m_strNameDisplayTyped);	// Need to fix this for a real name of the contact (not what was typed for the Navigation Tree)
		}
	pbinXcpStanzaReply->BinAppendText("</" d_szAPIe_Recommendations_TContacts "><" d_szAPIe_Recommendations_TGroups ">");
	CArrayPtrGroups arraypaGroupsRecommended;
	GetRecommendations_Groups(OUT &arraypaGroupsRecommended);
	TGroup ** ppGroupStop;
	TGroup ** ppGroup = arraypaGroupsRecommended.PrgpGetGroupsStop(OUT &ppGroupStop);
	while (ppGroup != ppGroupStop)
		{
		TGroup * pGroup = *ppGroup++;
		Assert(pGroup != NULL);
		Assert(pGroup->EGetRuntimeClass() == RTI(TGroup));
		pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_RecommendationGroup_h_str "/>", &pGroup->m_hashGroupIdentifier, &pGroup->m_strNameDisplayTyped);
		}
	pbinXcpStanzaReply->BinAppendText( "</" d_szAPIe_Recommendations_TGroups "></" d_szAPIe_Recommendations_ ">");
	} // XcpApiProfile_RecommendationsSerialize()

void
TContact::XcpApiContact_ProfileSerialize(INOUT CBinXcpStanzaType * pbinXcpStanzaReply) const
	{

	}

//	Serialize a group for the XCP API.
void
TGroup::XcpApiGroup_ProfileSerialize(INOUT CBinXcpStanzaType * pbinXcpStanzaReply) const
	{
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_TGroup_h_str ">", &m_hashGroupIdentifier, (m_uFlagsTreeItem & FTI_kfTreeItem_NameDisplayedGenerated) ? NULL : &m_strNameDisplayTyped);
	TGroupMember ** ppMemberStop;
	TGroupMember ** ppMember = m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
	while (ppMember != ppMemberStop)
		{
		TGroupMember * pMember = *ppMember++;
		Assert(pMember != NULL);
		Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
		if (pMember->m_pContact != pbinXcpStanzaReply->m_pContact)
			{
			pbinXcpStanzaReply->BinAppendText("<" d_szAPIe_TGroupMember_);
			pbinXcpStanzaReply->BinAppendXmlAttributeOfContactIdentifier(d_chAPIa_TGroupMember_idContact, pMember->m_pContact);	// {ic}, {is}	^i
			pbinXcpStanzaReply->BinAppendText("/>");
			}
		}
	pbinXcpStanzaReply->BinAppendTextSzv_VE("<" d_szAPIe_TGroupMember_ " c='^S'" "/>", &m_pAccount->m_strJID);	// Include self in the list
	pbinXcpStanzaReply->BinAppendText("</" d_szAPIe_TGroup_ ">");
	}

void
TGroup::XcpApiGroup_ProfileUnserialize(const CXmlNode * pXmlNodeApiParameters, INOUT CBinXcpStanzaType * pbinXcpApiExtraRequest)
	{
	// Fill in any missing group info from the data in pXmlNodeApiParameters
	if (m_uFlagsTreeItem & FTI_kfTreeItem_NameDisplayedGenerated)
		{
		PSZUC pszName = pXmlNodeApiParameters->PszuFindAttributeValue_NZ(d_chAPIa_TGroup_strName);
		if (pszName[0] != '\0')
			{
			m_strNameDisplayTyped = pszName;	// Assign the group name
			m_uFlagsTreeItem &= ~FTI_kfTreeItem_NameDisplayedGenerated;
			}
		}
	const CXmlNode * pXmlNodeMembers = pXmlNodeApiParameters->PFindElement(d_chAPIe_TGroupMember_);
	while (pXmlNodeMembers != NULL)
		{
		Member_PFindOrAllocate_NZ(pXmlNodeMembers->PszuFindAttributeValue(d_chAPIa_TGroupMember_idContact));
		//TContact * pContact = m_pAccount->Contact_PFindByIdentifierOrCreate_YZ(pXmlNodeMembers, d_chAPIe_GroupMember_idContact, INOUT pbinXcpApiExtraRequest);
		//MessageLog_AppendTextFormatCo(d_coRed, "XcpApiGroup_ProfileUnserialize() - Adding group member ^j\n", pContact);
		pXmlNodeMembers = pXmlNodeMembers->m_pNextSibling;
		}
	TreeItemChatLog_UpdateTextAndIcon();	// Need to optimize this
	} // XcpApiGroup_ProfileUnserialize()

#if 0
	/*
	A suggestion is an idea or plan by someone that is  put forward for certain considerations. While
	A recommendation  is an opinion which is given by someone who did an analysis of something presented to his/her clients and is considering whether to buy it out or not.
	Usually done by more knowlegeable people in that field being tackled. Anyone can make a suggestion.  But to make recommendations someone has to be more expert on that field.
	*/

	// Sample recommendations from a contact
	<R>
		<C i='contact identifier' n='name of the contact' d='description of the contact, or a comment why the contact is recommded' />	// Recommend a contact
		<G i='group identifier' n='' d='' />	// Recommend a group
	</R>

#endif
