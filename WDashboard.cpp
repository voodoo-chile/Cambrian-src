#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif
#include "WDashboard.h"

QFont g_oFontNormal;
QFont g_oFontBold;
QPen g_oPenDot;		// Pen to draw a dot to display a number within
QPen g_oPenDefault;

void
CPainterCell::DrawTextWithinCell(const QString & sText)
	{
	drawText(IN m_rcCell, Qt::AlignVCenter, sText);
	}

void
CPainterCell::DrawTextWithinCell_VE(PSZAC pszFmtTemplate, ...)
	{
	va_list vlArgs;
	va_start(OUT vlArgs, pszFmtTemplate);
	g_strScratchBufferStatusBar.Format_VL(pszFmtTemplate, vlArgs);
	DrawTextWithinCell(g_strScratchBufferStatusBar);
	}

void
CPainterCell::DrawTextUnderlineDotted(const QString & sText)
	{
	return;
	drawText(m_rcCell, sText);
	QFontMetrics oFontMetrics = fontMetrics();
	int cxWidth = oFontMetrics.width(sText);
	QPen oPenOld = pen();
	QPen oPen = oPenOld;
	oPen.setStyle(Qt::DotLine);
	setPen(oPen);
	DrawLineHorizontal(m_rcCell.left() + 2, cxWidth + 4, m_rcCell.top() + oFontMetrics.height() + 1);
	setPen(oPenOld);
	/*
	QFont oFontFooter = g_oFontNormal;
	oFontFooter.setUnderline(true);
	oPainter.setFont(oFontFooter);
	*/
	}

void
CPainterCell::DrawIconLeft(const QIcon & oIcon)
	{
	QRect rcIcon = m_rcCell;
	rcIcon.setWidth(16);
	oIcon.paint(this, rcIcon);
	m_rcCell.setLeft(m_rcCell.left() + 19);
	}

void
CPainterCell::DrawIconLeft(EMenuAction eMenuIcon)
	{
	DrawIconLeft(PGetMenuAction(eMenuIcon)->icon());
	}

//	Return the number of pixels of the drawing.  This is useful to chain drawing.
int
CPainterCell::DrawNumberWithinCircle(int nNumber)
	{
	//nNumber = qrand() % 100;
	if (nNumber > 0)
		{
		QRect rc = m_rcCell;
		rc.setLeft(m_rcCell.right() - 18);
		QPoint ptCenter = rc.center();
		setPen(g_oPenDot);
		ptCenter.setY(ptCenter.y() + 2);
		drawPoint(ptCenter);
		//DrawLineHorizontal(rc.left(), rc.right(), ptCenter.y());	// Draw a longer line for large numbers
		setPen(g_oPenDefault);
		//setFont(g_oFontBold);
		drawText(rc, Qt::AlignVCenter | Qt::AlignCenter, QString::number(nNumber));
		m_rcCell.setRight(m_rcCell.right() - 18);
		return 16;
		}
	return 0;
	}


WDashboard * g_pwDashboard;

void
CArrayPtrDashboardSectionItems::DeleteAllItems()
	{
	CDashboardSectionItem ** ppItemStop;
	CDashboardSectionItem ** ppItem = PrgpGetItemsStop(OUT &ppItemStop);
	while (ppItem != ppItemStop)
		{
		CDashboardSectionItem * pItem = *ppItem++;
		delete pItem;
		}
	RemoveAllElements();
	}

void
CArrayPtrDashboardSectionItems::AllocateItem(void * pvDataItem)
	{
	Add(PA_CHILD new CDashboardSectionItem(pvDataItem));
	}

/*
void
CArrayPtrDashboardSectionItems::AllocateItems(const CArray & arraypDataItems, int cDataItemsMax)
	{
	void ** ppvDataItemStop;
	void ** ppvDataItem = arraypDataItems.PrgpvGetElementsStopMax(OUT &ppvDataItemStop, cDataItemsMax);
	while (ppvDataItem != ppvDataItemStop)
		AllocateItem(*ppvDataItem++);
	}
*/

