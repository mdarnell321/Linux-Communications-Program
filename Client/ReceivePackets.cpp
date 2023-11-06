#include "ReceivePackets.h"
#include "SendPackets.h"
#include "Program.h"
#include "MemoryManagement.h"

void ReceivePackets::ServerRequestName(boost::shared_ptr<Packet> packet, bool *error)
{
	if(packet->CheckFinalizers(error) == false) return;
	SendPackets::SendNameToServer(Program::Username);
}
void ReceivePackets::ReceiveMySocket(boost::shared_ptr<Packet> packet, bool *error)
{
	int a = packet->GetInt(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	Program::mysocket = a;

	Form::mainFrame->CallAfter([]() {
		Form::mainFrame->GetConnection_Button()->Enable(true);
	});
	std::cout << "Full connection to server." <<"\n";
}
void ReceivePackets::ReceiveMessage(boost::shared_ptr<Packet> packet, bool *error)
{
	std::string msg = packet->GetString(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	Form::mainFrame->CallAfter([msg]() {
		Form::mainFrame->GetListBox()->Append(msg);
		Form::mainFrame->GetListBox()->Select(Form::mainFrame->GetListBox()->GetCount()-1);
		Form::mainFrame->GetListBox()->DeselectAll();
	});
}
void ReceivePackets::ReceiveServerResponnse(boost::shared_ptr<Packet> packet, bool *error)
{
	int response = packet->GetInt(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	switch(response) {
	case 0: { //Activate Connection Button
		Form::mainFrame->CallAfter([]() {
			Form::mainFrame->GetConnection_Button()->Enable(true);
		});
	}
	break;
	default:
		break;
	}
}
void ReceivePackets::ReceiveServerMessage(boost::shared_ptr<Packet> packet, bool *error)
{
	std::string msg = packet->GetString(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	std::cout << msg << std::endl;
}
void ReceivePackets::RoomCreationSuccess(boost::shared_ptr<Packet> packet, bool *error)
{
	int port = packet->GetInt(error);
	if(*error == true)return;
	packet->GetInt(error);
	if(*error == true)return;
	//Connect to room udp
	if(packet->CheckFinalizers(error) == false) return;
	Program::room_udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(Program::room_udp_socket == -1) {
		std::cout << "Unable to connect to room udp (1)."<< std::endl;
		return;
	}
	//Connect to room tcp
	Program::room_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int val = 1;
	setsockopt(Program::room_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if(Program::room_socket == -1) {
		std::cout << "Unable to connect to room tcp (1)."<< std::endl;
		return;
	}
	Program::room_address.sin_family = AF_INET;
	Program::room_address.sin_port = htons(port);
	Program::room_address.sin_addr.s_addr = inet_addr(Program::HostAddress.c_str());
	memset(Program::room_address.sin_zero, 0, sizeof(Program::room_address.sin_zero));

	int connection_status = connect(Program::room_socket, (struct sockaddr*) &Program::room_address, sizeof(Program::room_address));
	if(connection_status == -1) {
		std::cout << "Unable to connect to room tcp (2) with socket of " << Program::room_socket << " and a port of " << port << std::endl;
		return;
	}
	//end
	Program::StartRoomThreads();

	//UI stuff
	Form::mainFrame->CallAfter([]() {
		Form::mainFrame->GetVideo_Button()->Enable(true);
		Form::mainFrame->GetVoice_Button()->Enable(true);
		Form::mainFrame->GetSend_File()->Enable(true);
		Form::mainFrame->GetPick_File()->Enable(true);
		Form::mainFrame->GetConnection_Button()->Hide();
		Form::mainFrame->GetConnection_TextBox()->Hide();
		Form::mainFrame->GetSend_Msg()->Show();
		Form::mainFrame->GetVoice_Button()->Show();
		Form::mainFrame->GetListBox()->Show();
		Form::mainFrame->GetVideo_Button()->Show();
		Form::mainFrame->GetMessage_Input()->Show();
		Form::mainFrame->GetUser_List()->Show();
		Form::mainFrame->GetVideo_Button()->Show();
		Form::mainFrame->GetVoice_Button()->Show();
		Form::mainFrame->GetSend_File()->Show();
		Form::mainFrame->GetPick_File()->Show();
		Form::mainFrame->GetMyFileGauge()->Show();
		Form::mainFrame->Layout();
	});

}

void ReceivePackets::AddUserToList(boost::shared_ptr<Packet> packet, bool *error)
{

	int a = packet->GetInt(error);
	if(*error == true)return;
	std::string b = packet->GetString(error);
	if(*error == true)return;
	bool c =packet->GetBool(error);
	if(*error == true)return;
	bool d =  packet->GetBool(error);
	if(*error == true)return;
	int w =  packet->GetInt(error);
	if(*error == true)return;
	int h =  packet->GetInt(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	User* u = new User(a,b,c,d);


	MemoryManagement::Add_Obj(u);
	Program::User_List.push_back(std::move(u));
	if(u->main_tcp_socket == Program::mysocket) {
		Program::me = u;
	} else if(d == true && w != 0 && h != 0) {
		u->StartVideo();
	}
	Form::mainFrame->CallAfter([u]() {
		Form::mainFrame->GetUser_List()->Append(u->name);
	});
}
void RemoveUser(int sock)
{
	User *u = Program::AsUser(sock);
	if(u == nullptr)
		return;
	u->termination_thread = new std::thread( [u]() {
		u->StopVideo();
		u->StopVoice();
		while(u->voice_thread != nullptr ||u->video_thread != nullptr);
		Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [u]()
		{
			//all threads terminated, handle this user deletion
			for(int i = 0; i < (int)Program::User_List.size(); ++i) {
				if(Program::User_List[i] == u) {

					Form::mainFrame->CallAfter([i]() {
						Form::mainFrame->GetUser_List()->Delete(i);
					}
					                          );
					Program::User_List.erase(Program::User_List.begin() + i);
					//next
					for(unsigned int i =0; i < Program::File_Transfers.size(); ++i) { //if file transfer already exists for this user then cancel
						if(Program::File_Transfers[i]->Host == u) {
							if(u->handle_file_thread != nullptr) {
								Program::File_Transfers[i]->cant_continue = true;
							} else {
								FileStream* f = Program::File_Transfers[i];
								Program::File_Transfers.erase(Program::File_Transfers.begin()+i);
								MemoryManagement::Delete_Obj(f,2);
								Form::mainFrame->RefreshPendingFiles();
							}
							break;
						}
					}
					u->TaskTo_UserUDPThread( std::packaged_task<void()> { [u]()
					{
						u->jobs_running = false;
					}
					                                                    });
				}
			}
		}
		                                                                       });

	});

}
void ReceivePackets::GoodToLeaveRoom(boost::shared_ptr<Packet> packet, bool *error)
{
	if(packet->CheckFinalizers(error) == false) return;
	if(Program::room_socket != -1) {
		shutdown(Program::room_socket,SHUT_RDWR);
		close(Program::room_socket);
		Program::room_socket = -1;
	}
	if(Program::room_udp_socket != -1) {
		shutdown(Program::room_udp_socket,SHUT_RDWR);
		close(Program::room_udp_socket);
		Program::room_udp_socket = -1;
	}

	Form::mainFrame->CallAfter([]() { // back to main menu
		Form::mainFrame->GetSend_File()->Enable(true);
		Form::mainFrame->GetPick_File()->Enable(true);
		Form::mainFrame->GetConnection_Button()->Show();
		Form::mainFrame->GetConnection_TextBox()->Show();
		Form::mainFrame->GetSend_Msg()->Hide();
		Form::mainFrame->GetMessage_Input()->Hide();
		Form::mainFrame->RefreshPendingFiles();
		Form::mainFrame->GetVideo_Button()->Hide();
		Form::mainFrame->GetVoice_Button()->Hide();
		Form::mainFrame->GetSend_File()->Hide();
		Form::mainFrame->GetPick_File()->Hide();
		Form::mainFrame->GetListBox()->Clear();
		Form::mainFrame->GetMyFileGauge()->Hide();
		Form::mainFrame->GetVideo_Button()->SetLabelText(wxString::Format("%s", "Connect Video"));
		Form::mainFrame->GetVoice_Button()->SetLabelText(wxString::Format("%s", "Connect Voice"));
		Form::mainFrame->Layout();
	});

	for(unsigned int i = 0; i < Program::User_List.size(); ++i) {
		User* u = Program::User_List[i];
		RemoveUser(u->main_tcp_socket);
	}
	if(Program::MyFile != nullptr)
		MemoryManagement::Delete_Obj(Program::MyFile,2);
	Program::MyFile = nullptr;
	Program::CanContinueFile = true;

	for(unsigned int i =0; i < Program::File_Transfers.size(); ++i) { //if file transfer already exists for this user then cancel
		if(Program::File_Transfers[i]->pending == false) {
			Program::File_Transfers[i]->cant_continue = true;
		} else {
			FileStream* f = Program::File_Transfers[i];
			Program::File_Transfers.erase(Program::File_Transfers.begin()+i);
			MemoryManagement::Delete_Obj(f,2);
			Form::mainFrame->RefreshPendingFiles();
		}
	}
}

void ReceivePackets::RemoveUserFromList(boost::shared_ptr<Packet> packet, bool *error)
{
	int sock = packet->GetInt(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	RemoveUser(sock);

}
void ReceivePackets::OpenOrCloseUserVoice(boost::shared_ptr<Packet> packet, bool *error)
{
	int sock = packet->GetInt(error);
	if(*error == true)return;
	bool open = packet->GetBool(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	User* u = Program::AsUser(sock);
	if(u != nullptr) {

		if(u->voice_active == false && open == true)
			u->StartVoice();

		if(u->voice_active == true && open == false)
			u->StopVoice();

		u->voice_active = open;
		if(u == Program::me) {
			if(open == false) {
				Form::mainFrame->CallAfter([]() {
					Form::mainFrame->GetVoice_Button()->Enable(true);
				});
			}
		}
	}
}
void ReceivePackets::OpenOrCloseUserVideo(boost::shared_ptr<Packet> packet, bool *error)
{
	int sock = packet->GetInt(error);
	if(*error == true)return;
	bool open = packet->GetBool(error);
	if(*error == true)return;
	int w = packet->GetInt(error);
	if(*error == true)return;
	int h = packet->GetInt(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	User* u = Program::AsUser(sock);
	if(u != nullptr) {
		if(u->video_active == false && open == true)
			u->StartVideo();
		if(u->video_active == true && open == false)
			u->StopVideo();
		u->video_active = open;
		if(u == Program::me) {
			if(open == false) {
				Form::mainFrame->CallAfter([]() {
					Form::mainFrame->GetVideo_Button()->Enable(true);
				});
			}
		}
	}
}
void ReceivePackets::IncomingFileStreamRequest(boost::shared_ptr<Packet> packet, bool *error)
{
	int socket = packet->GetInt(error);
	if(*error == true)return;
	std::string name = packet->GetString(error);
	if(*error == true)return;
	unsigned long long _bytes = packet->GetULL(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	User* u = Program::AsUser(socket);
	if( u == Program::me) {
		Program::MyFile = new FileStream{u, name, _bytes,0, std::vector<char>{}, true, false, false};
		MemoryManagement::Add_Obj(Program::MyFile);
		return;
	}
	if(u == nullptr)
		return;
	for(unsigned int i =0; i < Program::File_Transfers.size(); ++i) { //if file transfer already exists for this user then cancel
		if(Program::File_Transfers[i]->Host == u) {
			return;
		}
	}

	FileStream *f = new FileStream{u, name, _bytes, 0, std::vector<char>{}, true, false, false};
	MemoryManagement::Add_Obj(f);
	Program::File_Transfers.push_back(std::move(f));
	Form::mainFrame->RefreshPendingFiles();
}

void ReceivePackets::FileStreamWaiting(boost::shared_ptr<Packet> packet, bool *error)
{
	int socket = packet->GetInt(error);
	if(*error == true)return;
	bool status = packet->GetBool(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	User* u = Program::AsUser(socket);

	if(u == Program::me) {
		if(status == true) {
			u->handle_file_thread = new std::thread(&User::Handle_File, u, Program::MyFile);
			MemoryManagement::Add_Obj(u->handle_file_thread);
			Program::CanContinueFile = true;
		} else {
			if(Program::MyFile != nullptr)
				MemoryManagement::Delete_Obj(Program::MyFile,2);
			Program::MyFile = nullptr;
			Program::CanContinueFile = true;
			Form::mainFrame->CallAfter([]() {
				Form::mainFrame->GetSend_File()->Enable(true);
				Form::mainFrame->GetPick_File()->Enable(true);
			});

		}
		return;
	}

	if(u != nullptr) {
		for(unsigned int i = 0; i < Program::File_Transfers.size(); ++i) {
			if(Program::File_Transfers[i]->Host == u) {
				FileStream* f = Program::File_Transfers[i];
				if(status == true) {
					f->pending = false;
					u->handle_file_thread = new std::thread(&User::Handle_File, u, f);
					MemoryManagement::Add_Obj(u->handle_file_thread);
				} else {
					if(u->handle_file_thread != nullptr) {
						f->cant_continue = true;
					} else {
						Program::File_Transfers.erase(Program::File_Transfers.begin()+i);
						MemoryManagement::Delete_Obj(f,2);
					}
					break;
				}
			}
		}
		Form::mainFrame->RefreshPendingFiles();
	}
}
void ReceivePackets::FileSendCaughtUp(boost::shared_ptr<Packet> packet, bool *error)
{
	if(packet->CheckFinalizers(error) == false) return;
	Program::CanContinueFile = true;
}
void ReceivePackets::Pong(boost::shared_ptr<Packet> packet, bool *error)
{

	if(packet->CheckFinalizers(error) == false) return;
	Program::Ping_Timer = 0;
}
/*___________________________________________________________________________________________________________________________________
THIS SECTION IS FOR ROOM TCP & UDP
____________________________________________________________________________________________________________________________________*/
void ReceivePackets::MainTCPSocketRequest(boost::shared_ptr<Packet> packet, bool *error)
{
	if(packet->CheckFinalizers(error) == false) return;
	SendPackets::SendMyTCPSocket(Program::mysocket);
}
void ReceivePackets::ReceiveUserVoice(boost::shared_ptr<Packet> packet, bool *error)
{

	int socket_id = packet->GetInt(error);
	if(*error == true)return;
	audio_buffer_data data = packet->GetShortBufferData(error);
	if(*error == true)return;
	int timestamp = packet->GetInt(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	data.timestamp = timestamp;
	if(data.array == nullptr)
		return;
	User* u = Program::AsUser(socket_id);
	if(u != nullptr) {
		if( u != Program::me && u->voice_active == true) {
			u->TaskTo_UserVoiceThread( std::packaged_task<void()> { [data, u]()
			{
				u->voice_buffer.push_back(std::move(data));
			}
			                                                      });
		}
	}
}
void ReceivePackets::ReceiveUserVideo(boost::shared_ptr<Packet> packet, bool *error)
{
	int socket_id = packet->GetInt(error);
	if(*error == true)return;

	int frame_id = packet->GetInt(error);
	if(*error == true)return;

	int index = packet->GetInt(error);
	if(*error == true)return;

	int total_in_set = packet->GetInt(error);
	if(*error == true)return;

	boost_sharedptr_bytes data = packet->GetBytes(error);
	if(*error == true)return;

	int timestamp = packet->GetInt(error);
	if(*error == true)return;

	int width = packet->GetInt(error);
	if(*error == true)return;
	
	if(packet->CheckFinalizers(error) == false) return;
	if(data.array == nullptr )
		return;
	data.index = index;
	User* u = Program::AsUser(socket_id);
	if(u != nullptr && u != Program::me) {
		if( u->video_active == true) {
			u->TaskTo_UserVideoThread(  std::packaged_task<void()> { [socket_id,frame_id,index,total_in_set, u,data,timestamp, width]()
			{
				bool found = false;
				for(int i =0 ; i < (int)u->video_buffer.size(); ++i) {
					if(u->video_buffer[i].id == frame_id) {
						found = true;
						int chosen_index = 0;
						for(int ii = 0; ii < (int)u->video_buffer[i].buff.size(); ++ii) {
							if(index <= u->video_buffer[i].buff[ii].index) {
								break;
							}
							chosen_index = ii + 1;
						}

						u->video_buffer[i].buff.insert(u->video_buffer[i].buff.begin()+(chosen_index), std::move(data));
						//check if frame has been rebuilt
						if((int)u->video_buffer[i].buff.size() == (int)u->video_buffer[i].required_length) {
							u->frames_passed++;
							if(i != 0) {
								for(int ii = i - 1; ii >= 0; --ii) {
									if(u->video_buffer[ii].buff.size() != u->video_buffer[ii].required_length) {
										for(int iii = 0; iii < (int)u->video_buffer[ii].buff.size(); iii++) {
											u->video_buffer[ii].buff[iii].array = nullptr;
										}
										u->frames_lost++;
										u->video_buffer.erase(u->video_buffer.begin() + ii);
										--i;
									}
								}
							}
						}
						break;
					}
				}

				if(found == false) {

					std::vector<boost_sharedptr_bytes> temp;
					temp.push_back(std::move(data));
					//find where to insert element at
					int chosen_index = 0;
					for(int i = 0; i < (int)u->video_buffer.size(); ++i) {
						if(frame_id <= u->video_buffer[i].id) {
							break;
						}
						chosen_index = i + 1;
					}

					u->video_buffer.insert(u->video_buffer.begin() + (chosen_index ),std::move(fragmented_frame{temp, frame_id, total_in_set,timestamp, width}));
				}
				if(u->frames_passed > 30)
					u->percent_frames_lost = ((float)u->frames_lost / u->frames_passed)*100;
				if (u->percent_frames_lost >= 10) {
					u->lost_frames_threshold_reached = 8;
				}

				if (u->frames_passed > 200) {
					u->frames_passed = 0;
					u->frames_lost = 0;
				}
			}
			                                                       });
		}
	}
}
void ReceivePackets::FileStreamReceiveData(boost::shared_ptr<Packet> packet, bool *error)
{
	int socket = packet->GetInt(error);
	if(*error == true)return;
	boost_sharedptr_bytes data = packet->GetBytes(error);
	if(*error == true)return;
	if(packet->CheckFinalizers(error) == false) return;
	User* u = Program::AsUser(socket);
	if( u == Program::me)
		return;
	if(data.length <= 0) {
		for(unsigned int i = 0; i < Program::File_Transfers.size(); ++i) {
			if(Program::File_Transfers[i]->Host == u) {
				if(u->handle_file_thread != nullptr) {
					Program::File_Transfers[i]->cant_continue = true;
				} else {
					FileStream* f = Program::File_Transfers[i];
					Program::File_Transfers.erase(Program::File_Transfers.begin()+i);
					MemoryManagement::Delete_Obj(f,2);
				}
				return;
			}
		}
	}
	if(u != nullptr && data.array != nullptr && data.length > 0 && u != Program::me) {
		for(unsigned int i = 0; i < Program::File_Transfers.size(); ++i) {
			if(Program::File_Transfers[i]->Host == u) {
				Program::File_Transfers[i]->buff.insert(Program::File_Transfers[i]->buff.end(), std::move(data.array.get()), data.array.get() + data.length);
			}
		}
	}
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