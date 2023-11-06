#ifndef RECEIVEPACKETS_H
#define RECEIVEPACKETS_H

#include "Packet.h"
#include "SendPackets.h"

class ReceivePackets
{
    public:
    static void ReceiveUserName(boost::shared_ptr<Packet>, int, bool*);
    static void CreateRoom(boost::shared_ptr<Packet>, int, bool*);
    static void ReceiveRoomMessage(boost::shared_ptr<Packet>, int, bool*);
	static void OpenUserVoice(boost::shared_ptr<Packet>, int, bool*);
	static void OpenUserVideo(boost::shared_ptr<Packet>, int, bool*);
	static void InitiateFileStream(boost::shared_ptr<Packet>, int, bool*);
	static void HandleFileSection(boost::shared_ptr<Packet>, int, bool*);
	static void UserFileDecision(boost::shared_ptr<Packet>, int, bool*);
	static void ClientRequestLeaveRoom(boost::shared_ptr<Packet>, int, bool*);
	static void TerminateFileStream(boost::shared_ptr<Packet>, int, bool *);
	static void Ping(boost::shared_ptr<Packet>, int, bool *);
};

#endif

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