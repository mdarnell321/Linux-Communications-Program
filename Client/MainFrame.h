#ifndef MAINFRAME_H
#define MAINFRAME_H
#include "wxcrafter.h"

class MainFrame : public MainFrameBaseClass
{
public:
	MainFrame(wxWindow* parent);
	virtual ~MainFrame();

	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void ButtonOneClicked(wxCommandEvent & event);
	void SendMessage_Clicked(wxCommandEvent & event);
	void ConnectVoice_Clicked(wxCommandEvent & event);
	void ConnectVideo_Clicked(wxCommandEvent & event);
	void StreamFile_Clicked(wxCommandEvent & event);
	void RefreshPendingFiles();
	void AcceptStreamA_Clicked(wxCommandEvent & event);
	void DeclineStreamA_Clicked(wxCommandEvent & event);
	void AcceptStreamB_Clicked(wxCommandEvent & event);
	void DeclineStreamB_Clicked(wxCommandEvent & event);
	void AcceptStreamC_Clicked(wxCommandEvent & event);
	void DeclineStreamC_Clicked(wxCommandEvent & event);
protected:
    virtual void OnMenuitem7MenuSelected(wxCommandEvent& event);
    virtual void ChatEnter(wxCommandEvent& event);
    virtual void FileMenuOpen(wxMenuEvent& event);
	virtual void sdgsdg(wxCommandEvent& event);
	virtual void UserMenuOpen(wxMenuEvent& event);
	virtual void UserList_Select(wxCommandEvent& event);
	virtual void User_ShowVideo(wxCommandEvent& event);
	virtual void User_CloseVideo(wxCommandEvent& event);
	virtual void Click_Disconnect(wxCommandEvent& event);

};
#endif // MAINFRAME_H
