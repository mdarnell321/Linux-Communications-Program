
#include "Program.h"
#include "SendPackets.h"
#include "Packet.h"
#include "ReceivePackets.h"
#include "V4L2Manager.h"
#include "MemoryManagement.h"


bool Program::running = false;
std::string Program::Username = "";
int Program::network_socket = -1;
int Program::room_socket = -1;
int Program::room_udp_socket = -1;
int Program::mysocket = -1;
std::thread* Program::UDPThread = nullptr;
std::thread* Program::TCPThread = nullptr;
std::thread* Program::Disconnect_Thread = nullptr;
struct sockaddr_in Program::room_address;
std::vector<User*> Program::User_List;
User* Program::me = nullptr;
std::vector<FileStream*> Program::File_Transfers;
std::string Program::HostAddress = "127.0.0.1";
bool Program::CanContinueFile = true;
FileStream* Program::MyFile = nullptr;
int Program::ms_Elapsed = 0;
int Program::Ping_Timer = 0;
int Program::fd = -1;
ALCdevice* Program::output_device = nullptr;
ALCcontext* Program::audio_context = nullptr;

void Program::StartThreads()
{
	new std::thread(&NetObjActions_Update);
	new std::thread(&One_Second_Update);
	new std::thread (&MemoryManagement::Update);
	new std::thread (&UpdateMSElapsed);
	new std::thread (&Listen_MainTCP);
}
void Program::StartRoomThreads()
{
	UDPThread = new std::thread(&Listen_RoomUDP);
	TCPThread = new std::thread(&Listen_RoomTCP);
	
	MemoryManagement::Add_Obj(UDPThread);
	MemoryManagement::Add_Obj(TCPThread);
}

User* Program::AsUser(int main_tcp_socket)
{
	for(int i = 0; i < (int)User_List.size(); i++) {
		if(User_List[i]->main_tcp_socket == main_tcp_socket) {
			return User_List[i];
		}
	}
	return nullptr;
}

std::deque<std::packaged_task<void()>> Program::ActionsOnNetObjThread;
std::mutex Program::netobj_mutex;


