#include "Room.h"
#include "Program.h"
#include "SendPackets.h"

Room::Room(int p)
{

	alive = true;
	struct sockaddr_in server_address;
	port = p;


	//Start Room TCP
	tcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(p);
	server_address.sin_addr.s_addr = INADDR_ANY;
	memset(server_address.sin_zero, 0, sizeof(server_address.sin_zero));
	int val = 1;
	setsockopt( tcp_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	bind(tcp_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
	listen(tcp_socket, 4);
	//Start Room UDP
	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	int val_B = 1;
	setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, &val_B, sizeof(val_B));
	int rec_val = 64000;
	setsockopt(udp_socket, SOL_SOCKET, SO_RCVBUF, &rec_val, sizeof(rec_val));
	bind(udp_socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
	//
	tcp_listen_thread = new std::thread(&Room::TCP_Listen, this);
	udp_listen_thread = new std::thread(&Room::UDP_Listen, this);
	UDP_task_thread = new std::thread(&Room::ActionsOnUDPThread_Update, this);
	File_task_thread = new std::thread(&Room::File_Tasks_Update, this);
	MemoryManagement::Add_Obj(tcp_listen_thread);
	MemoryManagement::Add_Obj(udp_listen_thread);
	MemoryManagement::Add_Obj(UDP_task_thread);
	MemoryManagement::Add_Obj(File_task_thread);
}

ENDPOINTS *Room::FindUDPEndPoint(struct sockaddr_in endpoint)
{
	std::string ep_string = std::string(inet_ntoa(endpoint.sin_addr)) + ":" + std::to_string(endpoint.sin_port);
	for(unsigned int i = 0; i < endpoints.size(); ++i) {
		std::string iteration_ep_string = std::string(inet_ntoa(endpoints[i].ep.sin_addr)) + ":" + std::to_string(endpoints[i].ep.sin_port);
		if (iteration_ep_string == ep_string) {
			return &endpoints[i];
		}
	}
	ENDPOINTS e_p = ENDPOINTS{endpoint};
	e_p.public_id = -1;
	endpoints.push_back(std::move(e_p));

	return &endpoints[endpoints.size() - 1];
}

void Room::TCP_Listen()   //For accepting new clients only. Owned by the room obj
{
	std::thread* thisthread = tcp_listen_thread;
	while(Program::running == true && alive == true) {
		if(thisthread == nullptr && tcp_listen_thread != nullptr)
			thisthread = tcp_listen_thread;
		struct sockaddr_in client_address;
		socklen_t address_length = sizeof(struct sockaddr_in);
		int client_socket = accept(tcp_socket, (struct sockaddr *)&client_address, &address_length); // incoming user in room
		if(client_socket == -1) {
			continue;
		} else {

			Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [client_socket, this]()
			{
				UnassignedUser* uu = new UnassignedUser(client_socket);
				MemoryManagement::Add_Obj(uu);
				UnassignedUser_List.push_back(std::move(uu));
				SendPackets::RequestUserMainSocket(client_socket);
			}
			                                                                });
		}
	}
	while(ObjsToDelete_A.size() > 0);
	Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [this]()
	{
		for(int i = 0; i < (int)Program::Room_List.size(); ++i) {
			if(Program::Room_List[i] == this)
				Program::Room_List.erase(Program::Room_List.begin() + i--);
		}
	}
	                                                                });

	EndThread = new std::thread( [this]() {
		if(tcp_socket != -1) {
			shutdown(tcp_socket,SHUT_RDWR);
			close(tcp_socket);
		}
		while(udp_listen_thread != nullptr || UDP_task_thread != nullptr|| File_task_thread != nullptr);
		MemoryManagement::Delete_Obj(this,2);

	});
	MemoryManagement::Delete_Thread(std::move(thisthread));
}



void Room::TaskTo_FileThread(std::packaged_task<void()> a)
{
	std::lock_guard<std::mutex> lock(file_tasks_mutex);
	file_tasks.push_back(std::move(a));
}

