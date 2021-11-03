#ifndef __STOCK_H__
#define __STOCK_H__

#include <atomic>
#include <iostream>

class Stock {
public:
  Stock(const std::string &name) : name_(name), price_(0) {
  }

  ~Stock() {
  }

  double getPrice() const {
    return price_.load(std::memory_order_relaxed);
  }

  void setPrice(const double price) {
    price_.store(price, std::memory_order_relaxed);
  }

  std::string key() const {
    return name_;
  }

private:
  std::string         name_;
  std::atomic<double> price_;
};  // class Stock

#endif /* __STOCK_H__ */
