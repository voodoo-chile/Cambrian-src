//	TBrowserTabs.cpp
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "TBrowserTabs.h"
#include "WLayoutTabbedBrowser.h"


TBrowserTabs::TBrowserTabs(TProfile * pProfile)
	{
	Assert(pProfile != NULL);
	Assert(pProfile->EGetRuntimeClass() == RTI(TProfile));
	m_pProfile			= pProfile;
	m_pawLayoutBrowser	= NULL;
	}

TBrowserTabs::~TBrowserTabs()
	{
	//TRACE1("TBrowserTabs::~TBrowserTabs(0x$p)", this);
	m_arraypaTabs.DeleteAllRuntimeObjects();
	MainWindow_DeleteLayout(PA_DELETING m_pawLayoutBrowser);
	}

void
TBrowserTabs::SetIconAndName(EMenuAction /*eMenuActionIcon*/, PSZAC pszName)
	{
	m_strNameDisplayTyped.BinInitFromStringWithNullTerminator(pszName);
	}

TBrowserTab*
TBrowserTabs::AddTab(CStr &sUrl)
	{
	// special case: use the first new tab to set the first non null url
	TBrowserTab *pBrowserTab = (TBrowserTab*) m_arraypaTabs.PvGetElementFirst_YZ();
	if ( m_arraypaTabs.GetSize() == 1 && !sUrl.FIsEmptyString() &&  pBrowserTab->m_url.FIsEmptyString() )
		{
		pBrowserTab->SetUrl(sUrl);
		}
	else
		{
		pBrowserTab = new TBrowserTab(this);
		pBrowserTab->m_url = sUrl;

		m_arraypaTabs.Add(pBrowserTab);
		if ( m_pawLayoutBrowser)
			m_pawLayoutBrowser->AddTab(pBrowserTab);
		}

	//if ( m_paTreeItemW_YZ != NULL)
	//	pBrowserTab->TreeItemW_DisplayWithinNavigationTree(this);

	return pBrowserTab;
	}

TBrowserTab*
TBrowserTabs::AddTab()
	{
	CStr url;
	TBrowserTab* pTab = AddTab(url);
	return pTab;
}

TBrowserTab *TBrowserTabs::PGetCurrentBrowserTab_YZ()
	{
	int nIndex = -1;

	if ( m_pawLayoutBrowser)
		nIndex = m_pawLayoutBrowser->CurrentTabIndex();

	if ( nIndex >= 0 )
		{
		WWebViewTabbed *pWebView = m_pawLayoutBrowser->getTab(nIndex);
		Assert(pWebView != NULL);
		Assert(pWebView->m_pTab != NULL);
		return pWebView->m_pTab;
		}

	return NULL;
	}

void TBrowserTabs::DeleteTab(int index)
	{
	TBrowserTab *pBrowserTab = (TBrowserTab *)m_arraypaTabs.PvGetElementAtSafe_YZ(index);
	if ( pBrowserTab != NULL)
		m_pawLayoutBrowser->RemoveTab(index);
		m_arraypaTabs.DeleteTreeItem(pBrowserTab);
	}

int
TBrowserTabs::GetTabsCount()
	{
	return m_arraypaTabs.GetSize();
	}


//	TBrowser::IRuntimeObject::PGetRuntimeInterface()
POBJECT
TBrowserTabs::PGetRuntimeInterface(const RTI_ENUM rti, IRuntimeObject * piParent) const
	{
	Report(piParent == NULL);
	return ITreeItem::PGetRuntimeInterface(rti, m_pProfile);
	}

//	TBrowser::IXmlExchange::XmlExchange()
void
TBrowserTabs::XmlExchange(INOUT CXmlExchanger * pXmlExchanger)
	{
	ITreeItem::XmlExchange(pXmlExchanger);
	pXmlExchanger->XmlExchangeObjects('T', INOUT_F_UNCH_S &m_arraypaTabs, TBrowserTab::S_PaAllocateBrowser, this);
	}

IXmlExchange *
TBrowserTabs::S_PaAllocateBrowserTabbed(PVOID pProfileParent)
	{
	Assert(pProfileParent != NULL);
	return new TBrowserTabs((TProfile*) pProfileParent);
	}

const EMenuActionByte c_rgzeActionsMenuBrowser[] =
	{
	eMenuAction_TreeItemRename,
	eMenuAction_Close,
	ezMenuActionNone
	};

//	TBrowser::ITreeItem::TreeItem_MenuAppendActions()
void
TBrowserTabs::TreeItem_MenuAppendActions(IOUT WMenu * pMenu)
	{
	pMenu->ActionsAdd(c_rgzeActionsMenuBrowser);
	}

//	TBrowser::ITreeItem::TreeItem_EDoMenuAction()
EMenuAction
TBrowserTabs::TreeItem_EDoMenuAction(EMenuAction eMenuAction)
	{
	switch (eMenuAction)
		{
	case eMenuAction_Close:
		MainWindow_SetCurrentLayout(NULL);
		m_pProfile->m_arraypaBrowsersTabbed.DeleteTreeItem(PA_DELETING this);
		return ezMenuActionNone;
	default:
		return ITreeItem::TreeItem_EDoMenuAction(eMenuAction);
		} // switch
	} // TreeItem_EDoMenuAction()

void
TBrowserTabs::TreeItem_GotFocus()
	{
	//MessageLog_AppendTextFormatCo(d_coChocolate, "TBrowserTabs::TreeItem_GotFocus()");
	if ( m_pawLayoutBrowser == NULL)
		{
		m_pawLayoutBrowser = new WLayoutTabbedBrowser(this, m_pProfile);

		// add tabs added before the layout was initialized
		TBrowserTab **ppBrowserTabStop;
		TBrowserTab **ppBrowserTab = m_arraypaTabs.PrgpGetBrowserTabStop(&ppBrowserTabStop);
		while(ppBrowserTab != ppBrowserTabStop)
			{
			TBrowserTab *pBrowserTab = *ppBrowserTab++;
			m_pawLayoutBrowser->AddTab(pBrowserTab);
			CStr text("Tab");
			pBrowserTab->m_strNameDisplayTyped = text;
			//pBrowserTab->TreeItemW_DisplayWithinNavigationTree(this);
			}

		// add default tab if there was none
		if ( m_arraypaTabs.GetSize() == 0)
			AddTab();
		}


	MainWindow_SetCurrentLayout(IN m_pawLayoutBrowser);
	}

void
TBrowserTabs::TreeItemBrowser_DisplayWithinNavigationTree()
	{
	//MessageLog_AppendTextFormatCo(d_coChocolate, "TBrowserTabs::TreeItemBrowser_DisplayWithinNavigationTree()");
	TreeItemW_DisplayWithinNavigationTree((m_pProfile->m_paTreeItemW_YZ != NULL) ?  m_pProfile : NULL);
	TreeItemW_SetIcon(eMenuAction_DisplaySecureWebBrowsing);
	}



