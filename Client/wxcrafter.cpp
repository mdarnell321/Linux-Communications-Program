//////////////////////////////////////////////////////////////////////
// This file was auto-generated by codelite's wxCrafter Plugin
// wxCrafter project file: wxcrafter.wxcp
// Do not modify this file by hand!
//////////////////////////////////////////////////////////////////////

#include "wxcrafter.h"


// Declare the bitmap loading function
extern void wxC9ED9InitBitmapResources();

static bool bBitmapLoaded = false;


MainFrameBaseClass::MainFrameBaseClass(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	: wxFrame(parent, id, title, pos, size, style)
{
	if ( !bBitmapLoaded ) {
		// We need to initialise the default bitmap handler
		wxXmlResource::Get()->AddHandler(new wxBitmapXmlHandler);
		wxC9ED9InitBitmapResources();
		bBitmapLoaded = true;
	}

	wxBoxSizer* boxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	this->SetSizer(boxSizer1);

	wxBoxSizer* boxSizer71 = new wxBoxSizer(wxVERTICAL);

	boxSizer1->Add(boxSizer71, 0, wxALL|wxEXPAND|wxALIGN_LEFT, WXC_FROM_DIP(5));

	wxBoxSizer* boxSizer18 = new wxBoxSizer(wxHORIZONTAL);

	boxSizer71->Add(boxSizer18, 0, wxALL|wxALIGN_LEFT, WXC_FROM_DIP(5));

	Connection_TextBox = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDLG_UNIT(this, wxSize(350,-1)), 0);
	Connection_TextBox->Hide();
#if wxVERSION_NUMBER >= 3000
	Connection_TextBox->SetHint(_("Enter Room Name."));
#endif

	boxSizer18->Add(Connection_TextBox, 0, wxALL, WXC_FROM_DIP(5));
	Connection_TextBox->SetMinSize(wxSize(350,-1));

	Connection_Button = new wxButton(this, wxID_ANY, _("Connect"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(120,-1)), 0);
	Connection_Button->Hide();

	boxSizer18->Add(Connection_Button, 0, wxALL, WXC_FROM_DIP(5));
	Connection_Button->SetMinSize(wxSize(120,-1));

	Message_Input = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDLG_UNIT(this, wxSize(350,-1)), wxTE_WORDWRAP|wxTE_PROCESS_ENTER);
#if wxVERSION_NUMBER >= 3000
	Message_Input->SetHint(_("Enter message."));
