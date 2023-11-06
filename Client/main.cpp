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

#include <wx/app.h>
#include <wx/event.h>
#include "MainFrame.h"
#include <wx/image.h>
#include "Packet.h"
#include "Program.h"
#include "Form.h"
#include "MemoryManagement.h"

#define Port 5000

// Define the MainApp
class MainApp : public wxApp
{
public:

	MainApp() {}
	virtual ~MainApp() {}

	virtual bool OnInit()
	{
		wxImage::AddHandler( new wxPNGHandler );
		wxImage::AddHandler( new wxJPEGHandler );

		Form::mainFrame = new MainFrame(NULL);
		Form::mainFrame->SetMinSize(wxSize(500,700));
		Form::mainFrame->SetSize(wxSize(500,700));
		Form::mainFrame->Layout();

		Form::mainFrame->CallAfter([]() {
			Form::mainFrame->GetConnection_Button()->Enable(false);
			Form::mainFrame->GetConnection_Button()->Show();
			Form::mainFrame->GetConnection_TextBox()->Show();
			Form::mainFrame->GetSend_Msg()->Hide();
			Form::mainFrame->GetMessage_Input()->Hide();
			Form::mainFrame->RefreshPendingFiles();
			Form::mainFrame->GetVideo_Button()->Hide();
			Form::mainFrame->GetVoice_Button()->Hide();
			Form::mainFrame->GetSend_File()->Hide();
			Form::mainFrame->GetPick_File()->Hide();
			Form::mainFrame->GetMyFileGauge()->Hide();
			Form::mainFrame->Layout();
		});

		SetTopWindow(Form::mainFrame);


		std::cout << "Please enter a username: ";
		std::cin >> Program::Username;
		if(Program::Username == "")
			exit(0);
		Program::running = true;
	
		struct sockaddr_in server_address;

		//Connect to server tcp
		Program::network_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if(Program::network_socket == -1) {
			std::cout << "Unable to connect to server.";
			return 0;
		}
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(Port);
		server_address.sin_addr.s_addr = inet_addr(Program::HostAddress.c_str());
		memset(server_address.sin_zero, 0, sizeof(server_address.sin_zero));
		int val = 1;
		setsockopt(Program::network_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
		int connection_status = connect(Program::network_socket, (struct sockaddr*) &server_address, sizeof(server_address));
		if(connection_status == -1) {
			std::cout << "Unable to connect to server.";
			return 0;
		}
		//end

		Program::StartThreads();
		return GetTopWindow()->Show();

	}
};

DECLARE_APP(MainApp)
IMPLEMENT_APP(MainApp)

