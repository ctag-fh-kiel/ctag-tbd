/**
 *  DelayLine Processor Base Class
 *
 *  Copyright (C) 2006-2018 Teru Kamogashira
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  aint32_t with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * DelayLine Processor Base Class
 */
class _FV3_(delayline)
{
 public:
  _FV3_(delayline)() ;
  virtual _FV3_(~delayline)();
  void free();
  virtual void         setSampleRate(_fv3_float_t fs) ;
  virtual _fv3_float_t getSampleRate();
  void setsize(int32_t size) ;
  int32_t getsize();
  virtual void mute();
  virtual _fv3_float_t process(_fv3_float_t input);
  /**
   * set the prime mode for delay lines.
   * the size of the delay lines will prime numbers by default.
   * To inhibit prime number size delay lines, set this options to false.
   * @param[in] value set true (default) to enable prime mode.
   */
  virtual void setPrimeMode(bool value){primeMode = value;}
  virtual bool getPrimeMode(){return primeMode;}
  
  inline _fv3_float_t& at(int32_t rindex)
  {
    int32_t readidx = baseidx + rindex;
    if(readidx >= bufsize) readidx -= bufsize;
    return buffer[readidx];
  }
  inline _fv3_float_t& operator[](int32_t rindex){return at(rindex);}

  inline _fv3_float_t at(_fv3_float_t rindex, _fv3_float_t& ap_save)
  {
    _fv3_float_t floor_mod = std::floor(rindex); // >= 0
    _fv3_float_t frac = rindex - floor_mod; // >= 0
    
    int32_t readidx_a = baseidx + (int32_t)floor_mod;
    if(readidx_a >= bufsize) readidx_a -= bufsize;
    int32_t readidx_b = readidx_a + 1;
    if(readidx_b >= bufsize) readidx_b -= bufsize;
    
    _fv3_float_t temp = buffer[readidx_b] + buffer[readidx_a]*(1-frac) - (1-frac)*ap_save;
    UNDENORMAL(temp);
    ap_save = temp;    
    return temp;
  }

 protected:
  inline void allpass(int32_t rindexbase, int32_t rlength, _fv3_float_t feedback)
  {
    int32_t rindex1 = rindexbase, rindex2 = rindex1 + rlength;
    _fv3_float_t r1 = (*this)[rindex1], r2 = (*this)[rindex2];
    r2 -= feedback * r1; r1 += feedback * r2;
    (*this)[rindex1] = r1; (*this)[rindex2] = r2;
  }

  /**
   * get the prime number length for delay lines.
   * @param[in] ms delay length in msec.
   */
  virtual int32_t p_(_fv3_float_t ms);
  
  _FV3_(delayline)(const _FV3_(delayline)& x);
  _FV3_(delayline)& operator=(const _FV3_(delayline)& x);  
  _fv3_float_t *buffer, currentfs;
  int32_t bufsize, baseidx;
  bool primeMode;
};
