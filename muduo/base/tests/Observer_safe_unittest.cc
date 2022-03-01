#include <stdio.h>

#include <algorithm>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>

#include "../Mutex.h"

class Observable;

class Observer : public boost::enable_shared_from_this<Observer> {
public:
  virtual ~Observer();

  virtual void update() = 0;

  void observe(Observable *s);

protected:
  Observable *subject_;
};

class Observable {
public:
  void register_(boost::weak_ptr<Observer> x);

  void notifyObservers() {
    muduo::MutexLockGuard lk(mutex_);

    Iterator it = observers_.begin();

    while (it != observers_.end()) {
      boost::shared_ptr<Observer> obj(it->lock());
      if (obj) {
        obj->update();
        it++;
      } else {
        printf("notify servers erase\n");
        it = observers_.erase(it);
      }
    }
  }

private:
  mutable muduo::MutexLock               mutex_;
  std::vector<boost::weak_ptr<Observer>> observers_;

  typedef std::vector<boost::weak_ptr<Observer>>::iterator Iterator;
};

Observer::~Observer() {
}

void Observer::observe(Observable *s) {
  s->register_(shared_from_this());
  subject_ = s;
}

void Observable::register_(boost::weak_ptr<Observer> x) {
  observers_.push_back(x);
}

class Foo : public Observer {
  virtual void update() override {
    printf("update %p\n", this);
  }
};

int main() {
  Observable subject;
  {
    boost::shared_ptr<Foo> p(new Foo);
    p->observe(&subject);
    subject.notifyObservers();
  }

  subject.notifyObservers();
}