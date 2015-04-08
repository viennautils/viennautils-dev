#ifndef VIENNAUTILS_TIMER_HPP
#define VIENNAUTILS_TIMER_HPP

/* ============================================================================
   Copyright (c) 2011-2014, Institute for Microelectronics,
                            Institute for Analysis and Scientific Computing,
                            TU Wien.

                            -----------------
                ViennaMesh - The Vienna Meshing Framework
                            -----------------

                    http://viennamesh.sourceforge.net/

   License:         MIT (X11), see file LICENSE in the base directory
=============================================================================== */

#include <iostream>



//void printOps(double num_ops, double exec_time)
//{
//  std::cout << "GFLOPS: " << num_ops / (1000000 * exec_time * 1000) << std::endl;
//}

#ifdef _WIN32

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max
namespace viennautils
{
  //! timer class
  class Timer
  {
  public:
    //! default constructor
    Timer()
    {
      QueryPerformanceFrequency(&freq);
    }
    //! start the timer
    void start()
    {
      QueryPerformanceCounter((LARGE_INTEGER*) &start_time);
    }
    //! retrieve the timer count
    double get() const
    {
      LARGE_INTEGER  end_time;
      QueryPerformanceCounter((LARGE_INTEGER*) &end_time);
      return (static_cast<double>(end_time.QuadPart) - static_cast<double>(start_time.QuadPart)) / static_cast<double>(freq.QuadPart);
    }


  private:
    //! timer states
    LARGE_INTEGER freq;
    LARGE_INTEGER start_time;
  };
}

#else

#include <sys/time.h>

namespace viennautils
{
  //! timer class
  class Timer
  {
  public:
    //! default constructor
    Timer() : ts(0)
    {}
    //! start the timer
    void start()
    {
      struct timeval tval;
      gettimeofday(&tval, NULL);
      ts = tval.tv_sec * 1000000 + tval.tv_usec;
    }
    //! retrieve the timer count
    double get() const
    {
      struct timeval tval;
      gettimeofday(&tval, NULL);
      unsigned long end_time = tval.tv_sec * 1000000 + tval.tv_usec;

      return static_cast<double>(end_time-ts) / 1000000.0;
    }

  private:
    //! state
    unsigned long ts;
  };
}

#endif

#endif
