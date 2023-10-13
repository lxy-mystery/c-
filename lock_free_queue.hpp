#ifndef __LOCK_FREE_QUEUE_HPP_H__
#define __LOCK_FREE_QUEUE_HPP_H__
#include <atomic>

template <typename T>
class LockFreeQueue {
private:
  struct Node {
    T data;
    std::atomic<Node*> next;

    Node(const T& value) : data(value), next(nullptr) {}
  };

  alignas(64) std::atomic<Node*> head;
  alignas(64) std::atomic<Node*> tail;

public:
  LockFreeQueue() {
    Node* dummyNode = new Node(T());
    head.store(dummyNode, std::memory_order_relaxed);
    tail.store(dummyNode, std::memory_order_relaxed);
  }

  ~LockFreeQueue() {
    while (Node* node = head.load(std::memory_order_relaxed)) {
      head.store(node->next.load(std::memory_order_relaxed), std::memory_order_relaxed);
      delete node;
    }
  }

  void Enqueue(const T& value) {
    Node* newNode = new Node(value);
    newNode->next.store(nullptr, std::memory_order_relaxed);

    Node* prevTail = nullptr;
    Node* currTail = nullptr;

    while (true) {
      prevTail = tail.load(std::memory_order_relaxed);
      currTail = prevTail->next.load(std::memory_order_acquire);

      if (prevTail == tail.load(std::memory_order_relaxed)) {
        if (currTail == nullptr) {
          if (prevTail->next.compare_exchange_weak(currTail, newNode, std::memory_order_release, std::memory_order_relaxed)) {
            break;
          }
        }
        else {
          tail.compare_exchange_weak(prevTail, currTail, std::memory_order_relaxed, std::memory_order_relaxed);
        }
      }
    }

    tail.compare_exchange_weak(prevTail, newNode, std::memory_order_release, std::memory_order_relaxed);
  }

  bool Dequeue(T& value) {
    Node* prevHead = nullptr;
    Node* currHead = nullptr;

    while (true) {
      prevHead = head.load(std::memory_order_relaxed);
      currHead = prevHead->next.load(std::memory_order_acquire);

      if (prevHead == head.load(std::memory_order_relaxed)) {
        if (currHead == nullptr) {
          return false;  // Queue is empty
        }

        if (head.compare_exchange_weak(prevHead, currHead, std::memory_order_relaxed, std::memory_order_relaxed)) {
          value = currHead->data;
          delete prevHead;
          break;
        }
      }
    }

    return true;
  }
};
#endif
