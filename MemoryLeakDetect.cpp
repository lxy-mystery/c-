#include "pch.h"
#include "MemoryLeakDetect.h"
#ifdef new
#undef new
inline void MemoryBlockList::insert(MemoryBlockNode* node) {
  if (head == nullptr) {
    head = node;
  }
  else {
    node->next = head;
    head->prev = node;
    head = node;
  }
}

inline MemoryBlockNode* MemoryBlockList::find_node_by_link(void* link) const {
  MemoryBlockNode* current = head;
  while (current != nullptr) {
    if (current->link == link) {
      return current;
    }
    current = current->next;
  }
  return nullptr;
}

inline void MemoryBlockList::remove(void* link)
{
  MemoryBlockNode* node = find_node_by_link(link);
  if (node == nullptr) {
    printf("==== WARN ==== address %p maybe double delete\n", link);
    return;
  }

  if (node == head) {
    head = node->next;
  }

  if (node->prev != nullptr) {
    node->prev->next = node->next;
  }

  if (node->next != nullptr) {
    node->next->prev = node->prev;
  }
  free(node);
}

inline MemoryLeakDetect& MemoryLeakDetect::instance() {
  static MemoryLeakDetect detect;
  return detect;
}

inline void MemoryLeakDetect::insert(void* ptr, const char* file, int line, int size) {
  MemoryBlockNode* node = (MemoryBlockNode*)malloc(sizeof(MemoryBlockNode));
  if (node == nullptr) {
    exit(-1);
  }
  node->line = line;
  node->size = size;
  node->link = ptr;
  node->prev = nullptr;
  node->next = nullptr;
  strcpy(node->file_name, file);
  printf("==== new ==== file %s:%d, address:%p, size:%d\n", node->file_name, node->line, node->link, node->size);
  memory_block_list.insert(node);
}

inline void MemoryLeakDetect::erase(void* ptr) {
  printf("==== delete ==== address:%p\n", ptr);
  memory_block_list.remove(ptr);
}

inline void MemoryLeakDetect::print() {
  MemoryBlockNode* current = memory_block_list.head;
  while (current != nullptr) {
    printf("==== info ==== file %s:%d, address:%p, size:%d\n", current->file_name, current->line, current->link, current->size);
    current = current->next;
  }
}

void* operator new(std::size_t size, const char* file, int line) {
  void* ptr = malloc(size);
  MemoryLeakDetect::instance().insert(ptr, file, line, size);
  return ptr;
}

void* operator new[](std::size_t size, const char* file, int line) {
  void* ptr = malloc(size);
  MemoryLeakDetect::instance().insert(ptr, file, line, size);
  return ptr;
}

void operator delete(void* ptr) {
  MemoryLeakDetect::instance().erase(ptr);
  free(ptr);
  ptr = nullptr;
}

void operator delete[](void* ptr) {
  MemoryLeakDetect::instance().erase(ptr);
  free(ptr);
  ptr = nullptr;
}
#define new new(__FILE__, __LINE__)
#endif