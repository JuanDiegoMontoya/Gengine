/*HEADER_GOES_HERE*/
#ifndef FrameRateController_DEF
#define FrameRateController_DEF

#include "Manager.h"
#include <chrono>
#include "../FactoryID.h"

class FrameRateController : public Manager
{
public:

  static FrameRateController* pFrameRateController;

  static FrameRateController* const GetFrameRateController() { SINGLETON(FrameRateController, pFrameRateController); }

  ~FrameRateController();
  void Init();
  void End();
  std::string GetName();
  float Update();
  void SetFrameRate(int fps_);
  float ElapsedTime() const { return elapsedTime; }

  PROPERTY(Bool, locked, true);

private:
  static std::chrono::nanoseconds error;

  FrameRateController();

  PROPERTY(Int, fps, 60);
  float elapsedTime = 0.0f;
  std::chrono::high_resolution_clock::time_point startTime;
  std::chrono::high_resolution_clock::time_point frameStartTime;

  friend void RegisterManagers();
};


#endif // !FrameRateController_DEF