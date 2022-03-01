#ifndef __NONCOPYABLE_H__
#define __NONCOPYABLE_H__

namespace muduo {

class noncopyable {
public:
  void operator=(const noncopyable &) = delete;
  noncopyable(const noncopyable &)    = delete;

protected:
  noncopyable()  = default;
  ~noncopyable() = default;
};

}  // namespace muduo

#endif /* __NONCOPYABLE_H__ */
