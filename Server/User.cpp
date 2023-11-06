#include "User.h"
#include "ReceivePackets.h"

void User::TCP_Listen()
{
	std::thread* thisthread = tcp_listen_thread;

	std::vector<unsigned char> packet_buffer;
	while(Program::running == true && active_on_serv == true) {
		if(thisthread == nullptr && tcp_listen_thread != nullptr)
			thisthread = tcp_listen_thread;

		//User Receive
		unsigned char received_bytes[12000] ;
		unsigned int count = recv(tcp_socket, received_bytes, 12000,0);

		if(count  <= 0) { // this user left room
			Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [this]()
			{
				for(int i = 0; i < (int)Program::User_List.size(); ++i) {
					if(Program::User_List[i] == this) {
						Program::User_List.erase(Program::User_List.begin() + i);
						break;
					}
				}
			}
			                                                                });
			shutdown(tcp_socket, SHUT_RDWR);
			close(tcp_socket);
			active_on_serv = false;
			break;
		}
		if(count >= 12000) {
			continue;
		}
		packet_buffer.insert(packet_buffer.end(), std::move(received_bytes), received_bytes + count);
		if(packet_buffer[packet_buffer.size() - 1] == 230 && packet_buffer[packet_buffer.size() - 2] == 231 && packet_buffer[packet_buffer.size() - 3] == 232 && packet_buffer[packet_buffer.size() - 4] == 233) {
			const unsigned int packet_buff_len = packet_buffer.size();
			boost::shared_ptr<unsigned char[]> packet_bytearr(new unsigned char[packet_buffer.size()]);
			std::copy(packet_buffer.begin(), packet_buffer.end(), packet_bytearr.get());
			packet_buffer.clear();
			int sock = tcp_socket;
			boost::shared_ptr<Packet> packet (new Packet(packet_bytearr, packet_buff_len));

			Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [ packet_buff_len, sock,packet]()
			{

				bool error = false;
				while(packet->_seeker < (packet_buff_len - 1)) {
					int packet_id = packet->GetInt(&error);
					if(error == true) return;
					switch(packet_id) {
					case 1:
						ReceivePackets::ReceiveUserName(packet, sock, &error);
						break;
					case 2:
						ReceivePackets::CreateRoom(packet, sock, &error);
						break;
					case 3:
						ReceivePackets::ReceiveRoomMessage(packet, sock, &error);
						break;
					case 4:
						ReceivePackets::OpenUserVoice(packet, sock, &error);
						break;
					case 5:
						ReceivePackets::OpenUserVideo(packet, sock, &error);
						break;
					case 6:
						ReceivePackets::InitiateFileStream(packet, sock, &error);
						break;
					case 7:
						ReceivePackets::UserFileDecision(packet, sock, &error);
						break;
					case 8:
						ReceivePackets::ClientRequestLeaveRoom(packet, sock, &error);
						break;
					case 9:
						ReceivePackets::TerminateFileStream(packet, sock, &error);
						break;
					case 10:
						ReceivePackets::Ping(packet, sock, &error);
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

	((RoomUser*)this)->EndThread = new std::thread( [this]() {
		while(((RoomUser*)this)->room_tcp_socket != -1);
		if(tcp_socket != -1) {
			shutdown(tcp_socket,SHUT_RDWR);
			close(tcp_socket);
		}
		Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [this]()
		{
			Room* r = Program::FindRoomWithSocket(tcp_socket);
			if(r != nullptr) {
				r->ObjsToDelete_A.push_back( std::move(DeletionObject{ this,0,0,1 }));
			} else {
				MemoryManagement::Delete_Obj((RoomUser*)this,1);
			}
		}
		                                                                });
	}
	                                              );
	MemoryManagement::Delete_Thread(std::move(thisthread));
}
RoomUser::~RoomUser()
{
	EndThread->detach();
	delete EndThread;
}

User::~User()
{
	std::cout << name << " with socket " << tcp_socket << " has been destroyed.\n";
}
User::User(std::string n, int s)
{
	name = n;
	tcp_socket = s;
	active_on_serv = true;
	tcp_listen_thread = new std::thread(&User::TCP_Listen, this);
	MemoryManagement::Add_Obj(tcp_listen_thread);
	std::cout << name << " with socket " << tcp_socket << " has been created.\n";
}

UnassignedUser::UnassignedUser(int s)
{
	active = true;
	socket = s;
	tcp_listen_thread = new std::thread(&UnassignedUser::TCP_Listen, this);
	MemoryManagement::Add_Obj(tcp_listen_thread);
}

UnassignedUser::~UnassignedUser()
{

}
RoomUser::RoomUser(std::string a, int b) : User(a,b)
{

}
void RoomUser::Room_TCP_Listen()
{

	std::thread* thisthread = room_tcp_listen_thread;
	std::vector<unsigned char> packet_buffer;
	while(Program::running == true && active == true) {
		if(thisthread == nullptr && room_tcp_listen_thread != nullptr)
			thisthread = room_tcp_listen_thread;
		unsigned char received_bytes[64000] ;

		unsigned int count = recv(room_tcp_socket, received_bytes, 64000,0);

		if(count  <= 0) { // this user left room
			active = false;
			shutdown(room_tcp_socket,SHUT_RDWR);
			close(room_tcp_socket);
			break;
		}
		if(count >= 64000)
			continue;

		packet_buffer.insert(packet_buffer.end(), std::move(received_bytes), received_bytes + count);

		if(packet_buffer[packet_buffer.size() - 1] == 230 && packet_buffer[packet_buffer.size() - 2] == 231 && packet_buffer[packet_buffer.size() - 3] == 232 && packet_buffer[packet_buffer.size() - 4] == 233) {
			const unsigned int packet_buff_len = packet_buffer.size();
			boost::shared_ptr<unsigned char[]> packet_bytearr(new unsigned char[packet_buffer.size()]);
			std::copy(packet_buffer.begin(), packet_buffer.end(), packet_bytearr.get());
			packet_buffer.clear();
			boost::shared_ptr<Packet> packet (new Packet(packet_bytearr, packet_buff_len));

			int sock = room_tcp_socket;
			Room* r = Program::FindRoomWithSocket(tcp_socket);
			if(r == nullptr)
				break;
			r->TaskTo_FileThread( std::packaged_task<void()> { [packet, packet_buff_len, sock]()
			{
				bool error = false;
				while(packet->_seeker < (packet_buff_len - 1)) {
					int packet_id = packet->GetInt(&error);
					if(error == true) return;
					if(packet_id != -302) // no identifiable packets left, we are done
						return;
					switch(packet_id) {
					case -302:
						ReceivePackets::HandleFileSection(packet, sock, &error);
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
	//Destroy this
	voice_active = false;
	video_active = false;
	Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [this]()
	{
		Room* r = Program::FindRoomWithSocket(room_tcp_socket);
		if(r != nullptr) {
			for(int i = 0; i < (int)r->User_List.size(); ++i) {
				if(r->User_List[i] == this) {
					SendPackets::SendUserLeftRoom(r->User_List[i]->tcp_socket, r);
					for(unsigned int ii = 0; ii < r->File_Transfers.size(); ++ii) {
						FileStream* f = r->File_Transfers[ii];

						for(unsigned int iii = 0; iii < f->Recipients.size(); ++iii) {
							if(f->Recipients[iii] == r->User_List[i]) {
								f->Recipients.erase(f->Recipients.begin() + iii--);
								if(f->Recipients.size() == 0 || f->Host == this) {
									r->File_Transfers.erase(r->File_Transfers.begin()+ii--);
									r->ObjsToDelete_A.push_back(std::move(DeletionObject{f,0,0,0}));
									break;
								}
							}
						}
						if(f->Host == this)
							f->Host = nullptr;
					}

					r->User_List.erase(r->User_List.begin() + i--);
					std::cout << "User " << name << " has left room\n";
				}
			}
			if(r->UnassignedUser_List.size() == 0 && r->User_List.size() == 0) {
				r->alive = false;
				std::cout << "Room destroyed.\n";
				shutdown(r->tcp_socket,SHUT_RDWR);
				close(r->tcp_socket);
				shutdown(r->udp_socket, SHUT_RDWR);
				close(r->udp_socket);
			}

		}

		if(room_tcp_socket != -1) {
			shutdown(room_tcp_socket,SHUT_RDWR);
			close(room_tcp_socket);
		}
		room_tcp_socket = -1;
	}
	                                                                });
	MemoryManagement::Delete_Thread(std::move(thisthread));

}

void UnassignedUser::TCP_Listen()
{

	std::thread* thisthread = tcp_listen_thread;
	std::vector<unsigned char> packet_buffer;
	bool received_packet = false;
	while(Program::running == true && active == true) {
		int iteration = 0;
		while(received_packet == true && active == true ) {
			iteration ++;
			if(iteration > 10)
				active = false;
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
		if(active == false) {
			break;
		}

		if(thisthread == nullptr && tcp_listen_thread != nullptr)
			thisthread = tcp_listen_thread;
		unsigned char received_bytes[12000] ;

		unsigned int count = recv(socket, received_bytes, 12000,0);

		if(count  <= 0) { // this user left room
			active = false;
			shutdown(socket,SHUT_RDWR);
			close(socket);
			break;
		}
		if(count >= 12000) {
			continue;
		}

		packet_buffer.insert(packet_buffer.end(), std::move(received_bytes), received_bytes + count);

		if(packet_buffer[packet_buffer.size() - 1] == 230 && packet_buffer[packet_buffer.size() - 2] == 231 && packet_buffer[packet_buffer.size() - 3] == 232 && packet_buffer[packet_buffer.size() - 4] == 233) {
			const unsigned int packet_buff_len = packet_buffer.size();
			boost::shared_ptr<unsigned char[]> packet_bytearr(new unsigned char[packet_buffer.size()]);
			std::copy(packet_buffer.begin(), packet_buffer.end(), packet_bytearr.get());
			packet_buffer.clear();
			boost::shared_ptr<Packet> packet (new Packet(packet_bytearr, packet_buff_len));
			received_packet = true;
			Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [ packet_buff_len,this,packet]()
			{
				bool error = false;
				while(packet->_seeker < (packet_buff_len - 1)) {
					int packet_id = packet->GetInt(&error);
					if(error == true) return;
					if(packet_id != -1)
						return;
					switch(packet_id) {
					case -1: {
						//create an actual user object now
						int Main_Sock =packet->GetInt(&error);
						if(error == true) return;
						if(packet->CheckFinalizers(&error) == false) return;
						RoomUser* main_u = (RoomUser*)Program::AsUser(Main_Sock);

						if(main_u != nullptr) {
							Room* r = Program::FindRoomWithSocket(socket);
							if(r != nullptr) {
								r->User_List.push_back(std::move(main_u));
								SendPackets::SendNewUser(main_u, r);
								for(unsigned int i = 0; i < r->User_List.size(); ++i) {
									if(r->User_List[i] != main_u)
										SendPackets::SendNewUser(r->User_List[i], main_u->tcp_socket);
								}
							}
							main_u->active = true;
							main_u->room_tcp_socket = socket;
							main_u->room_tcp_listen_thread = new std::thread(&RoomUser::Room_TCP_Listen, main_u);
							MemoryManagement::Add_Obj(main_u->room_tcp_listen_thread);
							main_u->voice_active = false;
							active = false;
						}
					}
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
//Destroy this

	Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [this]()
	{
		Room* r = Program::FindRoomWithSocket(socket);
		if(r != nullptr) {

			for(int i = 0; i < (int)r->UnassignedUser_List.size(); ++i) {
				if(r->UnassignedUser_List[i] == this) {
					r->UnassignedUser_List.erase(r->UnassignedUser_List.begin() + i--);
				}
			}
			if(r->UnassignedUser_List.size() == 0 && r->User_List.size() == 0) {
				r->alive = false;
				shutdown(r->tcp_socket, SHUT_RDWR);
				close(r->tcp_socket);
				shutdown(r->udp_socket, SHUT_RDWR);
				close(r->udp_socket);
			}
		}
		MemoryManagement::Delete_Obj(this,3);
	}
	                                                                });
	MemoryManagement::Delete_Thread(std::move(thisthread));
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