void
CArrayPtrDashboardSectionItems::AllocateTreeItems(const CArrayPtrTreeItems & arraypTreeItems, int cDataItemsMax)
	{
	ITreeItem ** ppTreeItemStop;
	ITreeItem ** ppTreeItem = arraypTreeItems.PrgpGetTreeItemsStopMax(OUT &ppTreeItemStop, cDataItemsMax);
	while (ppTreeItem != ppTreeItemStop)
		{
		ITreeItem * pTreeItem = *ppTreeItem++;
		CDashboardSectionItem * paItem = new CDashboardSectionItem(pTreeItem);
		paItem->m_uFlagsItem |= CDashboardSectionItem::FI_keDataType_ITreeItem;
		Add(PA_CHILD paItem);
		}
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
void
WDashboardSectionChannels::InitItems(TProfile * pProfile)
	{
	CArrayPtrGroups arraypGroups;
	pProfile->GetRecentChannels(OUT &arraypGroups);
	m_arraypaItems.AllocateTreeItems(arraypGroups, 10);
	}

void
WDashboardSectionGroups::InitItems(TProfile * pProfile)
	{
	CArrayPtrGroups arraypGroups;
	pProfile->GetRecentGroups(OUT &arraypGroups);
	m_arraypaItems.AllocateTreeItems(arraypGroups, 10);
	}

void
WDashboardSectionContacts::InitItems(TProfile * pProfile)
	{
	CArrayPtrContacts arraypContacts;
	pProfile->GetRecentContacts(OUT &arraypContacts);
	m_arraypaItems.AllocateTreeItems(arraypContacts, 10);
	}

void
WDashboardSectionBallots::InitItems(TProfile * pProfile)
	{
	/*
	IEvent ** ppEventStop;
	IEvent ** ppEvent = pProfile->m_arraypEventsRecentBallots.PrgpGetEventsStopLast(OUT &ppEventStop);
	while (ppEvent != ppEventStop)
		{
		IEvent * pEvent = *--ppEventStop;
		m_arraypaItems.AddItem(new CDashboardSectionItem_IEventBallot((IEventBallot *)pEvent));
		}
	*/
	}

void
WDashboardSectionChannels::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * pvGroupChannel)
	{
	TGroup * pGroup = (TGroup *)pvGroupChannel;
	pPainter->DrawNumberWithinCircle(pGroup->m_cMessagesUnread);
	pPainter->DrawTextWithinCell(pGroup->TreeItem_SGetNameDisplay());
	}

void
WDashboardSectionChannels::DrawFooter(CPainterCell * pPainter, UINT uFlagsItem)
	{
	pPainter->DrawTextUnderlineDotted("+10 more...");
	}

void
WDashboardSectionGroups::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * pvGroup)
	{
	TGroup * pGroup = (TGroup *)pvGroup;
	pPainter->DrawNumberWithinCircle(pGroup->m_cMessagesUnread);
	pPainter->DrawTextWithinCell(pGroup->TreeItem_SGetNameDisplay());
	}

void
WDashboardSectionContacts::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * pvContact)
	{
	TContact * pContact = (TContact *)pvContact;
	//EMenuAction eMenuIconPresence = pContact->Contact_EGetMenuActionPresence();
	pPainter->DrawIconLeft(pContact->Contact_FuIsOnline() ? eMenuAction_PresenceAccountOnline : eMenuAction_PresenceAccountOffline);
	pPainter->DrawNumberWithinCircle(pContact->m_cMessagesUnread);
	pPainter->DrawTextWithinCell_VE("$s", pContact->TreeItem_PszGetNameDisplay());
	}


void
WDashboardSectionContacts::DrawFooter(CPainterCell * pPainter, UINT /*uFlagsItem*/)
	{
	pPainter->DrawTextWithinCell_VE("+ $I more peers", 10);
	}

