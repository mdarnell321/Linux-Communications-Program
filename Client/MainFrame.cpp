#include "MainFrame.h"
#include <wx/aboutdlg.h>
#include "Program.h"
#include "SendPackets.h"
#include "V4L2Manager.h"
#include "Form.h"
#include "MemoryManagement.h"

MainFrame::MainFrame(wxWindow* parent)
	: MainFrameBaseClass(parent)
{
	Connection_Button->Bind(wxEVT_BUTTON, &MainFrame::ButtonOneClicked, this);
	Send_Msg->Bind(wxEVT_BUTTON, &MainFrame::SendMessage_Clicked, this);
	Voice_Button->Bind(wxEVT_BUTTON, &MainFrame::ConnectVoice_Clicked, this);
	Video_Button->Bind(wxEVT_BUTTON, &MainFrame::ConnectVideo_Clicked, this);
	Send_File->Bind(wxEVT_BUTTON, &MainFrame::StreamFile_Clicked, this);
	//
	AStream_Yes->Bind(wxEVT_BUTTON, &MainFrame::AcceptStreamA_Clicked, this);
	AStream_No->Bind(wxEVT_BUTTON, &MainFrame::DeclineStreamA_Clicked, this);
	BStream_Yes->Bind(wxEVT_BUTTON, &MainFrame::AcceptStreamB_Clicked, this);
	BStream_No->Bind(wxEVT_BUTTON, &MainFrame::DeclineStreamB_Clicked, this);
	CStream_Yes->Bind(wxEVT_BUTTON, &MainFrame::AcceptStreamC_Clicked, this);
	CStream_No->Bind(wxEVT_BUTTON, &MainFrame::DeclineStreamC_Clicked, this);
}

