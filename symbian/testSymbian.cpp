
#include <eikstart.h>
#include <aknapp.h>
#include <akndoc.h>
#include <aknappui.h>
#include <coecntrl.h>
#include <coemain.h>

//appview
class CPAppView : public CCoeControl
{
public:
	static CPAppView* NewL(const TRect& aRect)
	{
		CPAppView* self = CPAppView::NewLC(aRect);
		CleanupStack::Pop(self);
		return self;
	}

	static CPAppView* NewLC(const TRect& aRect)
	{
		CPAppView* self = new (ELeave) CPAppView;
		CleanupStack::PushL(self);
		self->ConstructL(aRect);
		return self;
	}

	virtual ~CPAppView()
	{
	}
public:
	void Draw(const TRect& aRect) const;

	virtual void SizeChanged()
	{
    	DrawNow();
	}
private:
	void ConstructL(const TRect& aRect)
	{
		CreateWindowL();
		SetRect( aRect );
		ActivateL();
	}

	CPAppView()
	{
	}
};

void CPAppView::Draw(const TRect& aRect) const
{
    CWindowGc& gc = SystemGc();
    TRect drawRect(Rect());
    gc.Clear(drawRect);
	//add paint codes


}

//appui
class CPAppUi : public CAknAppUi
{
public:
	void ConstructL()
	{
		// Initialise app UI with standard value.
		BaseConstructL();

		// Create view object
		iAppView = CPAppView::NewL( ClientRect() );
	}
	
	CPAppUi()
	{
	}

	virtual ~CPAppUi()
	{
		if (iAppView) {
			delete iAppView;
			iAppView = NULL;
		}
	}
private:
    void HandleCommandL(TInt aCommand);
	void HandleStatusPaneSizeChange();
private:
	CPAppView* iAppView;
};

void CPAppUi::HandleStatusPaneSizeChange()
{
	iAppView->SetRect( ClientRect() );
}

void CPAppUi::HandleCommandL(TInt aCommand)
{
	switch( aCommand )
	{
		case EEikCmdExit:
		case EAknSoftkeyExit:
			Exit();
			break;
		default:
			break;
	}
}


//document
class CPDocument : public CAknDocument
{
public:
	virtual ~CPDocument()
	{
	}

	static CPDocument* NewL(CEikApplication& aApp)
	{
		CPDocument* self = NewLC(aApp);
		CleanupStack::Pop(self);
		return self;
	}

	static CPDocument* NewLC(CEikApplication& aApp)
	{
		CPDocument* self = new (ELeave) CPDocument(aApp);
		CleanupStack::PushL(self);
		self->ConstructL();
		return self;
	}

public:
	CEikAppUi* CreateAppUiL()
	{
    	return (static_cast <CEikAppUi*> (new (ELeave) CPAppUi));
	}
private:
	void ConstructL()
	{
	}

	CPDocument(CEikApplication& aApp)
    	: CAknDocument(aApp)
	{
	}
};


// UID for the application;
const TUid KUidtestApp = { 0x0CF49CC0 };

//application
class CPApplication : public CAknApplication
{
public:
	TUid AppDllUid() const
	{
    	return KUidtestApp;
	}
protected:
	CApaDocument* CreateDocumentL()
	{
    	return (static_cast<CApaDocument*>
                    ( CPDocument::NewL(*this)));
	}
};

//system interface
LOCAL_C CApaApplication* NewApplication()
	{
	return new CPApplication;
	}

GLDEF_C TInt E32Main()
	{
	return EikStart::RunApplication( NewApplication );
	}