void Program::TaskTo_NetworkObjectThread(std::packaged_task<void()> a)// this is used if your trying to access a network obj (such as a user class).
{
	std::lock_guard<std::mutex> lock(netobj_mutex);
	ActionsOnNetObjThread.push_back(std::move(a));
}
void Program::UpdateMSElapsed()
{
	while(running == true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if(ms_Elapsed ++ > 2147483630)
			ms_Elapsed = 0;
	}
}
void Program::Listen_MainTCP()
{

	while(network_socket != -1 && running == true) {
		std::vector<unsigned char> packet_buffer;
		unsigned char received_bytes[12000];
		int count = recv(network_socket, received_bytes, 12000,0);
		if(count <= 0) {
			std::cout << "Server has been shutdown.";//
			break;
		}
		if(count > 12000)
			continue;
		packet_buffer.insert(packet_buffer.end(), std::move(received_bytes), received_bytes + count);
		if(packet_buffer[packet_buffer.size() - 1] == 230 && packet_buffer[packet_buffer.size() - 2] == 231 && packet_buffer[packet_buffer.size() - 3] == 232 && packet_buffer[packet_buffer.size() - 4] == 233) {
			const unsigned int packet_buff_len = packet_buffer.size();
			boost::shared_ptr<unsigned char[]>packet_bytearr (new unsigned char[packet_buffer.size()]);
			std::copy(packet_buffer.begin(), packet_buffer.end(), packet_bytearr.get());
			packet_buffer.clear();
			boost::shared_ptr<Packet>packet (new Packet(packet_bytearr, packet_buff_len));
			TaskTo_NetworkObjectThread( std::packaged_task<void()> { [packet, packet_buff_len, packet_bytearr]()
			{
				bool error = false;
				while(packet->_seeker < (packet_buff_len - 1)) {
					int packet_id = packet->GetInt(&error);
					if(error == true) return;

					switch(packet_id) {
					case 1:
						ReceivePackets::ServerRequestName(packet,&error);
						break;
					case 2:
						ReceivePackets::RoomCreationSuccess(packet,&error);
						break;
					case 3:
						ReceivePackets::ReceiveMySocket(packet,&error);
						break;
					case 4: 
						ReceivePackets::ReceiveMessage(packet,&error);
						break;
					case 6:
						ReceivePackets::AddUserToList(packet,&error);
						break;
					case 7:
						ReceivePackets::OpenOrCloseUserVoice(packet,&error);
						break;
					case 8:
						ReceivePackets::RemoveUserFromList(packet,&error);
						break;
					case 9:
						ReceivePackets::OpenOrCloseUserVideo(packet,&error);
						break;
					case 10:
						ReceivePackets::IncomingFileStreamRequest(packet,&error);
						break;
					case 11:
						ReceivePackets::FileStreamWaiting(packet,&error);
						break;
					case 12:
						ReceivePackets::FileSendCaughtUp(packet,&error);
						break;
					case 13:
						ReceivePackets::ReceiveServerResponnse(packet,&error);
						break;
					case 14:
						ReceivePackets::ReceiveServerMessage(packet,&error);
						break;
					case 15:
						ReceivePackets::GoodToLeaveRoom(packet,&error);
						break;
					case 16:
						ReceivePackets::Pong(packet,&error);
						break;
					default:
						return;
					}

					if(error == true) return;
				}
			}
			                                                                });
		}
	}
	if(room_udp_socket != -1) {
		shutdown(room_udp_socket,SHUT_RDWR);
		close(room_udp_socket);
	}
	if(room_socket != -1) {
		shutdown(room_socket,SHUT_RDWR);
		close(room_socket);
	}
	if(network_socket != -1) {
		shutdown(network_socket,SHUT_RDWR);
		close(network_socket);
	}
	memset(&room_address, 0, sizeof(struct sockaddr_in));

}
void Program::Listen_RoomTCP()
{
	std::vector<unsigned char> packet_buffer;
	while(room_socket != -1) {

		unsigned char received_bytes[64000];
		int count = recv(room_socket, received_bytes, 64000,0);


		if(count <= 0) {
			std::cout << "Room has been destroyed.\n";
			room_socket = -1;
			break;
		}
		packet_buffer.insert(packet_buffer.end(), std::move(received_bytes), received_bytes + count);
		if(packet_buffer[packet_buffer.size() - 1] == 230 && packet_buffer[packet_buffer.size() - 2] == 231 && packet_buffer[packet_buffer.size() - 3] == 232 && packet_buffer[packet_buffer.size() - 4] == 233) {
			const unsigned int packet_buff_len = packet_buffer.size();
			boost::shared_ptr<unsigned char[]> packet_bytearr(new unsigned char[packet_buffer.size()]);
			std::copy(packet_buffer.begin(), packet_buffer.end(), packet_bytearr.get());
			packet_buffer.clear();

			boost::shared_ptr<Packet>packet (new Packet(packet_bytearr, packet_buff_len));

			TaskTo_NetworkObjectThread( std::packaged_task<void()> { [packet, packet_buff_len, packet_bytearr]()
			{
				bool error = false;
				while(packet->_seeker < (packet_buff_len - 1)) {
					int packet_id = packet->GetInt(&error);
					if(error == true) return;
					if(packet_id != -301 && packet_id != -302) { // no identifiable packets left, we are done
						return;
					}
					switch(packet_id) {
					case -301:
						ReceivePackets::MainTCPSocketRequest(packet, &error);
						break;
					case -302:
						ReceivePackets::FileStreamReceiveData(packet, &error);
						break;
					default:
						return;
					}
					if(error == true) return;

				}


			}
			                                                                });
		}
	}
	MemoryManagement::Delete_Thread(std::move(TCPThread));
	TCPThread = nullptr;
}
void Program::Listen_RoomUDP()
{
	std::vector<unsigned char> packet_buffer;

	while(room_udp_socket != -1) {


		unsigned char received_bytes[100000];
		socklen_t length;
		unsigned int count = recvfrom(room_udp_socket, (unsigned char*)received_bytes, 100000,0, (struct sockaddr*)&room_address,&length);

		if(count <= 0) {
			room_udp_socket = -1;
			break;
		}
		packet_buffer.insert(packet_buffer.end(), std::move(received_bytes), received_bytes + count);
		if(packet_buffer[packet_buffer.size() - 1] == 230 && packet_buffer[packet_buffer.size() - 2] == 231 && packet_buffer[packet_buffer.size() - 3] == 232 && packet_buffer[packet_buffer.size() - 4] == 233) {
			const unsigned int packet_buff_len = packet_buffer.size();
			boost::shared_ptr<unsigned char[]> packet_bytearr(new unsigned char[packet_buffer.size()]);


			std::copy(packet_buffer.begin(), packet_buffer.end(), packet_bytearr.get());
			packet_buffer.clear();

			boost::shared_ptr<Packet>packet (new Packet(packet_bytearr, packet_buff_len));
			TaskTo_NetworkObjectThread( std::packaged_task<void()> { [packet, packet_buff_len, packet_bytearr]()
			{
				bool error = false;
				while(packet->_seeker < (packet_buff_len - 1)) {
					int packet_id = packet->GetInt(&error);
					if(error == true) return;

					if(packet_id != -2 && packet_id != -3&& packet_id != -4) { // no identifiable packets left, we are done
						return;
					}
					switch(packet_id) {
					case -2:
						ReceivePackets::ReceiveUserVoice(packet,&error);
						break;
					case -3: 
						ReceivePackets::ReceiveUserVideo(packet,&error);
						break;
					case -4:
						if(packet->CheckFinalizers(&error) == false) return;
						break;
					default:
						return;
					}
					if(error == true) return;

				}


			}
			                                                                });

		}
	}
	MemoryManagement::Delete_Thread(std::move(UDPThread));
	UDPThread = nullptr;
}
void Program::One_Second_Update()
{
	while (running== true) {

		TaskTo_NetworkObjectThread(std::packaged_task<void()> { []()
		{
			if (network_socket == -1)
				return;
			SendPackets::Ping();
			if(room_udp_socket != -1)
				SendPackets::UDPPing(mysocket);
			static bool lost = false;
			Ping_Timer++;
			if (Ping_Timer > 10) {
				//lost connection, handle disconnnect
				if (lost == false) {
					std::cout << "Lost connection to server.\n";
					Form::mainFrame->CallAfter([]() {
						Form::mainFrame->GetConnection_Button()->Enable(false);
						Form::mainFrame->GetConnection_Button()->Hide();
						Form::mainFrame->GetConnection_TextBox()->Hide();
						Form::mainFrame->GetSend_Msg()->Hide();
						Form::mainFrame->GetMessage_Input()->Hide();
						Form::mainFrame->RefreshPendingFiles();
						Form::mainFrame->GetVideo_Button()->Hide();
						Form::mainFrame->GetVoice_Button()->Hide();
						Form::mainFrame->GetSend_File()->Hide();
						Form::mainFrame->GetPick_File()->Hide();
						Form::mainFrame->GetMyFileGauge()->Hide();
						Form::mainFrame->GetListBox()->Hide();
						Form::mainFrame->GetUser_List()->Hide();
						Form::mainFrame->GetAStream_No()->Show(false);
						Form::mainFrame->GetAStream_Text()->Show(false);
						Form::mainFrame->GetAStream_Yes()->Show(false);
						Form::mainFrame->GetAStream_Percentage()->Show(false);
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
						Form::mainFrame->Layout();
					});
				}
				lost = true;
			}
			for (unsigned int i = 0; i < User_List.size(); ++i) {
				User* u = User_List[i];
				if(u->video_active == true && u == me) {
					u->vid_send_timer ++;
					if(u->vid_send_timer > 3) {
						if(me->video_active == true &&me->video_thread != nullptr) {
							Form::mainFrame->CallAfter([]() {
								Form::mainFrame->GetVideo_Button()->SetLabelText(wxString::Format("%s", "Connect Video"));
							});
							SendPackets::OpenVideo(false,0,0);
						}
					}
				}

				if (u->video_active == true && u->lost_frames_threshold_reached > 0) {
					u->TaskTo_UserVideoThread(std::packaged_task<void()> { [u]()
					{
						if (u->lost_frames_threshold_reached > 0) {
							u->lost_frames_threshold_reached--;
						}
					}
					                                                     });
				}
			}
		}
		                                                               });

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
void Program::NetObjActions_Update()
{

	while (running== true) {
		std::vector<std::packaged_task<void()>> temp;
		std::unique_lock<std::mutex> lock(netobj_mutex);
		while(ActionsOnNetObjThread.empty() == false) {
			temp.push_back(std::move(ActionsOnNetObjThread.front()));
			ActionsOnNetObjThread.pop_front();
		}
		lock.unlock();
		for(int i =0 ; i < temp.size(); ++i) {
			(temp[i])();
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