MainFrame::~MainFrame()
{

}
void MainFrame::RefreshPendingFiles()
{
	Form::mainFrame->CallAfter([this]() {
		switch(Program::File_Transfers.size()) {
		case 1:
			Form::mainFrame->GetAStream_No()->Show(Program::File_Transfers[0]->Host != Program::me && Program::File_Transfers[0]->responded == false);
			Form::mainFrame->GetAStream_Text()->Show(true);
			Form::mainFrame->GetAStream_Yes()->Show(Program::File_Transfers[0]->Host != Program::me && Program::File_Transfers[0]->responded == false);
			Form::mainFrame->GetAStream_Text()->SetLabel(wxString::Format("File of %llu bytes from %s", Program::File_Transfers[0]->total_bytes,Program::File_Transfers[0]->Host->name));
			Form::mainFrame->GetAStream_Percentage()->Show(Program::File_Transfers[0]->pending == false);
			//
			Form::mainFrame->GetBStream_No()->Show(false);
			Form::mainFrame->GetBStream_Text()->Show(false);
			Form::mainFrame->GetBStream_Yes()->Show(false);
			Form::mainFrame->GetBStream_Percentage()->Show(false);
			//
			Form::mainFrame->GetCStream_No()->Show(false);
			Form::mainFrame->GetCStream_Text()->Show(false);
			Form::mainFrame->GetCStream_Yes()->Show(false);
			Form::mainFrame->GetCStream_Percentage()->Show(false);
			break;
		case 2:
			Form::mainFrame->GetBStream_No()->Show(Program::File_Transfers[1]->Host != Program::me && Program::File_Transfers[1]->responded == false);
			Form::mainFrame->GetBStream_Text()->Show(true);
			Form::mainFrame->GetBStream_Yes()->Show(Program::File_Transfers[1]->Host != Program::me && Program::File_Transfers[1]->responded == false);
			Form::mainFrame->GetBStream_Percentage()->Show(Program::File_Transfers[1]->pending == false);
			//
			Form::mainFrame->GetAStream_No()->Show(Program::File_Transfers[0]->Host != Program::me&& Program::File_Transfers[0]->responded == false);
			Form::mainFrame->GetAStream_Text()->Show(true);
			Form::mainFrame->GetAStream_Yes()->Show(Program::File_Transfers[0]->Host != Program::me&& Program::File_Transfers[0]->responded == false);
			Form::mainFrame->GetAStream_Percentage()->Show(Program::File_Transfers[0]->pending == false);
			//
			Form::mainFrame->GetCStream_No()->Show(false);
			Form::mainFrame->GetCStream_Text()->Show(false);
			Form::mainFrame->GetCStream_Yes()->Show(false);
			Form::mainFrame->GetCStream_Percentage()->Show(false);
			Form::mainFrame->GetAStream_Text()->SetLabel(wxString::Format("File of %llu bytes from %s", Program::File_Transfers[0]->total_bytes,Program::File_Transfers[0]->Host->name));
			Form::mainFrame->GetBStream_Text()->SetLabel(wxString::Format("File of %llu bytes from %s", Program::File_Transfers[1]->total_bytes,Program::File_Transfers[1]->Host->name));

			break;
		case 3:
			Form::mainFrame->GetCStream_No()->Show(Program::File_Transfers[2]->Host != Program::me&& Program::File_Transfers[2]->responded == false);
			Form::mainFrame->GetCStream_Text()->Show(true);
			Form::mainFrame->GetCStream_Yes()->Show(Program::File_Transfers[2]->Host != Program::me&& Program::File_Transfers[2]->responded == false);
			Form::mainFrame->GetAStream_No()->Show(Program::File_Transfers[0]->Host != Program::me&& Program::File_Transfers[0]->responded == false);
			Form::mainFrame->GetAStream_Text()->Show(true);
			Form::mainFrame->GetAStream_Yes()->Show(Program::File_Transfers[0]->Host != Program::me&& Program::File_Transfers[0]->responded == false);
			Form::mainFrame->GetBStream_No()->Show(Program::File_Transfers[1]->Host != Program::me&& Program::File_Transfers[1]->responded == false);
			Form::mainFrame->GetBStream_Text()->Show(true);
			Form::mainFrame->GetBStream_Yes()->Show(Program::File_Transfers[1]->Host != Program::me&& Program::File_Transfers[1]->responded == false);

			Form::mainFrame->GetAStream_Percentage()->Show(Program::File_Transfers[0]->pending == false);
			Form::mainFrame->GetBStream_Percentage()->Show(Program::File_Transfers[1]->pending == false);
			Form::mainFrame->GetCStream_Percentage()->Show(Program::File_Transfers[2]->pending == false);

			Form::mainFrame->GetAStream_Text()->SetLabel(wxString::Format("File of %llu bytes from %s", Program::File_Transfers[0]->total_bytes,Program::File_Transfers[0]->Host->name));
			Form::mainFrame->GetBStream_Text()->SetLabel(wxString::Format("File of %llu bytes from %s", Program::File_Transfers[1]->total_bytes,Program::File_Transfers[1]->Host->name));
			Form::mainFrame->GetCStream_Text()->SetLabel(wxString::Format("File of %llu bytes from %s", Program::File_Transfers[2]->total_bytes,Program::File_Transfers[2]->Host->name));
			break;
		default:
			Form::mainFrame->GetCStream_No()->Show(false);
			Form::mainFrame->GetCStream_Text()->Show(false);
			Form::mainFrame->GetCStream_Yes()->Show(false);
			Form::mainFrame->GetCStream_Percentage()->Show(false);

			Form::mainFrame->GetAStream_No()->Show(false);
			Form::mainFrame->GetAStream_Text()->Show(false);
			Form::mainFrame->GetAStream_Yes()->Show(false);
			Form::mainFrame->GetAStream_Percentage()->Show(false);

			Form::mainFrame->GetBStream_No()->Show(false);
			Form::mainFrame->GetBStream_Text()->Show(false);
			Form::mainFrame->GetBStream_Yes()->Show(false);
			Form::mainFrame->GetBStream_Percentage()->Show(false);
			break;
		}
		if(Program::MyFile == nullptr) {
			Send_File->Enable(true);
			GetPick_File()->Enable(true);
		}
		Form::mainFrame->Layout();
	});
}
void MainFrame::OnExit(wxCommandEvent& event)
{
	if(Program::room_udp_socket != -1) {
		shutdown(Program::room_udp_socket,SHUT_RDWR);
		close(Program::room_udp_socket);
	}
	if(Program::room_socket != -1) {
		shutdown(Program::room_socket,SHUT_RDWR);
		close(Program::room_socket);
	}
	if(Program::network_socket != -1) {
		shutdown(Program::network_socket,SHUT_RDWR);
		close(Program::network_socket);
	}
	memset(&Program::room_address, 0, sizeof(struct sockaddr_in));

	wxUnusedVar(event);
	Close();
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
	wxUnusedVar(event);
	wxAboutDialogInfo info;
	info.SetCopyright(_("My MainFrame"));
	info.SetLicence(_("GPL v2 or later"));
	info.SetDescription(_("Short description goes here"));
	::wxAboutBox(info);
}

