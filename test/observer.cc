

#include <memory>
#include <vector>

#include "../muduo/base/Mutex.hpp"

class Observable {
public:
  void register_(std::weak_ptr<Observable> x);
  void notifyObservers();

private:
  mutable muduo::MutexLock                                 mutex_;
  std::vector<std::weak_ptr<Observable>>                   observers_;
  typedef std::vector<std::weak_ptr<Observable>>::iterator Iterator;
};

void Observable::notifyObservers() {
  muduo::MutexLockGuard lockguard(mutex_);
  Iterator              it = observers_.begin();
  while (it != observers_.end()) {
    std::shared_ptr<Observable> obj(it->lock());
    if (obj) {
      // 提升成功
      obj->update();
      ++it;
    } else {
      // 对象已销毁，从容器中去除weak_ptr
      it = observers_.erase(it);
    }
  }
}
