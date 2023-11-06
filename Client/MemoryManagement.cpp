#include "MemoryManagement.h"
#include "Program.h"

std::vector<void*> MemoryManagement::Instantiated_Objs;
std::vector<void*> MemoryManagement::Objs_To_Add;
std::vector<DeletionObject> MemoryManagement::ObjDeletion_List;
std::vector<DeletionObject> MemoryManagement::Objs_To_Delete;
std::mutex MemoryManagement::objs_mutex;
std::mutex MemoryManagement::objdelete_mutex;

void MemoryManagement::Add_Obj(void *obj)
{
	std::lock_guard<std::mutex> lock(objs_mutex);
	Objs_To_Add.push_back(std::move(obj));
}
void MemoryManagement::Delete_Obj(void *obj, int the_class)
{
	std::lock_guard<std::mutex> lock(objdelete_mutex);
	Objs_To_Delete.push_back(std::move(DeletionObject{obj,0,0, the_class}));
}
void MemoryManagement::Delete_Thread(void *obj)
{
	std::lock_guard<std::mutex> lock(objdelete_mutex);
	Objs_To_Delete.push_back(std::move(DeletionObject{obj,2,0,-1}));
}
void MemoryManagement::Update()
{
	int iterations = 0;
	while (Program::running== true) {
		if(iterations <= 10) {
			{

				std::vector<DeletionObject> _temp;
				std::unique_lock<std::mutex> lock(objdelete_mutex);
				while(Objs_To_Delete.empty() == false) {
					_temp.push_back(std::move(Objs_To_Delete.front()));
					Objs_To_Delete.erase(Objs_To_Delete.begin());
				}
				lock.unlock();
				for(int i =0 ; i < _temp.size(); ++i) {
					bool found = false;
					for(unsigned int ii = 0; ii < MemoryManagement::ObjDeletion_List.size(); ++ii) {
						if(_temp[i].ptr == MemoryManagement::ObjDeletion_List[ii].ptr)
							found = true;
					}
					auto obj(std::move(_temp[i]));
					if(found == false) {
						MemoryManagement::ObjDeletion_List.push_back(std::move(obj));
					}
				}
			}
		}
		if(iterations <= 20) {
			{
				std::vector<void*> _temp;
				std::unique_lock<std::mutex> lock(objs_mutex);
				while(Objs_To_Add.empty() == false) {
					_temp.push_back(std::move(Objs_To_Add.front()));
					Objs_To_Add.erase(Objs_To_Add.begin());
				}
				lock.unlock();
				for(int i =0 ; i < _temp.size(); ++i) {
					bool found = false;
					for(unsigned int ii = 0; ii < MemoryManagement::Instantiated_Objs.size(); ++ii) {
						if(_temp[i] == MemoryManagement::Instantiated_Objs[ii])
							found = true;
					}
					void* obj = std::move(_temp[i]);
					if(found == false) {
						MemoryManagement::Instantiated_Objs.push_back(std::move(obj));
					}
				}
			}
		}
		if(iterations == 25) {
			std::vector<DeletionObject> comebackto;
			while(MemoryManagement::ObjDeletion_List.size() > 0) {
				bool found = false;
				for(unsigned int ii = 0; ii < MemoryManagement::Instantiated_Objs.size(); ++ii) {
					if(MemoryManagement::ObjDeletion_List[0].ptr == MemoryManagement::Instantiated_Objs[ii]) {
						MemoryManagement::Instantiated_Objs.erase(MemoryManagement::Instantiated_Objs.begin() + ii--);
						found = true;
					}
				}

				if(found == true) {
					int tc = MemoryManagement::ObjDeletion_List[0].the_class;
					switch(MemoryManagement::ObjDeletion_List[0].type) {
					case 0:
						if(tc == 0)
							delete (User*)MemoryManagement::ObjDeletion_List[0].ptr;
						else if(tc == 2)
							delete (FileStream*)MemoryManagement::ObjDeletion_List[0].ptr;
						else
							delete (void*)MemoryManagement::ObjDeletion_List[0].ptr;

						break;
					case 2:
						if(MemoryManagement::ObjDeletion_List[0].ptr != nullptr && MemoryManagement::ObjDeletion_List[0].ptr != nullptr) {
							((std::thread*)MemoryManagement::ObjDeletion_List[0].ptr)->detach();
							delete (std::thread*)MemoryManagement::ObjDeletion_List[0].ptr;
						}
						break;
					default:
						break;
					}
				} else {
					if(MemoryManagement::ObjDeletion_List[0].retry++ < 100) {
						comebackto.push_back(std::move(MemoryManagement::ObjDeletion_List[0]));
					}
				}
				MemoryManagement::ObjDeletion_List.erase(MemoryManagement::ObjDeletion_List.begin());
			}
			if(comebackto.size() > 0) {
				for(unsigned int i =0; i < comebackto.size(); ++i) {
					MemoryManagement::ObjDeletion_List.push_back(std::move(comebackto[i]));
				}

			}
			comebackto.clear();
		}

		if(iterations++ >= 25) {
			iterations = 0;
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