void MainFrame::ButtonOneClicked(wxCommandEvent & event)
{
	std::string a =Connection_TextBox->GetValue().ToStdString();
	if(a == "")
		return;
	Connection_Button->Enable(false);
	SendPackets::RequestRoomCreation(a);
}
void sendmsg()
{
	std::string msg =Form::mainFrame->GetMessage_Input()->GetValue().ToStdString();
	if(msg == "")
		return;
	Form::mainFrame->GetMessage_Input()->SetValue(wxString::Format(""));
	Form::mainFrame->GetMessage_Input()->SetFocus();
	SendPackets::SendMessageToRoom(msg);
}
void MainFrame::SendMessage_Clicked(wxCommandEvent & event)
{
	sendmsg();
}
void MainFrame::ConnectVoice_Clicked(wxCommandEvent & event)
{
	Form::mainFrame->CallAfter([]() {
		Form::mainFrame->GetVoice_Button()->Enable(false);
	});
	Voice_Button->SetLabelText(wxString::Format("%s",  ( Program::me->voice_active == true?"Connect Voice":"Disconnect Voice")));
	if(Program::me->voice_active == false) {
		Program::me->StartVoice();
	} else {
		SendPackets::OpenVoice(false);
	}

}


bool file_exists (std::string p)
{
	struct stat info;
	return stat(p.c_str(), &info) == 0;
}
void MainFrame::ConnectVideo_Clicked(wxCommandEvent & event)
{
	std::string device_path = "/dev/video0";
	if (file_exists("/dev/video0") == false) {
		if (file_exists("/dev/video1") == false) {
			std::cout << "No camera device found.\n";
			return;
		}
		else
		{
			device_path = "/dev/video1";
		}
	}
	V4L2Manager::cam_path = device_path;
	Form::mainFrame->CallAfter([]() {
		Form::mainFrame->Video_Button->Enable(false);
	});
	if(Program::me == nullptr)
		return;
	if(Program::me->video_active == false && Program::me->video_thread == nullptr) {
		Video_Button->SetLabelText(wxString::Format("Disconnect Video"));
		Program::me->StartVideo();
		return;
	} else if(Program::me->video_active == true && Program::me->video_thread != nullptr) {
		Video_Button->SetLabelText(wxString::Format("%s", "Connect Video"));
		SendPackets::OpenVideo(false,0,0);
	}

}
void MainFrame::User_ShowVideo(wxCommandEvent& event)
{

	if(Program::User_List.size() > 0) {
		const int selection = User_List->GetSelection();
		if(selection >= 0 && selection < (int)Program::User_List.size() ) {
			User *u = Program::User_List[selection];
			u->ClearVideoBuff();
		}
	}
}

void MainFrame::User_CloseVideo(wxCommandEvent& event)
{


}
void MainFrame::UserList_Select(wxCommandEvent& event)
{

}
void MainFrame::UserMenuOpen(wxMenuEvent& event)
{

}
void MainFrame::sdgsdg(wxCommandEvent& event)
{
}

