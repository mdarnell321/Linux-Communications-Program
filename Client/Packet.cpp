#include "Packet.h"

void Packet::Finalize()
{
	Bytes_To_Send.push_back(233);
	Bytes_To_Send.push_back(232);
	Bytes_To_Send.push_back(231);
	Bytes_To_Send.push_back(230);
}
void Packet::Write(boost_sharedptr_bytes data)
{
	Write((int)data.length);
	Bytes_To_Send.insert(Bytes_To_Send.end(), data.array.get(), data.array.get() + data.length);
}
boost_sharedptr_bytes Packet::GetBytes(bool *error)
{
	int length = GetInt(error);
	if(length <= 0) {
		if(length < 0)
			*error = true;
		return boost_sharedptr_bytes{nullptr, 0};
	}
	if(_seeker + length - 1 >= Receive_Length) {
		//Index out of range
		*error = true;
		return boost_sharedptr_bytes{nullptr, 0};
	}
	boost::shared_ptr<unsigned char[]> data(new unsigned char[length]);
	std::copy(Received_Bytes.get() + _seeker, Received_Bytes.get() + _seeker + length, data.get());
	_seeker += length;
	
	return boost_sharedptr_bytes{data, length};
}

void Packet::Write(audio_buffer_data data)
{
	Write((int)data.length );
	unsigned char* casted = (unsigned char *)data.array.get();
	Bytes_To_Send.insert(Bytes_To_Send.end(), casted, casted+ data.length * 2);
}
audio_buffer_data Packet::GetShortBufferData(bool *error)
{
	int length = GetInt(error);
	if((length*2) <= 0) {
		if((length*2) < 0)
			*error = true;
		return audio_buffer_data{nullptr, 0};
	}
	if(_seeker + (length*2) - 1 >= Receive_Length) {
		//Index out of range
		*error = true;
		return audio_buffer_data{nullptr, 0};
	}
	boost::shared_ptr<short[]> data(new short[length]);
	std::copy(Received_Bytes.get() + _seeker, Received_Bytes.get() + _seeker + length * 2, (unsigned char*)data.get());
	_seeker += (length*2);
	
	return audio_buffer_data {data, length};;
}

void Packet::Write(int num)
{
	for(int i = 0; i < 4; ++i) {
		Bytes_To_Send.push_back(num >> (i*8));
	}
}
void Packet::Write(bool boolean)
{
	unsigned char a =boolean == true ? 1 : 0 ;
	Bytes_To_Send.push_back(a);
}
bool Packet::GetBool(bool *error)
{
	if(_seeker >= Receive_Length) {
		//Index out of range
		*error = true;
		return false;
	}
	bool boolean = false;
	boolean = Received_Bytes[_seeker++] == 1 ? true : false;
	return boolean;
}

void Packet::Write(short num)
{
	for(int i = 0; i < 2; ++i) {
		Bytes_To_Send.push_back(num >> (i*8));
	}
}
int Packet::GetInt(bool *error)
{
	if(_seeker + 3 >= Receive_Length) {
		//Index out of range
		*error = true;
		return 0;
	}

	int num = 0;

	for(int i = 0; i < 4; ++i) {
		num = num | Received_Bytes[_seeker++] << (i*8);
	}
	return num;
}
void Packet::Write(unsigned long long num)
{
	for(int i = 0; i < 8; ++i) {
		Bytes_To_Send.push_back(num >> (i*8));
	}
}
short Packet::GetShort(bool *error)
{
	if(_seeker + 1 >= Receive_Length) {
		//Index out of range
		*error = true;
		return 0;
	}

	short num = 0;

	for(int i = 0; i < 2; ++i) {
		num = num | Received_Bytes[_seeker++] << (i*8);
	}
	return num;
}
unsigned long long Packet::GetULL(bool *error)
{
	if(_seeker + 7 >= Receive_Length) {
		//Index out of range
		*error = true;
		return 0;
	}

	unsigned long long num = 0;

	for(int i = 0; i < 8; ++i) {
		num = num | Received_Bytes[_seeker++] << (i*8);

	}
	return num;
}
void Packet::Write(std::string str)
{
	const int size = str.size();
	Write(size);
	for(int i = 0; i < size; ++i) {
		unsigned char mod = (unsigned char)str[i];
		Bytes_To_Send.push_back(std::move(mod));
	}
}
bool Packet::CheckFinalizers(bool *error)
{
	if(_seeker + 3 >= Receive_Length) {
		//Index out of range
		*error = true;
		return false;
	}
	if(Received_Bytes[_seeker++] == 233 && Received_Bytes[_seeker++] == 232&& Received_Bytes[_seeker++] == 231&& Received_Bytes[_seeker++]== 230) {
		return true;
	} else {
		*error = true;
		return false;
	}
}
std::string Packet::GetString(bool *error)
{
	const int size = GetInt(error);
	if(size < 0) {
		*error = true;
		return "";
	}
	char cstring[size + 1];

	if(_seeker + size - 1 >= Receive_Length) {
		//Index out of range
		*error = true;
		return "";
	}
	std::copy(Received_Bytes.get() + _seeker, Received_Bytes.get() + _seeker + size, cstring);
	_seeker += size;
	cstring[size] = '\0';
	std::string cppstring = cstring;

	return cppstring;
}
Packet::Packet(boost::shared_ptr<unsigned char[]> data, unsigned int r)
{
	Received_Bytes = data;
	Receive_Length = r;
	_seeker = 0;
};
Packet::Packet(int packetid)
{
	Write(packetid);
	_seeker = 0;
};
Packet::~Packet()
{

};
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