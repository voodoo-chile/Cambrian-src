//	DDialogProperties.cpp
//
//	Core classes to create "property pages" as well as the implementation of a few of them.

#include "DDialogProperties.h"

DDialogPropertyPage::DDialogPropertyPage(PSZAC pszName) : QListWidgetItem(pszName), WWidget(NULL)
	{
	//new QLabel(pszName, this);
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
#define SL_DDialogProperties(pfmSlot)		SL_DDialog(pfmSlot, DDialogProperties)

DDialogProperties::DDialogProperties()
	{
	m_pwPagesList = new QListWidget(this);
	m_pwPagesStacked = new QStackedWidget(this);
	OLayoutHorizontal * poLayoutHorizontal = new OLayoutHorizontal(this);
	poLayoutHorizontal->addWidget(m_pwPagesList);
	OLayoutVertical * poLayoutVertical = new OLayoutVertical(poLayoutHorizontal);
	poLayoutVertical->addWidget(m_pwPagesStacked);
	connect(m_pwPagesList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(SL_PageChanged(QListWidgetItem*,QListWidgetItem*)));

	Dialog_CreateButtonsOkCancel(SL_DDialogProperties(SL_ButtonOK_clicked));
	m_poLayoutButtons = new OLayoutHorizontal(poLayoutVertical);
	m_poLayoutButtons->setAlignment(Qt::AlignRight);
	m_poLayoutButtons->addWidget(m_pwButtonOK);
	m_poLayoutButtons->addWidget(m_pwButtonCancel);
	}

void
DDialogProperties::PageAdd(DDialogPropertyPage * pawPage)
	{
	m_pwPagesList->addItem(pawPage); // pawPage->windowTitle());
	m_pwPagesStacked->addWidget(pawPage);
	}

//	DDialogProperties::QDialog::exec()
int
DDialogProperties::exec()
	{
	if (m_pwPagesList->currentItem() == NULL)
		m_pwPagesList->setCurrentRow(0);	// If no page is selected, select the first one
	return QDialog::exec();
	}

void
DDialogProperties::SL_PageChanged(QListWidgetItem * pPageCurrent, QListWidgetItem * pPagePrevious)
	{
	Endorse(pPagePrevious == NULL);
	DDialogPropertyPage * pwPage = (DDialogPropertyPage *)pPageCurrent;
	m_pwPagesStacked->setCurrentWidget(pwPage);
	}

void
DDialogProperties::SL_ButtonOK_clicked()
	{
	EMessageBoxInformation("OK");
	}

void
DDialogProperties::SL_ButtonCancel_clicked()
	{

	}


///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogPropertiyPageAccountGeneral::DDialogPropertiyPageAccountGeneral(TAccountXmpp * pAccount) : DDialogPropertyPage("General")
	{
	m_pAccount = pAccount;
	/*
	OLayoutVerticalAlignTop * poLayoutVertical = new OLayoutVerticalAlignTop(this);
	OLayoutHorizontal * poLayout = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	//poLayout->setSizeConstraint(QLayout::SetMaximumSize);
	poLayout->Layout_AddLabelAndWidgetH_PA("Username", new WEditUsername);
	poLayout->Layout_AddLabelAndWidgetH_PA("Password", new WEditPassword);
	poLayout->addWidget(PA_CHILD new QWidget, 1);
	*/
	OLayoutForm * pLayout = new OLayoutForm(this);
	pLayout->Layout_PwAddRowLabelEditReadOnly("Username:", pAccount->m_strUsername);
	m_pwEditPassword = pLayout->Layout_PwAddRowLabelEditPassword("Password:");
	pLayout->Layout_PwAddRowLabelEditReadOnly("Server:", pAccount->m_strServerName);
	pLayout->Layout_PwAddRowLabelEditReadOnlyInt("Port:", pAccount->m_uServerPort);
	pLayout->Layout_PwAddRowLabelEditReadOnly("Resource:", pAccount->m_strJIDwithResource.PchFindCharacter('/'));
	pLayout->Layout_PwAddRowLabelEditReadOnlyFingerprint("Certificate Fingerprint:", *pAccount->Certificate_PGetBinaryFingerprint());
	setMinimumWidth(500);
	}

DDialogPropertiyPageAccountTest::DDialogPropertiyPageAccountTest(PSZAC pszName) : DDialogPropertyPage(pszName)
	{
	new QLabel(pszName, this);
	}

DDialogPropertiesAccount::DDialogPropertiesAccount(TAccountXmpp * pAccount)
	{
	m_pAccount = pAccount;
	Dialog_SetCaptionFormat_VE("Account - ^j", pAccount);
	PageAdd(new DDialogPropertiyPageAccountGeneral(pAccount));
	PageAdd(new DDialogPropertiyPageAccountTest("RootID"));
	PageAdd(new DDialogPropertiyPageAccountTest("PGP"));
	PageAdd(new DDialogPropertiyPageAccountTest("Bitcoin"));
	PageAdd(new DDialogPropertiyPageAccountTest("Notes"));
	m_pwPagesList->setFixedWidth(100);
	}


void
TAccountXmpp::DisplayDialogProperties()
	{
	DDialogPropertiesAccount dialog(this);
	dialog.FuExec();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogPropertiyPageContactGeneral::DDialogPropertiyPageContactGeneral(TContact * pContact) : DDialogPropertyPage("General")
	{
	m_pContact = pContact;
	OLayoutVerticalAlignTop * poLayoutVertical = new OLayoutVerticalAlignTop(this);
	OLayoutHorizontal * poLayout = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	poLayout->Layout_AddLabelAndWidgetH_PA("Name", new WEditUsername);
	poLayout->addWidget(PA_CHILD new QWidget, 1);
	}

DDialogPropertiesContact::DDialogPropertiesContact(TContact * pContact)
	{
	m_pContact = pContact;
	Dialog_SetCaptionFormat_VE("Contact - ^j", pContact);
	PageAdd(new DDialogPropertiyPageContactGeneral(pContact));
	PageAdd(new DDialogPropertiyPageAccountTest("RootID"));
	PageAdd(new DDialogPropertiyPageAccountTest("PGP"));
	PageAdd(new DDialogPropertiyPageAccountTest("Bitcoin"));
	PageAdd(new DDialogPropertiyPageAccountTest("Notes"));
	m_pwPagesList->setFixedWidth(100);
	}

void
TContact::DisplayDialogProperties()
	{
	DDialogPropertiesContact dialog(this);
	dialog.FuExec();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
TGroup::DisplayDialogProperties()
	{
	DDialogPropertiesGroup dialog(this);
	dialog.FuExec();
	}

DDialogPropertiyPageGroupGeneral::DDialogPropertiyPageGroupGeneral(TGroup * pGroup) : DDialogPropertyPage("General")
	{
	m_pGroup = pGroup;
	OLayoutVerticalAlignTop * poLayoutVertical = new OLayoutVerticalAlignTop(this);
	OLayoutHorizontal * poLayout = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	poLayout->Layout_AddLabelAndWidgetH_PA("Group ID", new WEditReadOnly(g_strScratchBufferStatusBar.Format("{h|}", &pGroup->m_hashGroupIdentifier)));
	poLayout = new OLayoutHorizontalAlignLeft(poLayoutVertical);
	poLayout->Layout_AddLabelAndWidgetH_PA("File Name", new WEditReadOnly(QDir::toNativeSeparators(pGroup->Vault_SGetPath())));
	//poLayout->addWidget(PA_CHILD new QWidget, 1);
	}

DDialogPropertiesGroup::DDialogPropertiesGroup(TGroup * pGroup)
	{
	m_pGroup = pGroup;
	Dialog_SetCaptionFormat_VE("Group - $s", pGroup->TreeItem_PszGetNameDisplay());
	PageAdd(new DDialogPropertiyPageGroupGeneral(pGroup));
	m_pwPagesList->setFixedWidth(100);
	setMinimumWidth(600);
	}


///////////////////////////////////////////////////////////////////////////////////////////////////
//	Very similar as CTreeWidgetItem
class CListWidgetTreeItem :  public QListWidgetItem
{
public:
	ITreeItem * m_piTreeItem;

public:
	CListWidgetTreeItem(WListContacts * pwParent, ITreeItem * piTreeItem);
};

CListWidgetTreeItem::CListWidgetTreeItem(WListContacts * pwParent, ITreeItem * piTreeItem) : QListWidgetItem(pwParent->m_oIconContact, CString(piTreeItem->TreeItem_PszGetNameDisplay()), pwParent)
	{
	m_piTreeItem = piTreeItem;
	}


WListContacts::WListContacts()
	{
	m_oIconContact = PGetMenuAction(eMenuAction_Contact)->icon();	// Create a cached copy of the icon for better performance
	}

void
WListContacts::ContactAdd(TContact * pContact)
	{
	Assert(pContact->EGetRuntimeClass() == RTI(TContact));
	(void)new CListWidgetTreeItem(this, pContact);
	}

void
WListContacts::ContactsAdd(IN_MOD_SORT CArrayPtrContacts & arraypContacts)
	{
	arraypContacts.SortByNameDisplay();
	TContact ** ppContactStop;
	TContact ** ppContact = arraypContacts.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		ContactAdd(pContact);
		}
	}

void
WListContacts::ContactAddTransfer(PA_DELETING QListWidgetItem * paItem)
	{
	ContactAdd((TContact *)((CListWidgetTreeItem *)paItem)->m_piTreeItem);
	delete paItem;	// Deleting the QListWidgetItem will automatically remove it from its parent WListContacts
	}
/*
void
WListContacts::ContactRemove(TContact * pContact)
	{
	// Find the contact in the list and delete it
	Assert(FALSE && "NYI");
	}
*/

//	Get all the contacts in the list
void
WListContacts::ContactsGetAll(IOUT CArrayPtrContacts * parraypContacts) const
	{
	Assert(parraypContacts != NULL);
	int cRows = count();
	for (int iRow = 0; iRow < cRows; iRow++)
		{
		CListWidgetTreeItem * pItem = (CListWidgetTreeItem *)item(iRow);
		Assert(pItem->m_piTreeItem->EGetRuntimeClass() == RTI(TContact));
		parraypContacts->Add(pItem->m_piTreeItem);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
DDialogGroupAddContacts::DDialogGroupAddContacts(ITreeItemChatLogEvents * pContactOrGroup) : DDialogOkCancelWithLayouts("Add Contacts to Group", eMenuAction_GroupAddContacts)
	{
	m_pwListContactsAvailable = new WListContacts;
	m_pwListContactsInGroup = new WListContacts;

	m_pContactOrGroup = pContactOrGroup;
	m_arraypContactsAvailable.Copy(IN &pContactOrGroup->m_pAccount->m_arraypaContacts);
	if (pContactOrGroup->EGetRuntimeClass() == RTI(TGroup))
		{
		// We are updating an existing group
		m_pGroup = (TGroup *)pContactOrGroup;
		m_pGroup->Members_GetContacts(IOUT &m_arraypContactsInGroup);
		}
	else
		{
		// We are creating a new group
		Assert(pContactOrGroup->EGetRuntimeClass() == RTI(TContact));
		m_arraypContactsInGroup.Add((TContact *)pContactOrGroup);
		//m_pwListContactsInGroup->ContactAdd((TContact *)pContactOrGroup);
		m_pGroup = NULL;
		}

	m_poLayoutBody->Layout_PwAddRowLabel("Double-click on the contacts you wish to add or remove to the group:");
	OLayoutHorizontal * poLayoutHorizontal = new OLayoutHorizontal(m_poLayoutBody);
	OLayoutVerticalAlignTop * poLayoutVertical = new OLayoutVerticalAlignTop(poLayoutHorizontal);
	poLayoutVertical->Layout_PwAddRowLabel("Contacts not in group");
	poLayoutVertical->addWidget(PA_CHILD m_pwListContactsAvailable);
//	poLayoutVertical->addWidget(new WButtonIcon(eMenuAction_ContactAdd, "Include the selected contact(s) to the group"), d_zNA, Qt::AlignRight);
	poLayoutVertical = new OLayoutVerticalAlignTop(poLayoutHorizontal);
	poLayoutVertical->Layout_PwAddRowLabel("Contacts in group");
	poLayoutVertical->addWidget(PA_CHILD m_pwListContactsInGroup);
//	poLayoutVertical->addWidget(new WButtonIcon(eMenuAction_ContactRemove, "Remove selected contact(s) from the group"));
	poLayoutHorizontal = new OLayoutHorizontal(m_poLayoutBody);
	WButtonText * pwButtonOK = new WButtonTextWithIcon((m_pGroup != NULL) ? " Add Contacts " : " Create Group ", eMenuAction_Group);
	poLayoutHorizontal->addWidget(pwButtonOK, d_zNA, Qt::AlignRight);

	// Populate the lists of contacts
	m_arraypContactsAvailable.RemoveTreeItems(IN m_arraypContactsInGroup);	// Make sure m_arraypContactsAvailable and m_arraypContactsInGroup are mutually exclusive
	m_pwListContactsAvailable->ContactsAdd(IN_MOD_SORT m_arraypContactsAvailable);
	m_pwListContactsInGroup->ContactsAdd(IN_MOD_SORT m_arraypContactsInGroup);
	connect(m_pwListContactsAvailable, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(SL_ContactAvailableDoubleClicked(QListWidgetItem*)));
	connect(m_pwListContactsInGroup, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(SL_ContactInGroupDoubleClicked(QListWidgetItem*)));
	connect(pwButtonOK, SIGNAL(clicked()), this, SLOT(SL_ButtonOK()));
	}

DDialogGroupAddContacts::~DDialogGroupAddContacts()
	{
	}

// Update group by adding the new selected contacts and removing the contacts not in the list
void
DDialogGroupAddContacts::SL_ButtonOK()
	{
	CArrayPtrContacts arraypContactsInGroup;
	m_pwListContactsInGroup->ContactsGetAll(IOUT &arraypContactsInGroup);

	if (m_pGroup == NULL)
		{
		// We are creating a new group, therefore display it to the Navigation Tree before adding/removing members
		m_pGroup = new TGroup(m_pContactOrGroup->m_pAccount);
		m_pGroup->m_pAccount->m_arraypaGroups.Add(PA_CHILD m_pGroup);
		m_pGroup->GroupInitNewIdentifier();
		m_pGroup->TreeItemGroup_DisplayWithinNavigationTree();
		}
	else
		{
		// Update the existing group by removing any group member (contact) not in the list
		TGroupMember ** ppMemberStop;
		TGroupMember ** ppMember = m_pGroup->m_arraypaMembers.PrgpGetMembersStop(OUT &ppMemberStop);
		while (ppMember != ppMemberStop)
			{
			TGroupMember * pMember = *ppMember++;
			Assert(pMember != NULL);
			Assert(pMember->EGetRuntimeClass() == RTI(TGroupMember));
			Assert(pMember->m_pContact->EGetRuntimeClass() == RTI(TContact));
			if (!arraypContactsInGroup.FindElementF(pMember->m_pContact))
				m_pGroup->Member_Remove_UI(pMember);
			}
		arraypContactsInGroup.RemoveTreeItems(IN m_arraypContactsInGroup);	// Remove the existing contacts and we have a list of the new contacts to add
		}

	// Add the new group members (contacts)
	TContact ** ppContactStop;
	TContact ** ppContact = arraypContactsInGroup.PrgpGetContactsStop(OUT &ppContactStop);
	while (ppContact != ppContactStop)
		{
		TContact * pContact = *ppContact++;
		m_pGroup->Member_Add_UI(pContact);
		}
	SL_ButtonOK_clicked();
	} // SL_ButtonOK()

void
DDialogGroupAddContacts::SL_ContactAvailableDoubleClicked(QListWidgetItem * paItem)
	{
	m_pwListContactsInGroup->ContactAddTransfer(PA_DELETING paItem);
	}

void
DDialogGroupAddContacts::SL_ContactInGroupDoubleClicked(QListWidgetItem * paItem)
	{
	m_pwListContactsAvailable->ContactAddTransfer(PA_DELETING paItem);
	}

void
ITreeItemChatLogEvents::DisplayDialogAddContactsToGroup()
	{
	DDialogGroupAddContacts dialog(this);
	dialog.FuExec();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
ITreeItemChatLogEvents::DisplayDialogBallotSend(CEventBallotSent * pEventBallotInit)
	{
	DDialogBallotSend dialog(this, pEventBallotInit);
	dialog.FuExec();
	}

#define SL_DDialogBallotSend(pfmSlot)		SL_DDialog(pfmSlot, DDialogBallotSend)
DDialogBallotSend::DDialogBallotSend(ITreeItemChatLogEvents * pContactOrGroup, CEventBallotSent * pEventBallotInit) : DDialogOkCancelWithLayouts("Send Ballot", eMenuAction_BallotSend)
	{
	Assert(pContactOrGroup != NULL);
	m_pContactOrGroup = pContactOrGroup;
	CEventBallotSent * paEventBallotInit = NULL;
	if (pEventBallotInit == NULL)
		paEventBallotInit = pEventBallotInit = new CEventBallotSent(NULL);	// Create an empty ballot event to initialize the dialog

	Dialog_AddButtonsOkCancel_RenameButtonOk(SL_DDialogBallotSend(SL_ButtonOK_clicked), "Send Ballot |Broadcast the ballot the group", eMenuAction_BallotSend);
	m_pwButtonOK->setDefault(false);

	OLayoutForm * pLayout = new OLayoutForm(m_poLayoutBody);
	m_pwEditTitle = pLayout->Layout_PwAddRowLabelEdit("Title:", pEventBallotInit->m_strTitle);
	m_pwEditDescription = pLayout->Layout_PwAddRowLabelEditTextAreaH("Description:", pEventBallotInit->m_strDescription, 3);

	m_pLayoutQuestions = new OLayoutVertical(m_poLayoutBody);
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = pEventBallotInit->m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	if (ppChoice == ppChoiceStop)
		SL_ButtonAdd();		// Add an empty question
	else
		{
		while (ppChoice != ppChoiceStop)
			{
			_CEventBallotChoice * pChoice = *ppChoice++;
			_OLayoutBallotChoice * paQuestion = new _OLayoutBallotChoice(this);
			paQuestion->m_pwEditChoice->Edit_SetText(pChoice->m_strQuestion);
			}
		}

	WButtonTextWithIcon * pwButtonAdd = new WButtonTextWithIcon("&New Question |Add a new question to the ballot", eMenuIconAdd);
	m_poLayoutBody->addWidget(pwButtonAdd);
	connect(pwButtonAdd, SIGNAL(clicked()), this, SLOT(SL_ButtonAdd()));
	delete paEventBallotInit;
	m_pwEditTitle->setFocus();
	}

DDialogBallotSend::~DDialogBallotSend()
	{
	}

void
DDialogBallotSend::SL_ButtonOK_clicked()
	{
	// Always create a new ballot when the user click on OK
	CEventBallotSent * pEventBallot = new CEventBallotSent(NULL);
	pEventBallot->m_strTitle = m_pwEditTitle;
	pEventBallot->m_strDescription = m_pwEditDescription;

	CStr strQuestion;
	// Add the ballot questions to the event
	_OLayoutBallotChoice ** ppLayoutStop;
	_OLayoutBallotChoice ** ppLayout = m_arraypLayoutChoices.PrgpGetQuestionsStop(OUT &ppLayoutStop);
	while (ppLayout != ppLayoutStop)
		{
		_OLayoutBallotChoice * pLayout = *ppLayout++;
		strQuestion = pLayout->m_pwEditChoice;
		if (strQuestion.FIsEmptyString())
			continue;	// Don't include empty questions
		_CEventBallotChoice * pChoice = pEventBallot->PAllocateNewChoice();
		pChoice->m_strQuestion = strQuestion;
		}
	m_pContactOrGroup->Vault_InitEventForVaultAndDisplayToChatLog(PA_CHILD pEventBallot);
	DDialogOkCancelWithLayouts::SL_ButtonOK_clicked();
	}

void
DDialogBallotSend::SL_ButtonAdd()
	{
	_OLayoutBallotChoice * paQuestion = new _OLayoutBallotChoice(this);
	paQuestion->m_pwEditChoice->setFocus();
	}

void
DDialogBallotSend::SL_ButtonRemove()
	{
	WButtonIcon * pwButtonRemove = (WButtonIcon *)sender();	// Which button the user clicked
	_OLayoutBallotChoice ** ppLayoutStop;
	_OLayoutBallotChoice ** ppLayout = m_arraypLayoutChoices.PrgpGetQuestionsStop(OUT &ppLayoutStop);
	while (ppLayout != ppLayoutStop)
		{
		_OLayoutBallotChoice * pLayout = *ppLayout++;
		if (pwButtonRemove == pLayout->m_pwButtonRemove)
			{
			m_arraypLayoutChoices.RemoveElementI(pLayout);
			delete pLayout; // Delete the question
			return;
			}
		}
	Assert(FALSE && "Unreachable code");
	}

_OLayoutBallotChoice::_OLayoutBallotChoice(PA_PARENT DDialogBallotSend * pwDialogBallotParent) : OLayoutHorizontal(pwDialogBallotParent->m_pLayoutQuestions)
	{
	_OLayoutBallotChoice * poLayoutLast = (_OLayoutBallotChoice *)pwDialogBallotParent->m_arraypLayoutChoices.PvGetElementLast_YZ();
	pwDialogBallotParent->m_arraypLayoutChoices.Add(this);
	m_pwButtonRemove = new WButtonIcon(eMenuIconRemove, "Remove this question from the ballot");
	addWidget(m_pwButtonRemove);
	m_pwEditChoice = new WEdit;
	connect(m_pwEditChoice, SIGNAL(returnPressed()), pwDialogBallotParent, SLOT(SL_ButtonAdd()));
	connect(m_pwButtonRemove, SIGNAL(clicked()), pwDialogBallotParent, SLOT(SL_ButtonRemove()));
	Layout_AddLabelAndWidgetH_PA("Ballot Question:", m_pwEditChoice);

	QWidget::setTabOrder((poLayoutLast != NULL) ? poLayoutLast->m_pwEditChoice : (QWidget *)pwDialogBallotParent->m_pwEditDescription, m_pwEditChoice);	// Make sure the new edit follows a smooth smooth continuity with others
	}

_OLayoutBallotChoice::~_OLayoutBallotChoice()
	{
	// This destructor is a workaround of a Qt bug where a deleted layout remains displayed on the screen.  Hopefully this Qt bug will be fixed and this code won't be needed
	int cItems = count();
	while (--cItems >= 0)
		{
		QWidget * pwWidget = itemAt(cItems)->widget();
		if (pwWidget != NULL)
			pwWidget->hide();	// Hide every widget in the layout
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
CEventBallotReceived::DisplayDialogBallotVote()
	{
	DDialogBallotVote dialog(this);
	dialog.FuExec();
	}

#define SL_DDialogBallotVote(pfmSlot)		SL_DDialog(pfmSlot, DDialogBallotVote)
DDialogBallotVote::DDialogBallotVote(CEventBallotReceived * pEventBallotVote) : DDialogOkCancelWithLayouts("Vote", eMenuAction_BallotSend)
	{
	m_pEventBallotVote = pEventBallotVote;
	Dialog_AddButtonsOkCancel_RenameButtonOk(SL_DDialogBallotVote(SL_ButtonVote), "Vote |Cast a vote on this ballot", eMenuAction_BallotSend);

	Dialog_SetCaption(pEventBallotVote->m_strTitle);
	m_poLayoutBody->addWidget(new WLabelSelectableWrap(pEventBallotVote->m_strDescription));
	//m_poLayoutBody->addWidget(new WLabel("Please select one option:"));

	WGroupBox * pwGroupBox = new WGroupBox(m_poLayoutBody);
	pwGroupBox->Widget_SetTitleFormat_VE_Gsb("Please select one option:");
	m_poLayoutBody->addWidget(pwGroupBox);

	UINT_BALLOT_CHOICES ukfChoiceMask = 0x00000001;	// Initialize the first bit
	OLayoutVertical * pLayoutOptions = new OLayoutVertical(pwGroupBox);
	_CEventBallotChoice ** ppChoiceStop;
	_CEventBallotChoice ** ppChoice = pEventBallotVote->m_arraypaChoices.PrgpGetChoicesStop(OUT &ppChoiceStop);
	while (ppChoice != ppChoiceStop)
		{
		_CEventBallotChoice * pChoice = *ppChoice++;
		WButtonRadio * pwButtonRadio = new WButtonRadio;
		m_arraypwButtonOptions.Add(pwButtonRadio);
		pwButtonRadio->setText(pChoice->m_strQuestion);
		pLayoutOptions->addWidget(pwButtonRadio);
		//pLayoutOptions->addWidget(new WLabel(pChoice->m_strQuestion));
		pwButtonRadio->setChecked((pEventBallotVote->m_ukmChoices & ukfChoiceMask) != 0);
		ukfChoiceMask <<= 1;
		}
	}

void
DDialogBallotVote::SL_ButtonVote()
	{
	UINT_BALLOT_CHOICES ukfChoiceMask = 0x00000001;	// Initialize the first bit
	UINT_BALLOT_CHOICES ukmChoices = 0;			// Nothing selected yet
	QWidget ** ppWidgetStop;
	QWidget ** ppWidget = m_arraypwButtonOptions.PrgpGetWidgetsStop(OUT &ppWidgetStop);
	while (ppWidget != ppWidgetStop)
		{
		WButtonRadio * pwButtonRadio = (WButtonRadio *)*ppWidget++;
		if (pwButtonRadio->isChecked())
			ukmChoices |= ukfChoiceMask;
		ukfChoiceMask <<= 1;
		}
	if (ukmChoices == 0)
		{
		EMessageBoxWarning("Please select a choice!");
		return;
		}
	m_pEventBallotVote->SetChoices(ukmChoices);
	DDialogOkCancelWithLayouts::SL_ButtonOK_clicked();
	}