unsigned long long getfilesize(std::string p)
{
	struct stat info;
	if (stat(p.c_str(), &info) == 0)
		return info.st_size;
	return 0;
}
void MainFrame::StreamFile_Clicked(wxCommandEvent & event)
{
	wxString p = GetPick_File()->GetPath();
	if(p == "")
		return;

	std::ifstream in(p, std::ifstream::ate | std::ifstream::binary);

	wxString t;

	unsigned long long file_size = getfilesize(std::string(p));
	if(file_size > 107374182400) {
		std::cout << "This file exceeds the 100 GB stream limit.\n";
		return;
	}
	if(file_size <= 0) {
		std::cout << "Invalid file.\n";
		return;
	}
	Send_File->Enable(false);
	GetPick_File()->Enable(false);
	t=GetPick_File()->GetFileName().GetFullName();
	SendPackets::InitiateFileStream(file_size, t.ToStdString());
}
void MainFrame::AcceptStreamA_Clicked(wxCommandEvent & event)
{
	SendPackets::UserFileStreamDecision(Program::File_Transfers[0]->Host->main_tcp_socket, true);
	AStream_Yes->Show(false);
	AStream_No->Show(false);
	Program::File_Transfers[0]->responded = true;
	Layout();
}
void MainFrame::DeclineStreamA_Clicked(wxCommandEvent & event)
{
	SendPackets::UserFileStreamDecision(Program::File_Transfers[0]->Host->main_tcp_socket, false);
	FileStream *f = Program::File_Transfers[0];
	MemoryManagement::Delete_Obj(f,2);
	Program::File_Transfers.erase(Program::File_Transfers.begin());
	RefreshPendingFiles();
}
void MainFrame::AcceptStreamB_Clicked(wxCommandEvent & event)
{
	SendPackets::UserFileStreamDecision(Program::File_Transfers[1]->Host->main_tcp_socket, true);
	BStream_Yes->Show(false);
	BStream_No->Show(false);
	Program::File_Transfers[1]->responded = true;
	Layout();
}
void MainFrame::DeclineStreamB_Clicked(wxCommandEvent & event)
{
	SendPackets::UserFileStreamDecision(Program::File_Transfers[1]->Host->main_tcp_socket, false);
	Program::File_Transfers.erase(Program::File_Transfers.begin());
	RefreshPendingFiles();
}
void MainFrame::AcceptStreamC_Clicked(wxCommandEvent & event)
{
	SendPackets::UserFileStreamDecision(Program::File_Transfers[2]->Host->main_tcp_socket, true);
	CStream_Yes->Show(false);
	CStream_No->Show(false);
	Program::File_Transfers[2]->responded = true;
	Layout();
}
void MainFrame::DeclineStreamC_Clicked(wxCommandEvent & event)
{
	SendPackets::UserFileStreamDecision(Program::File_Transfers[2]->Host->main_tcp_socket, false);
	Program::File_Transfers.erase(Program::File_Transfers.begin());
	RefreshPendingFiles();
}
void MainFrame::FileMenuOpen(wxMenuEvent& event)
{
	static std::vector<wxMenuItem*> temps;
	for(int i = 0; i < (int)temps.size(); ++i) {
		m_name6->Delete(temps[i]);
		temps.erase(temps.begin() +i);
	}
	if(Program::room_socket != -1) {
		wxMenuItem * m_showVid = new wxMenuItem(m_name6, wxID_ANY, _("Disconnect"), _("Leave current room."), wxITEM_NORMAL);
		temps.push_back(m_showVid);
		m_name6->Append(m_showVid);
		this->Connect(m_showVid->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainFrame::Click_Disconnect), NULL, this);
	}
}
void MainFrame::Click_Disconnect(wxCommandEvent& event)
{
	SendPackets::RequestLeaveRoom();
}
void MainFrame::ChatEnter(wxCommandEvent& event)
{
	sendmsg();
}
void MainFrame::OnMenuitem7MenuSelected(wxCommandEvent& event)
{
	exit(0);
}
/*
MIT License

Copyright (c) 2023 mdarnell321

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/