#ifndef SEQUENCE_QUEUE_H
#define SEQUENCE_QUEUE_H

#include "SequenceTypes.h"
#include <vector>

// Define a max queue size to prevent memory issues
#define MAX_QUEUE_SIZE 20

class SequenceQueue {
private:
  std::vector<SequenceStep> queue;

public:
  SequenceQueue() {}

  bool add(const SequenceStep &step) {
    if (queue.size() >= MAX_QUEUE_SIZE) {
      return false;
    }
    queue.push_back(step);
    return true;
  }

  bool isEmpty() const { return queue.empty(); }

  // Peek at the first item
  bool peek(SequenceStep &step) const {
    if (isEmpty())
      return false;
    step = queue.front();
    return true;
  }

  // Remove the first item
  void pop() {
    if (!isEmpty()) {
      queue.erase(queue.begin());
    }
  }

  int size() const { return queue.size(); }

  void clear() { queue.clear(); }
};

#endif
