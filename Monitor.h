#include <cstdint>

class Monitor
{
private:
  int32_t x_;
  int32_t y_;

public:
  Monitor();
  ~Monitor();

  void reset();                     // ���Z�b�g
  void display(const char* str);       // �����\��
  void next();                      // ���̍s�Ɉڂ�

};