singleton WDashboardCaption : public QWidget
{
public:
	WDashboardCaption();
};

WDashboardCaption::WDashboardCaption()
	{
	QToolButton * pwButtonUndock = new QToolButton(this);
	pwButtonUndock->setToolTip("Float / Unfloat");
	pwButtonUndock->setStyleSheet("QToolButton { border: none; padding: 3px; }");
	QPixmap oPixmap = style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
	pwButtonUndock->setIcon(oPixmap);
	pwButtonUndock->setCursor(Qt::ArrowCursor);
	pwButtonUndock->setFocusPolicy(Qt::ClickFocus);

	QHBoxLayout * layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->addStretch();
	layout->addWidget(pwButtonUndock);
	setLayout(layout);
	setCursor(Qt::OpenHandCursor);		// This cursor shows to the user he/she may drag the widget to undock the Navigation Tree
	}

#include "WNavigationTree.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
WDashboard::WDashboard()
	{
	Assert(g_pwDashboard == NULL);
	g_pwDashboard = this;
	m_pProfile = NULL;
	m_pItemSelected = NULL;
	setObjectName("Dashboard");
	g_oFontBold = g_oFontNormal = font();
	g_oFontBold.setWeight(QFont::Bold);

	m_pwLabelCaption = new WLabel;
	//m_pwLabelCaption->setStyleSheet("background-color:#8080FF");
	m_pwLabelCaption->setStyleSheet("background-color:#3e313c; color:white; font-size:16px; border-bottom:2px solid #372c36; padding:2px");
	m_pwLabelCaption->setMargin(3);
	m_pwLabelCaption->setFont(g_oFontBold);
	#if 1
	setTitleBarWidget(m_pwLabelCaption);
	#else
	setTitleBarWidget(new WDashboardCaption);
	#endif

	QWidget * pwWidgetDashboard = new QWidget;	// Main widget for the dashboard
	#if 0
	pwWidgetDashboard->setStyleSheet("background-color:#A0A0FF;");	// Light blue
	#else
	pwWidgetDashboard->setStyleSheet("background-color:#4d394b;");
	#endif
	setWidget(PA_CHILD pwWidgetDashboard);
	m_poLayoutVertial = new OLayoutVerticalAlignTop(pwWidgetDashboard);
	Layout_MarginsClear(INOUT m_poLayoutVertial);
	m_poLayoutVertial->setSpacing(0);

	InitToGarbage(OUT &m_sections, sizeof(m_sections));
	m_sections.pwSectionBalots = new WDashboardSectionBallots("Ballots");
	m_sections.pwSectionChannels = new WDashboardSectionChannels("Channels");
	m_sections.pwSectionContacts = new WDashboardSectionContacts("Peers");
	m_sections.pwSectionGroups = new WDashboardSectionGroups("Private Groups");

	// Add each section to the vertical layout
	for (WDashboardSection ** ppwSection = (WDashboardSection **)&m_sections; (BYTE *)ppwSection < (BYTE *)&m_sections + sizeof(m_sections); ppwSection++)
		m_poLayoutVertial->addWidget(*ppwSection);
	}

void
WDashboard::ProfileSelectedChanged(TProfile * pProfile)
	{
	m_pProfile = pProfile;
	m_pwLabelCaption->Label_SetTextPlain((pProfile != NULL) ? pProfile->m_strNameProfile : c_strEmpty);
	m_pItemSelected = NULL;
	// Notify each section the selected profile changed
	for (WDashboardSection ** ppwSection = (WDashboardSection **)&m_sections; (BYTE *)ppwSection < (BYTE *)&m_sections + sizeof(m_sections); ppwSection++)
		{
		WDashboardSection * pwSection = *ppwSection;
		pwSection->m_pParent = this;
		pwSection->m_pItemMouseHovering = NULL;
		pwSection->m_arraypaItems.DeleteAllItems();
		if (pProfile != NULL)
			pwSection->InitItems(pProfile);
		pwSection->updateGeometry();
		}
	//m_poLayoutVertial->invalidate();
	//updateGeometry();
	}

