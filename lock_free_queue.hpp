#ifndef __LOCK_FREE_QUEUE_HPP_
#define __LOCK_FREE_QUEUE_HPP_
#include <atomic>
#include <memory>

template<typename T>
class LockFreeQueue {
private:
  struct Node {
    std::shared_ptr<T> data;
    std::atomic<Node*> next;

    Node(const T& item) : data(std::make_shared<T>(item)), next(nullptr) {}
  };

  alignas(128) std::atomic<Node*> head;
  alignas(128) std::atomic<Node*> tail;

public:
  LockFreeQueue() : head(new Node(T())), tail(head.load()) {}

  ~LockFreeQueue() {
    while (Node* const oldHead = head.load()) {
      head.store(oldHead->next);
      delete oldHead;
    }
  }

  void Enqueue(const T& item) {
    Node* newNode = new Node(item);
    while (true) {
      Node* const currTail = tail.load();
      Node* const next = currTail->next.load();
      if (currTail == tail.load()) {
        if (next == nullptr) {
          if (std::atomic_compare_exchange_strong(&currTail->next, &next, newNode)) {
            std::atomic_compare_exchange_strong(&tail, &currTail, newNode);
            return;
          }
        }
        else {
          std::atomic_compare_exchange_strong(&tail, &currTail, next);
        }
      }
    }
  }

  std::shared_ptr<T> Dequeue() {
    while (true) {
      Node* const currHead = head.load();
      Node* const currTail = tail.load();
      Node* const next = currHead->next.load();
      if (currHead == head.load()) {
        if (currHead == currTail) {
          if (next == nullptr) {
            return std::shared_ptr<T>();
          }
          std::atomic_compare_exchange_strong(&tail, &currTail, next);
        }
        else {
          if (std::atomic_compare_exchange_strong(&head, &currHead, next)) {
            std::shared_ptr<T> result(currHead->data);
            delete currHead;
            return result;
          }
        }
      }
    }
  }
};
#endif
