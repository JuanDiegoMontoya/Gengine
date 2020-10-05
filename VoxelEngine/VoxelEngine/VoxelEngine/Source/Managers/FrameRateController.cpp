/*HEADER_GOES_HERE*/
#include "../../Headers/Managers/FrameRateController.h"
#include "../../Headers/Factory.h"
#include <thread>

FrameRateController* FrameRateController::pFrameRateController = nullptr;

std::chrono::nanoseconds FrameRateController::error = std::chrono::nanoseconds(1); // One ten-thousandth of a frame, our acceptable error.

FrameRateController::FrameRateController()
{
}

FrameRateController::~FrameRateController()
{
  pFrameRateController = nullptr;
}

std::string FrameRateController::GetName()
{
  return "FrameRateController";
}

void FrameRateController::Init()
{
  elapsedTime = 0.0f;                                     // Reset the elapsed time
  startTime = std::chrono::high_resolution_clock::now();  // Handle fence-post of first start time.
  frameStartTime = startTime;                             // The frame start time may vary slightly from the desired start time.
}

void FrameRateController::End()
{
}

float FrameRateController::Update()
{
  //All autos are std::chrono::nanoseconds, but I'm lazy and don't want to type that..
  std::chrono::nanoseconds fpsInNanoseconds(1000000000 / int(fps));

  auto endTime = std::chrono::high_resolution_clock::now(); //Frame has just ended, get the end time

  auto dt = endTime - startTime;                            //Calculate how long the frame took to complete

  if (locked) //If the frame late is locked..
  {
    auto desiredEndTIme = startTime + fpsInNanoseconds; //Get the end time that will keep us running at 60 fps.

    if (endTime < desiredEndTIme) //If the frame occurred faster than the frame rate,
    {

      while (endTime < desiredEndTIme) //Sleep for the rest of the frame time
      {
        std::this_thread::sleep_for(error);

        endTime = std::chrono::high_resolution_clock::now();
      }
      dt = endTime - startTime;
      //startTime += fpsInNanoseconds; //Get the new start time
      //elapsedTime += 1.0f / int(fps);     //Add one frame to the elapsed time.
      //return 1.0f / int(fps);             //Return the frame's duration.
    }
  }

  //If the frame rate is unlocked or game is lagging..
  float  returnDt = (float)dt.count() / 1'000'000'000;
  elapsedTime += returnDt;              //Add the frame time to dt.
  startTime = endTime;                    //Get the new start time.
  return returnDt;  //Return the exact frame time.

}

void FrameRateController::SetFrameRate(int fps_)
{
  fps = fps_;
}
