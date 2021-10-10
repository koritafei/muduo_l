#ifndef __NONCOPYABLE_H__
#define __NONCOPYABLE_H__

namespace muduo {

class Noncopyable {
public:
  Noncopyable(const Noncopyable &) = delete;
  Noncopyable &operator=(const Noncopyable &) = delete;

protected:
  Noncopyable()          = default;
  virtual ~Noncopyable() = default;
};

}  // namespace muduo

#endif /* __NONCOPYABLE_H__ */
