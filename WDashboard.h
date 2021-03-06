#ifndef WDASHBOARD_H
#define WDASHBOARD_H
#ifndef PRECOMPILEDHEADERS_H
	#include "PreCompiledHeaders.h"
#endif

class WDashboard;

//	Class having a 'boundary rectangle' where the painter is allowed to draw.
//	A typical use of this class is to paint a cell in a grid.
class CPainterCell : public OPainter
{
public:
	QRect m_rcCell;
	int m_yCenter;		// Center point of the cell.

public:
	CPainterCell(QPaintDevice * poPaintDevice): OPainter(poPaintDevice) { }

	void DrawTextWithinCell(const QString & sText);
	void DrawTextWithinCell_VE(PSZAC pszFmtTemplate, ...);
	void DrawTextUnderlineDotted(const QString & sText);
	int DrawNumberWithinCircle(int nNumber);
	void DrawIconLeft(const QIcon & oIcon);
	void DrawIconLeft(EMenuAction eMenuIcon);
};

//	Extra information regarding the hit-testing
enum EHitTestSection
	{
	eHitTestSection_zNone		= 0x0000,
	eHitTestSection_kfHeader	= 0x0001,
	eHitTestSection_kfFooter	= 0x0002
	};

//	Interface to draw one item on the dashboard
class CDashboardSectionItem	// (item)
{
public:
	enum
		{
		FI_keDataType_zUnknown		= 0x0000,
		FI_keDataType_ITreeItem		= 0x0001,	// The content of m_data contains a pointer to ITreeItem
		FI_keDataType_IEvent		= 0x0002,	// The content of m_data contains a pointer to IEvent
		FI_kmDataTypeMask			= 0x000F,

		FI_kfMouseHover				= 0x0010,	// The mouse is hovering over the item
		FI_kfSelected				= 0x0020,
		FI_kfDrawBold				= 0x0100,	// Draw the item in bold
		};
	UINT m_uFlagsItem;
	union
		{
		void * pvDataItem;			// Generic pointer for the data related to the item

		ITreeItem * piTreeItem;
		TContact * pContact;
		TGroup * pGroup;

		IEvent * pEvent;
		IEventBallot * pEventBallot;
		} m_data;

public:
	CDashboardSectionItem(void * pvDataItem) { m_uFlagsItem = 0; m_data.pvDataItem = pvDataItem; }
};

class CArrayPtrDashboardSectionItems : private CArray
{
public:
	inline CDashboardSectionItem ** PrgpGetItemsStop(OUT CDashboardSectionItem *** pppItemStop) const { return (CDashboardSectionItem **)PrgpvGetElementsStop(OUT (void ***)pppItemStop); }
	inline int GetSize() const { return CArray::GetSize(); }

	void DeleteAllItems();
	void AllocateItem(void * pvDataItem);
	void AllocateTreeItems(const CArrayPtrTreeItems & arraypTreeItems, int cDataItemsMax);
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WDashboardSection : public WWidget
{
public:
	WDashboard * m_pParent;	// This pointer is necessary when an item is clicked, the other item(s) selected from other section(s) are unselected.
	CString m_sName;		// Name of the section
	CArrayPtrDashboardSectionItems m_arraypaItems;	// Items to draw
	CDashboardSectionItem * m_pItemMouseHovering;	// Which item in the section has the mouse hovering
public:
	WDashboardSection(PSZAC pszSectionName);
	~WDashboardSection();
	virtual void InitItems(TProfile * pProfile);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvDataItem);
	virtual void DrawFooter(CPainterCell * pPainter, UINT uFlagsItem);
	virtual void OnItemSelected(CDashboardSectionItem * pItem);

protected:
	virtual QSize sizeHint() const;					// From QWidget
	virtual int heightForWidth(int cxWidth) const;	// From QWidget
	virtual void paintEvent(QPaintEvent *);			// From QWidget
	virtual void mouseMoveEvent(QMouseEvent * pEventMouse);
	virtual void mouseReleaseEvent(QMouseEvent * pEventMouse);
	virtual void leaveEvent(QEvent *);

	CDashboardSectionItem * _PGetItemAtPosY(int yPos, OUT EHitTestSection * peHitTestSection) const;
	void _SetItemMouseHovering(CDashboardSectionItem * pItemMouseHovering);
	void WidgetRedraw() { update(); }
};

class CArrayPtrDashboardSections : public CArray
{
public:
	inline WDashboardSection ** PrgpGetSectionsStop(OUT WDashboardSection *** pppSectionStop) const { return (WDashboardSection **)PrgpvGetElementsStop(OUT (void ***)pppSectionStop); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class WDashboardSectionChannels : public WDashboardSection
{
public:
	WDashboardSectionChannels(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvGroupChannel);
	virtual void DrawFooter(CPainterCell * pPainter, UINT uFlagsItem);
};

class WDashboardSectionGroups : public WDashboardSection
{
public:
	WDashboardSectionGroups(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvGroup);
};

class WDashboardSectionContacts : public WDashboardSection
{
public:
	WDashboardSectionContacts(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
	virtual void DrawItem(CPainterCell * pPainter, UINT uFlagsItem, void * pvContact);
	virtual void DrawFooter(CPainterCell * pPainter, UINT uFlagsItem);
};

class WDashboardSectionBallots : public WDashboardSection
{
public:
	WDashboardSectionBallots(PSZAC pszSectionName) : WDashboardSection(pszSectionName) { }
	virtual void InitItems(TProfile * pProfile);
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//	The dashboard is a widget showing recent events related to a profile
singleton WDashboard : public QDockWidget
{
	Q_OBJECT
protected:
	TProfile * m_pProfile;							// Pointer of the profile the dashboard is displaying
	WLabel * m_pwLabelCaption;
	OLayoutVerticalAlignTop * m_poLayoutVertial;	// Stack the sections vertically
//	CArrayPtrDashboardSections m_arraypSections;
	struct	// Contain one pointer per section.  Those pointers are for a quick access to a section
		{
		WDashboardSectionBallots * pwSectionBalots;
		WDashboardSectionChannels * pwSectionChannels;
		WDashboardSectionContacts * pwSectionContacts;
		WDashboardSectionGroups * pwSectionGroups;		// Private groups
		} m_sections;
	CDashboardSectionItem * m_pItemSelected;		// Which item is selected (has the focus)

public:
	WDashboard();
	void ProfileSelectedChanged(TProfile * pProfile);
	void NewEventsFromContactOrGroup(ITreeItemChatLogEvents * pContactOrGroup_NZ);
	void NewEventRelatedToBallot(IEventBallot * pEventBallot);
	void RefreshContact(TContact * pContact);
	void RefreshGroup(TGroup * pGroup);
	BOOL FSelectItem(CDashboardSectionItem * pItem);
}; // WDashboard

#endif // WDASHBOARD_H