BOOL
WDashboard::FSelectItem(CDashboardSectionItem * pItem)
	{
	if (pItem == m_pItemSelected)
		return FALSE;
	if (m_pItemSelected != NULL)
		{
		if (m_pItemSelected->m_uFlagsItem & CDashboardSectionItem::FI_kfSelected)
			{
			m_pItemSelected->m_uFlagsItem &= ~CDashboardSectionItem::FI_kfSelected;
			}
		}
	if (pItem != NULL)
		{
		pItem->m_uFlagsItem |= CDashboardSectionItem::FI_kfSelected;
		}
	m_pItemSelected = pItem;
	update();	// Redraw the whole dashboard. This is not efficient, however it works.  Ideally there should be a mechanism to redraw individual items
	return TRUE;
	}

void
WDashboard::NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ)
	{
	Assert(pContactOrGroup_NZ != NULL);
	if (pContactOrGroup_NZ->PGetProfile() == m_pProfile)
		ProfileSelectedChanged(m_pProfile);	// At the moment, refresh the entire thing.  This is not efficient, but it works
	}

void
WDashboard::NewEventRelatedToBallot(IEventBallot * pEventBallot)
	{
	// Search if the ballot is already there, and if not, add it
	//m_sections.pwSectionBalots->m_arraypaItems
	}

void
WDashboard::RefreshContact(TContact * pContact)
	{
	Assert(pContact != NULL);
	if (pContact->PGetProfile() == m_pProfile)
		m_sections.pwSectionContacts->repaint();
	}

void
WDashboard::RefreshGroup(TGroup * pGroup)
	{
	if (pGroup->PGetProfile() == m_pProfile)
		m_sections.pwSectionGroups->repaint();
	}

///////////////////////////////////////////////////////////////////////////////////////////////////
WDashboardSection::WDashboardSection(PSZAC pszSectionName) : WWidget(NULL)
	{
	m_sName = pszSectionName;
	m_pItemMouseHovering = NULL;
	setMouseTracking(true);
	}

WDashboardSection::~WDashboardSection()
	{
	//m_arraypaItems.DeleteAllItems();
	}

void
WDashboardSection::InitItems(TProfile * pProfile)
	{
	Assert(pProfile != NULL);
	}

void
WDashboardSection::DrawItem(CPainterCell * pPainter, UINT /*uFlagsItem*/, void * pvDataItem)
	{
	}

void
WDashboardSection::DrawFooter(CPainterCell * pPainter, UINT /*uFlagsItem*/)
	{
	}

void
WDashboardSection::OnItemSelected(CDashboardSectionItem * pItem)
	{
	Assert(pItem != NULL);
	UINT ukmDataType = (pItem->m_uFlagsItem & CDashboardSectionItem::FI_kmDataTypeMask);
	if (ukmDataType == CDashboardSectionItem::FI_keDataType_ITreeItem)
		{
		NavigationTree_SelectTreeItem(pItem->m_data.piTreeItem);
		return;
		}

	}

#define d_cxMarginSection		5
#define d_cxWidthNumber			20
#define d_cyHeightSectionItem	18
#define d_yPosFirstItem			3 + d_cyHeightSectionItem

QSize
WDashboardSection::sizeHint() const
	{
	return QSize(150, m_arraypaItems.GetSize() * d_cyHeightSectionItem + 28 + 16);
	}

//	WDashboardSection::QWidget::heightForWidth()
int
WDashboardSection::heightForWidth(int cxWidth) const
	{
	MessageLog_AppendTextFormatSev(eSeverityErrorAssert, "WDashboardSection::heightForWidth($i)\n", cxWidth);
	Assert(FALSE && "Is this method ever called?");
	return QWidget::heightForWidth(cxWidth);
	}

#define d_coItemSelected	0x4C9689
#define d_coItemMouseHover	0x3E313C

