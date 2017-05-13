#include <cstdint>

class Monitor
{
private:
  int32_t x_;
  int32_t y_;

public:
  Monitor();
  ~Monitor();

  void reset();                     // リセット
  void display(const char* str);       // 文字表示
  void next();                      // 次の行に移る

};
