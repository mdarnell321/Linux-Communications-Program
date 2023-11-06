#ifndef MEMMANAGER_H
#define MEMMANAGER_H

#include <iostream>
#include <vector>
#include <mutex>
typedef struct {
	void* ptr;
	int type;
	int retry;
	int the_class;
} DeletionObject;

class MemoryManagement
{
public:
	static void Delete_Obj(void*, int);
	static void Delete_Thread(void*);
	static void Add_Obj(void*);
	static void Update();
	static std::vector<void*> Objs_To_Add;
	static std::vector<void*> Instantiated_Objs;
	static std::vector<DeletionObject> Objs_To_Delete;
	static std::vector<DeletionObject> ObjDeletion_List;
	static std::mutex objs_mutex;
	static std::mutex objdelete_mutex;
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