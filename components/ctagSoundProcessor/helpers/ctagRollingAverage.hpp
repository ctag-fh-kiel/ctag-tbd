#ifndef ctagRollingAverge_h
#define ctagRollingAverge_h

#include <deque>
#include "ctagFastMath.hpp"

namespace CTAG::SP::HELPERS
{
    // Inspired by algorithm as found here: https://im-coder.com/berechne-rolling-moving-average-in-c.html
    #define ctag_stdev(cnt, sum, ssq) fastsqrt((((float)(cnt))*ssq - fastpow2((float)(sum))) / ((float)(cnt)*((float)(cnt)-1)))
    class ctagRollingAverage
    {
      public:
          ctagRollingAverage(int n=10)
          {
              sum=0;
              ssq=0;
              q.resize(n);
          }
          void push(float v)
          {
             float t=q.front();
             sum-=t;
             ssq-=t*t;
             q.pop_front();
             q.push_back(v);
             sum+=v;
             ssq+=v*v;
          }
          float size()
          {
              return q.size();
          }
          float mean()
          {
              return sum/size();
          }
          float dejitter(float cv_val)
          {
            push(cv_val);
            return sum/size();
          }
          float stdev()
          {
              return ctag_stdev(size(), sum, ssq);
          }
      private:
        std::deque<float> q = {0};
        float sum;
        float ssq;
    };

    class TBDdejitter
    {
    public:
        TBDdejitter(unsigned int numElements=10) { m_numElements = numElements; }
        float dejitter(float cv_value)
        {
          average = average - last_entry/m_numElements + cv_value/m_numElements;
          last_entry = cv_value;
          return average*m_numElements;
        }
    private:
        unsigned int m_numElements = 10;
        float average = 0;
        float last_entry = 0;
    };
};


#endif