#endif

	boxSizer18->Add(Message_Input, 0, wxALL, WXC_FROM_DIP(5));
	Message_Input->SetMinSize(wxSize(350,-1));

	Send_Msg = new wxButton(this, wxID_ANY, _("Send Message"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(120,-1)), 0);

	boxSizer18->Add(Send_Msg, 0, wxALL, WXC_FROM_DIP(5));
	Send_Msg->SetMinSize(wxSize(120,-1));

	wxBoxSizer* boxSizer33 = new wxBoxSizer(wxHORIZONTAL);

	boxSizer71->Add(boxSizer33, 0, wxALL, WXC_FROM_DIP(5));

	wxArrayString ListBoxArr;
	ListBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDLG_UNIT(this, wxSize(335,400)), ListBoxArr, wxLB_SINGLE);

	boxSizer33->Add(ListBox, 0, wxALL, WXC_FROM_DIP(5));
	ListBox->SetMinSize(wxSize(335,400));

	wxArrayString User_ListArr;
	User_List = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDLG_UNIT(this, wxSize(150,400)), User_ListArr, wxLB_SINGLE);

	boxSizer33->Add(User_List, 0, wxALL, WXC_FROM_DIP(5));
	User_List->SetMinSize(wxSize(150,400));

	wxBoxSizer* boxSizer81 = new wxBoxSizer(wxHORIZONTAL);
	boxSizer81->SetMinSize(30,30);

	boxSizer71->Add(boxSizer81, 1, wxLEFT|wxRIGHT, WXC_FROM_DIP(5));

	wxStaticBoxSizer* A = new wxStaticBoxSizer( new wxStaticBox(this, wxID_ANY, _("Stream Communications")), wxHORIZONTAL);
	A->SetMinSize(150,60);

	boxSizer81->Add(A, 1, wxLEFT|wxRIGHT, WXC_FROM_DIP(5));

	Voice_Button = new wxButton(this, wxID_ANY, _("Connect Voice"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(100,-1)), 0);

	A->Add(Voice_Button, 0, wxALL, WXC_FROM_DIP(5));
	Voice_Button->SetMinSize(wxSize(100,-1));

	Video_Button = new wxButton(this, wxID_ANY, _("Connect Video"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(100,-1)), 0);

	A->Add(Video_Button, 0, wxALL, WXC_FROM_DIP(5));
	Video_Button->SetMinSize(wxSize(100,-1));

	A->Add(0, 50, 1, wxALL, WXC_FROM_DIP(5));
	A->SetMinSize(wxSize(150,60));

	wxStaticBoxSizer* staticBoxSizer87 = new wxStaticBoxSizer( new wxStaticBox(this, wxID_ANY, _("Stream File")), wxVERTICAL);
	staticBoxSizer87->SetMinSize(150,60);

	boxSizer81->Add(staticBoxSizer87, 1, wxLEFT|wxRIGHT, WXC_FROM_DIP(5));

	wxBoxSizer* boxSizer97 = new wxBoxSizer(wxHORIZONTAL);

	staticBoxSizer87->Add(boxSizer97, 1, wxLEFT|wxRIGHT|wxEXPAND, WXC_FROM_DIP(5));

	Send_File = new wxButton(this, wxID_ANY, _("Initiate"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(-1,-1)), 0);

	boxSizer97->Add(Send_File, 1, wxLEFT|wxRIGHT|wxTOP, WXC_FROM_DIP(5));

	Pick_File = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, _("Select a file"), wxT("*"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(-1,-1)), wxFLP_DEFAULT_STYLE|wxFLP_SMALL);

	boxSizer97->Add(Pick_File, 1, wxLEFT|wxRIGHT|wxTOP, WXC_FROM_DIP(5));

	MyFileGauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxDLG_UNIT(this, wxSize(210,10)), wxGA_HORIZONTAL);
	MyFileGauge->SetValue(10);

	staticBoxSizer87->Add(MyFileGauge, 0, wxALL, WXC_FROM_DIP(5));
	MyFileGauge->SetMinSize(wxSize(210,10));
	staticBoxSizer87->SetMinSize(wxSize(150,60));
	boxSizer81->SetMinSize(wxSize(30,30));

	wxStaticBoxSizer* IncomingFilePanel = new wxStaticBoxSizer( new wxStaticBox(this, wxID_ANY, _("Incoming File Streams")), wxVERTICAL);

	boxSizer71->Add(IncomingFilePanel, 1, wxLEFT|wxRIGHT|wxBOTTOM, WXC_FROM_DIP(5));

	wxBoxSizer* UserA_Stream = new wxBoxSizer(wxHORIZONTAL);

	IncomingFilePanel->Add(UserA_Stream, 0, wxLEFT|wxRIGHT|wxEXPAND, WXC_FROM_DIP(5));

	AStream_Text = new wxStaticText(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDLG_UNIT(this, wxSize(-1,-1)), 0);

	UserA_Stream->Add(AStream_Text, 0, wxLEFT|wxRIGHT, WXC_FROM_DIP(5));

	AStream_Percentage = new wxStaticText(this, wxID_ANY, _("000%"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(40,16)), 0);

	UserA_Stream->Add(AStream_Percentage, 0, 0, WXC_FROM_DIP(5));
	AStream_Percentage->SetMinSize(wxSize(40,16));

	AStream_Yes = new wxButton(this, wxID_ANY, _("Accept"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(65,22)), 0);

	UserA_Stream->Add(AStream_Yes, 0, 0, WXC_FROM_DIP(5));
	AStream_Yes->SetMinSize(wxSize(65,22));

	AStream_No = new wxButton(this, wxID_ANY, _("Reject"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(65,22)), 0);

	UserA_Stream->Add(AStream_No, 0, 0, WXC_FROM_DIP(5));
	AStream_No->SetMinSize(wxSize(65,22));

	wxBoxSizer* UserB_Stream = new wxBoxSizer(wxHORIZONTAL);

	IncomingFilePanel->Add(UserB_Stream, 0, wxLEFT|wxRIGHT|wxEXPAND, WXC_FROM_DIP(5));

	BStream_Text = new wxStaticText(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDLG_UNIT(this, wxSize(-1,-1)), 0);

	UserB_Stream->Add(BStream_Text, 0, wxLEFT|wxRIGHT, WXC_FROM_DIP(5));

	BStream_Percentage = new wxStaticText(this, wxID_ANY, _("000%"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(40,16)), 0);

	UserB_Stream->Add(BStream_Percentage, 0, wxLEFT|wxRIGHT, WXC_FROM_DIP(5));
	BStream_Percentage->SetMinSize(wxSize(40,16));

	BStream_Yes = new wxButton(this, wxID_ANY, _("Accept"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(65,22)), 0);

	UserB_Stream->Add(BStream_Yes, 0, 0, WXC_FROM_DIP(5));
	BStream_Yes->SetMinSize(wxSize(65,22));

	BStream_No = new wxButton(this, wxID_ANY, _("Reject"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(65,22)), 0);

	UserB_Stream->Add(BStream_No, 0, 0, WXC_FROM_DIP(5));
	BStream_No->SetMinSize(wxSize(65,22));

	wxBoxSizer* UserC_Stream = new wxBoxSizer(wxHORIZONTAL);

	IncomingFilePanel->Add(UserC_Stream, 0, wxLEFT|wxRIGHT|wxEXPAND, WXC_FROM_DIP(5));

	CStream_Text = new wxStaticText(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDLG_UNIT(this, wxSize(-1,-1)), 0);

	UserC_Stream->Add(CStream_Text, 0, wxLEFT|wxRIGHT, WXC_FROM_DIP(5));

	CStream_Percentage = new wxStaticText(this, wxID_ANY, _("000%"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(40,16)), 0);

	UserC_Stream->Add(CStream_Percentage, 0, wxLEFT|wxRIGHT, WXC_FROM_DIP(5));
	CStream_Percentage->SetMinSize(wxSize(40,16));

	CStream_Yes = new wxButton(this, wxID_ANY, _("Accept"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(65,22)), 0);

	UserC_Stream->Add(CStream_Yes, 0, 0, WXC_FROM_DIP(5));
	CStream_Yes->SetMinSize(wxSize(65,22));

	CStream_No = new wxButton(this, wxID_ANY, _("Reject"), wxDefaultPosition, wxDLG_UNIT(this, wxSize(65,22)), 0);

	UserC_Stream->Add(CStream_No, 0, 0, WXC_FROM_DIP(5));
	CStream_No->SetMinSize(wxSize(65,22));

	m_menuBar = new wxMenuBar(0);
	this->SetMenuBar(m_menuBar);

	m_name6 = new wxMenu();
	m_menuBar->Append(m_name6, _("File"));

	m_menuItem7 = new wxMenuItem(m_name6, wxID_EXIT, _("Exit\tAlt-X"), _("Quit"), wxITEM_NORMAL);
	m_name6->Append(m_menuItem7);

	m_mainToolbar = this->CreateToolBar(wxTB_FLAT, wxID_ANY);
	m_mainToolbar->SetToolBitmapSize(wxSize(16,16));

	SetName(wxT("MainFrameBaseClass"));
	SetMinClientSize(wxSize(500,650));
	SetSize(wxDLG_UNIT(this, wxSize(500,650)));
	if (GetSizer()) {
		GetSizer()->Fit(this);
	}
	if(GetParent()) {
		CentreOnParent(wxVERTICAL);
	} else {
		CentreOnScreen(wxVERTICAL);
	}
	// Connect events
	Message_Input->Connect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(MainFrameBaseClass::ChatEnter), NULL, this);
	User_List->Connect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(MainFrameBaseClass::UserList_Select), NULL, this);
	m_name6->Connect(wxEVT_MENU_OPEN, wxMenuEventHandler(MainFrameBaseClass::FileMenuOpen), NULL, this);
	this->Connect(m_menuItem7->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrameBaseClass::OnMenuitem7MenuSelected), NULL, this);

}

MainFrameBaseClass::~MainFrameBaseClass()
{
	Message_Input->Disconnect(wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(MainFrameBaseClass::ChatEnter), NULL, this);
	User_List->Disconnect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(MainFrameBaseClass::UserList_Select), NULL, this);
	m_name6->Disconnect(wxEVT_MENU_OPEN, wxMenuEventHandler(MainFrameBaseClass::FileMenuOpen), NULL, this);
	this->Disconnect(m_menuItem7->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrameBaseClass::OnMenuitem7MenuSelected), NULL, this);

}