//	WDashboardSection::QWidget::paintEvent()
void
WDashboardSection::paintEvent(QPaintEvent *)
	{
	CPainterCell oPainter(this);
//	oPainter.drawRect(0, 0, width() - 1, height() - 1);
	g_oPenDefault = oPainter.pen();
	g_oPenDefault.setColor(d_coWhite);
	#if 1
	oPainter.setRenderHint(QPainter::Antialiasing,true);
	oPainter.setRenderHint(QPainter::SmoothPixmapTransform,true);
	#endif
	g_oPenDot.setColor(0xEB4D5C);
	g_oPenDot.setWidth(16);
	g_oPenDot.setCapStyle(Qt::RoundCap);
//	oPainter.setOpacity(0.7);

	oPainter.setPen(g_oPenDefault);

	QRect rcItemBackground;
	QRect rcSection = rect();
	qreal xRight = rcSection.width();
	qreal yTop = d_yPosFirstItem;

	oPainter.m_rcCell.setTop(d_yPosFirstItem);
	oPainter.m_rcCell.setHeight(d_cyHeightSectionItem);
	CDashboardSectionItem ** ppItemStop;
	CDashboardSectionItem ** ppItem = m_arraypaItems.PrgpGetItemsStop(OUT &ppItemStop);
	while (ppItem != ppItemStop)
		{
		oPainter.m_rcCell.setLeft(d_cxMarginSection);
		oPainter.m_rcCell.setRight(xRight);
		//oPainter.drawRect(oPainter.m_rcCell);
		CDashboardSectionItem * pItem = *ppItem++;
		UINT uFlagsItem = pItem->m_uFlagsItem;
		//uFlagsItem = qrand() % 8;
		QRGB coBackgroundFill = d_zNA;
		if (uFlagsItem & CDashboardSectionItem::FI_kfMouseHover)
			coBackgroundFill = d_coItemMouseHover;
		if (uFlagsItem & CDashboardSectionItem::FI_kfSelected)
			{
			uFlagsItem |= CDashboardSectionItem::FI_kfDrawBold;	// All selected items are in bold
			coBackgroundFill = d_coItemSelected;
			}
		if (coBackgroundFill != d_zNA)
			{
			rcItemBackground = oPainter.m_rcCell;
			rcItemBackground.setLeft(0);
			oPainter.fillRect(rcItemBackground, coBackgroundFill);
			}
		oPainter.setFont((uFlagsItem & CDashboardSectionItem::FI_kfDrawBold) ? g_oFontBold : g_oFontNormal);
		DrawItem(&oPainter, uFlagsItem, pItem->m_data.pvDataItem);
		yTop += d_cyHeightSectionItem;
		oPainter.m_rcCell.moveTop(yTop);
		}
	// Draw the footer
	DrawFooter(&oPainter, 0);
	/*
	//Qt::DashDotDotLine
	QTextCharFormat warningFormat;
	warningFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
	warningFormat.setUnderlineColor(Qt::darkYellow);
	*/

	// Draw the caption last using the bold font
	oPainter.setFont(g_oFontBold);
	/*
	rcSection.setHeight(d_cyHeightSectionItem);
	p.drawText(d_cxMarginSection, IN rcSection, Qt::AlignVCenter, m_sName);
	*/

	oPainter.drawText(d_cxMarginSection, d_cyHeightSectionItem, m_sName);
	oPainter.drawLine(rcSection.bottomLeft(), rcSection.bottomRight());
	/*
	oPainter.drawRect( 10, 10, 85, 35 );
	oPainter.drawRoundRect( 10, 55, 85, 35 );
	QRect rect( 105, 10, 85, 35 );
	oPainter.drawRoundRect( rect );
	oPainter.drawRect( rect.translated( 0, 45 ) );
	*/
	}

