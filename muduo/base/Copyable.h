#ifndef __COPYABLE_H__
#define __COPYABLE_H__

namespace muduo {

class copyable {
protected:
  copyable()  = default;
  ~copyable() = default;
};

}  // namespace muduo

#endif /* __COPYABLE_H__ */
