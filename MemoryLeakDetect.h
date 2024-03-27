#ifndef __MEMORYLEAKDETECT_H__
#define __MEMORYLEAKDETECT_H__

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>

struct MemoryBlockNode {
  char file_name[128];
  int line;
  int size;
  void* link;
  MemoryBlockNode* prev;
  MemoryBlockNode* next;
};

struct MemoryBlockList
{
  void insert(MemoryBlockNode* node);

  MemoryBlockNode* find_node_by_link(void* link) const;

  void remove(void* link);

  MemoryBlockNode* head;
};

class MemoryLeakDetect {
public:
  static MemoryLeakDetect& instance();

  void insert(void* ptr, const char* file, int line, int size);

  void erase(void* ptr);

  void print();
private:
  MemoryBlockList memory_block_list;
};

void* operator new(std::size_t size, const char* file, int line);

void* operator new[](std::size_t size, const char* file, int line);

void operator delete(void* ptr);

void operator delete[](void* ptr);

#define new new(__FILE__, __LINE__)
#endif