CDashboardSectionItem *
WDashboardSection::_PGetItemAtPosY(int yPos, OUT EHitTestSection * peHitTestSection) const
	{
	*peHitTestSection = eHitTestSection_zNone;
	if (yPos <= d_yPosFirstItem)
		{
		*peHitTestSection = eHitTestSection_kfHeader;
		return NULL;
		}
	int ySection = d_yPosFirstItem + d_cyHeightSectionItem;
	// Perform some hit testing
	CDashboardSectionItem ** ppItemStop;
	CDashboardSectionItem ** ppItem = m_arraypaItems.PrgpGetItemsStop(OUT &ppItemStop);
	while (ppItem != ppItemStop)
		{
		CDashboardSectionItem * pItem = *ppItem++;
		if (yPos <= ySection)
			return pItem;
		ySection += d_cyHeightSectionItem;
		}
	return NULL;
	}

void
WDashboardSection::_SetItemMouseHovering(CDashboardSectionItem * pItemMouseHovering)
	{
	if (pItemMouseHovering == m_pItemMouseHovering)
		return;	// Nothing to do
	if (m_pItemMouseHovering != NULL)
		m_pItemMouseHovering->m_uFlagsItem &= ~CDashboardSectionItem::FI_kfMouseHover;
	if (pItemMouseHovering != NULL)
		pItemMouseHovering->m_uFlagsItem |= CDashboardSectionItem::FI_kfMouseHover;
	m_pItemMouseHovering = pItemMouseHovering;
	//repaint();
	update();
	}

void
WDashboardSection::mouseMoveEvent(QMouseEvent * pEventMouse)
	{
	if (pEventMouse->button() == Qt::NoButton)
		{
		QPoint ptMouse = pEventMouse->pos();
		EHitTestSection eHitTestSection;
		_SetItemMouseHovering(_PGetItemAtPosY(ptMouse.y(), OUT &eHitTestSection));
		}
	} // mouseMoveEvent()


void
WDashboardSection::mouseReleaseEvent(QMouseEvent * pEventMouse)
	{
	MessageLog_AppendTextFormatSev(eSeverityNoise, "WDashboardSection::mouseReleaseEvent() - buttons=$x\n", pEventMouse->button());
	if (pEventMouse->button() == Qt::LeftButton)
		{
		EHitTestSection eHitTestSection;
		CDashboardSectionItem * pItemMouseClicked = _PGetItemAtPosY(pEventMouse->pos().y(), OUT &eHitTestSection);
		if (pItemMouseClicked == m_pItemMouseHovering)
			{
			if (m_pParent->FSelectItem(pItemMouseClicked) && pItemMouseClicked != NULL)
				OnItemSelected(pItemMouseClicked);
			}
		}
	}

void
WDashboardSection::leaveEvent(QEvent *)
	{
	_SetItemMouseHovering(NULL);
	}

void
Dashboard_UpdateAccordingToSelectedProfile(TProfile * pProfileSelected)
	{
	Endorse(pProfileSelected == NULL);
	if (g_pwDashboard != NULL)
		g_pwDashboard->ProfileSelectedChanged(pProfileSelected);
	}

void
Dashboard_UpdateContact(TContact * pContact)
	{
	if (g_pwDashboard != NULL)
		g_pwDashboard->RefreshContact(pContact);
	}

void
Dashboard_UpdateGroup(TGroup * pGroup)
	{
	if (g_pwDashboard != NULL)
		g_pwDashboard->RefreshGroup(pGroup);
	}

void
Dashboard_NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ)
	{
	if (g_pwDashboard != NULL)
		g_pwDashboard->NewEventsFromContactOrGroup(pContactOrGroup_NZ);
	}

void
Dashboard_NewEventRelatedToBallot(IEventBallot * pEventBallot)
	{
	if (g_pwDashboard != NULL)
		g_pwDashboard->NewEventRelatedToBallot(pEventBallot);
	}

void
Dashboard_RemoveSelection()
	{
	Assert(g_pwDashboard != NULL);
	(void)g_pwDashboard->FSelectItem(NULL);
	}
