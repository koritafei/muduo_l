#ifndef __STOCKFACTORY_H__
#define __STOCKFACTORY_H__

#include <muduo/base/Mutex.h>

#include <boost/noncopyable.hpp>
#include <functional>
#include <map>
#include <memory>

#include "Stock.h"

using std::placeholders::_1;

class StockFactory : public boost::noncopyable,
                     std::enable_shared_from_this<StockFactory> {
public:
  std::shared_ptr<Stock> get(const std::string &key) {
    std::shared_ptr<Stock> pStock;
    muduo::MutexLockGuard  lock(mutex_);
    std::weak_ptr<Stock> & wkStock = stocks_[key];
    pStock                         = wkStock.lock();
    if (!pStock) {
      pStock.reset(new Stock(key),
                   std::bind(&StockFactory::weakDeleteCallback,
                             std::weak_ptr<StockFactory>(shared_from_this()),
                             _1));
      wkStock = pStock;
    }
    return pStock;
  }

private:
  static void weakDeleteCallback(const std::weak_ptr<StockFactory> &wkFactory,
                                 Stock *                            stock) {
    std::shared_ptr<StockFactory> factory(wkFactory.lock());
    if (factory) {
      factory->removeStock(stock);
    }
    delete stock;
  }

  void removeStock(Stock *stock) {
    if (stock) {
      muduo::MutexLockGuard lock(mutex_);
      stocks_.erase(stock->key());
    }
  }

  mutable muduo::MutexLock                    mutex_;
  std::map<std::string, std::weak_ptr<Stock>> stocks_;
};  // class StockFactory

#endif /* __STOCKFACTORY_H__ */
