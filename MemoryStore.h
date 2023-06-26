#ifndef __MEMORY_STORE_H__
#define __MEMORY_STORE_H__
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<iostream>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>
#include<time.h>
#include<string.h>

template<class T>
class MemoryStore {
public:
  MemoryStore(int ipc_key, int max_size) :is_attach_(false), ipc_key_(ipc_key), max_size_(max_size) {
    attach();
  };
  ~MemoryStore() {
    detach();
  }

  T* get() {
    if (!is_attach_) {
      attach();
    }
    return (T*)data_;
  }

  void* get(uint32_t pos) {
    if (!is_attach_) {
      attach();
    }
    return (uint8_t*)data_ + pos;
  }
private:
  bool attach() {
    if (is_attach_) {
      return true;
    }
    int shmid = shmget((key_t)ipc_key_, max_size_, 0666 | IPC_CREAT);
    if (shmid == -1) {
      std::cout << "shmget failed:" << strerror(errno) << std::endl;
      return false;
    }
    void* shm = shmat(shmid, 0, 0);
    if (shm == (void*)-1) {
      std::cout << "shmat failed! " << strerror(errno) << std::endl;
      return false;
    }
    data_ = (T*)shm;
    printf("attach ipc_key %d to 0x%x successful!\n", ipc_key_, shm);
    is_attach_ = true;
    return true;
  }

  bool detach() {
    printf("detach ipc_key %d successful!\n", ipc_key_);
    if (is_attach_) {
      return shmdt(data_) != -1;
    }
    return true;
  }
private:
  T*    data_;
  bool  is_attach_;
  int   ipc_key_;
  int   max_size_;
};

#endif