void Room::File_Tasks_Update()
{
	std::thread* thisthread = File_task_thread;
	while (Program::running == true && alive == true) {
		if(thisthread == nullptr && File_task_thread != nullptr)
			thisthread = File_task_thread;
		std::vector<std::packaged_task<void()>> _temp;
		std::unique_lock<std::mutex> lock(file_tasks_mutex);
		while(file_tasks.empty() == false) {
			_temp.push_back(std::move(file_tasks.front()));
			file_tasks.pop_front();
		}
		lock.unlock();
		for(int i =0 ; i < _temp.size(); ++i) {
			(_temp[i])();
		}
	}
	Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [ this]()
	{
		File_task_thread = nullptr;
	}
	                                                                });
	MemoryManagement::Delete_Thread(std::move(thisthread));
}



void Room::TaskTo_UDPThread(std::packaged_task<void()> a)
{
	std::lock_guard<std::mutex> lock(ActionsOnUserUDPThread_mutex);
	ActionsOnUserUDPThread.push_back(std::move(a));
}

void Room::ActionsOnUDPThread_Update()
{
	std::thread* thisthread = UDP_task_thread;
	while (Program::running == true && alive == true) {
		if(thisthread == nullptr && UDP_task_thread != nullptr)
			thisthread = UDP_task_thread;

		std::vector<std::packaged_task<void()>> _temp;
		std::unique_lock<std::mutex> lock(ActionsOnUserUDPThread_mutex);
		while(ActionsOnUserUDPThread.empty() == false) {
			_temp.push_back(std::move(ActionsOnUserUDPThread.front()));
			ActionsOnUserUDPThread.pop_front();
		}
		lock.unlock();
		for(int i =0 ; i < _temp.size(); ++i) {
			(_temp[i])();
			if(File_Transfers.size() == 0) {
				for(unsigned int ii = 0; ii < ObjsToDelete_A.size(); ++ii) {
					void* ptr = ObjsToDelete_A[ii].ptr;
					int c = ObjsToDelete_A[ii].the_class;
					ObjsToDelete_A.erase(ObjsToDelete_A.begin()+ ii--);
					MemoryManagement::Delete_Obj(ptr,c);
				}
			}
		}
		if(File_Transfers.size() == 0) {
			for(unsigned int i = 0; i < ObjsToDelete_A.size(); ++i) {
				void* ptr = ObjsToDelete_A[i].ptr;
				int c = ObjsToDelete_A[i].the_class;

				ObjsToDelete_A.erase(ObjsToDelete_A.begin()+ i--);
				MemoryManagement::Delete_Obj(ptr,c);
			}
		}
	}
	//Destroy this

	for(unsigned int i = 0; i < ObjsToDelete_A.size(); ++i) {
		void* ptr = ObjsToDelete_A[i].ptr;
		int c = ObjsToDelete_A[i].the_class;
		ObjsToDelete_A.erase(ObjsToDelete_A.begin()+ i--);
		MemoryManagement::Delete_Obj(ptr,c);
	}
	Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [ this]()
	{
		UDP_task_thread = nullptr;
	}
	                                                                });
	MemoryManagement::Delete_Thread(std::move(thisthread));
}
std::string AddressString(struct sockaddr_in addr)
{
	return  std::string(inet_ntoa(addr.sin_addr)) + ":" + std::to_string( addr.sin_port);
}
void AssignEndpoint(Room* r, ENDPOINTS *e, int public_id)
{
	bool foundepublicid = false;
	for(unsigned int i = 0 ; i < r->endpoints.size(); ++i) {
		if(r->endpoints[i].public_id == public_id)
			foundepublicid = true;
	}
	if(foundepublicid == false && e->public_id == -1)
		e->public_id = public_id;

}
void Room::UDP_Listen()
{

	std::thread* thisthread = udp_listen_thread;
	while(Program::running == true && alive == true) {
		if(thisthread == nullptr && udp_listen_thread != nullptr)
			thisthread = udp_listen_thread;
		unsigned char received_bytes[100000];
		struct sockaddr_in client_address;
		socklen_t length;
		unsigned int count = recvfrom(udp_socket, received_bytes, 100000, 0, (struct sockaddr*)&client_address, &length);

		if(count <= 0 ||count >= 100000)
			continue;
		ENDPOINTS* e = FindUDPEndPoint(client_address);

		e->data.insert(e->data.end(), std::move(received_bytes), received_bytes + count);
		e->timestamp = std::chrono::system_clock::now();
		if(e->data[e->data.size() - 1] == 230 && e->data[e->data.size() - 2] == 231 && e->data[e->data.size() - 3] == 232 && e->data[e->data.size() - 4] == 233) {

			const unsigned int packet_buff_len = e->data.size();
			boost::shared_ptr<unsigned char[]> packet_bytearr(new unsigned char[e->data.size()]);
			std::copy(e->data.begin(), e->data.end(), packet_bytearr.get());
			e->data.clear();
			boost::shared_ptr<Packet> packet (new Packet(packet_bytearr, packet_buff_len));

			TaskTo_UDPThread( std::packaged_task<void()> { [e,packet,client_address,packet_buff_len, this]()
			{
				bool error = false;
				while(packet->_seeker < (packet_buff_len - 1)) {
					int packet_id = packet->GetInt(&error);
					if(error == true) return;
					if(packet_id != -1 &&packet_id != -2 && packet_id != -3 && packet_id != -4)
						return;
					switch(packet_id) {
					case -2: { // Receive voice data

						int socket = packet->GetInt(&error);
						if(error == true) return;
						audio_buffer_data data = packet->GetShortBufferData(&error);
						if(error == true) return;
						int timestamp = packet->GetInt(&error);
						if(error == true) return;
						if(packet->CheckFinalizers(&error) == false) return;
						if(e->public_id == -1)
							AssignEndpoint(this, e, socket);
						if(e->public_id != socket)
							break;
						SendPackets::SendVoiceBufferData( socket,data,timestamp);
					}
					break;
					case -3: { // Receive Video data
						int socket = packet->GetInt(&error);
						if(error == true) return;
						int frameid = packet->GetInt(&error);
						if(error == true) return;
						int index = packet->GetInt(&error);
						if(error == true) return;
						int total_in_set = packet->GetInt(&error);
						if(error == true) return;
						boost_sharedptr_bytes data = packet->GetBytes(&error);
						if(error == true) return;
						int timestamp = packet->GetInt(&error);
						if(error == true) return;
						int width = packet->GetInt(&error);
						if(error == true) return;
						if(packet->CheckFinalizers(&error) == false) return;
						if(e->public_id == -1)
							AssignEndpoint(this, e, socket);
						if(e->public_id != socket)
							break;

						SendPackets::SendVideoBufferData( socket,  data,  frameid,  index,  total_in_set,timestamp, width);
					}
					break;
					case -4: { // UDP Ping
						int socket = packet->GetInt(&error);
						if(error == true) return;
						if(packet->CheckFinalizers(&error) == false) return;
						if(e->public_id == -1)
							AssignEndpoint(this, e, socket);
						if(e->public_id != socket)
							break;
						SendPackets::UDPPong(udp_socket,client_address);
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
		for(unsigned int ii = 0; ii < endpoints.size(); ++ii) {
			std::chrono::system_clock::time_point cur_time = std::chrono::system_clock::now();
			int diff = std::chrono::duration_cast<std::chrono::seconds>(cur_time-endpoints[ii].timestamp).count();
			if(diff > int(5)) {
				endpoints.erase(endpoints.begin()+ii--);
				continue;
			}
		}
	}

	//Destroy this
	Program::TaskTo_NetworkObjectThread( std::packaged_task<void()> { [ this]()
	{
		if(udp_socket != -1) {
			shutdown(udp_socket,SHUT_RDWR);
			close(udp_socket);
		}
		udp_socket = -1;
		udp_listen_thread = nullptr;
	}
	                                                                });
	MemoryManagement::Delete_Thread(std::move(thisthread));
}
Room::~Room()
{
	EndThread->detach();
	delete EndThread;
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
