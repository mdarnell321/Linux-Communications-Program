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

#include "Program.h"
#include "SendPackets.h"
#include "Packet.h"

#define Port 5000

int main()
{
	Program::running = true;
	Program::StartThreads();
	struct sockaddr_in server_address;

	//Start TCP
	Program::Server_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(Port);
	server_address.sin_addr.s_addr = INADDR_ANY;
	memset(server_address.sin_zero, 0, sizeof(server_address.sin_zero));
	int val = 1;
	setsockopt(Program::Server_Socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	bind(Program::Server_Socket, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
	listen(Program::Server_Socket, 256);
	//
	std::cout << "Server started.\n";
	while(Program::running);


	return 0;
}
