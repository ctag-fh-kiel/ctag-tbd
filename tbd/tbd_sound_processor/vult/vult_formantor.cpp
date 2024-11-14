/* Code automatically generated by Vult https://github.com/modlfo/vult */
#include "vult_formantor.h"

void Util__ctx_type_3_init(Util__ctx_type_3 &_output_){
   Util__ctx_type_3 _ctx;
   _ctx.y1 = 0.0f;
   _ctx.x1 = 0.0f;
   _output_ = _ctx;
   return ;
}

float Util_dcblock(Util__ctx_type_3 &_ctx, float x0){
   float y0;
   y0 = (x0 + (- _ctx.x1) + (0.995f * _ctx.y1));
   _ctx.x1 = x0;
   _ctx.y1 = y0;
   return y0;
}

void Tricore__ctx_type_0_init(Tricore__ctx_type_0 &_output_){
   Tricore__ctx_type_0 _ctx;
   _ctx.reset_state = false;
   _ctx.reset_phase = 0.0f;
   _ctx.rate = 0.0f;
   _ctx.phase = 0.0f;
   _ctx.direction = false;
   Util__ctx_type_0_init(_ctx._inst451);
   Util__ctx_type_0_init(_ctx._inst351);
   Util__ctx_type_1_init(_ctx._inst13b);
   Tricore_default(_ctx);
   _output_ = _ctx;
   return ;
}

float Tricore_process(Tricore__ctx_type_0 &_ctx, float cv, float reset, float disable){
   float out;
   out = 0.0f;
   if(Util_change(_ctx._inst13b,cv)){
      _ctx.rate = (4.f * Util_cvToRate_1024(cv));
   }
   float new_phase;
   float _if_13;
   if(_ctx.direction){
      _if_13 = _ctx.rate;
   }
   else
   {
      _if_13 = (- _ctx.rate);
   }
   new_phase = (_ctx.phase + _if_13);
   uint8_t bdisable;
   bdisable = (disable > 0.5f);
   if(Util_edge(_ctx._inst351,(reset > 0.0f)) || Util_edge(_ctx._inst451,bdisable)){
      _ctx.reset_phase = _ctx.phase;
      _ctx.reset_state = true;
      new_phase = 0.0f;
   }
   if(new_phase > 1024.f){
      new_phase = (1024.f + (- (-1024.f + new_phase)));
      _ctx.direction = false;
   }
   else
   {
      if(new_phase < -1024.f){
         _ctx.direction = true;
      }
   }
   if(bdisable){
      _ctx.phase = 0.0f;
   }
   else
   {
      _ctx.phase = new_phase;
   }
   if(_ctx.reset_state){
      if((fabsf(_ctx.reset_phase) > 10.f) || bdisable){
         float reset_speed;
         if(bdisable){
            reset_speed = 0.01f;
         }
         else
         {
            reset_speed = 0.5f;
         }
         _ctx.reset_phase = (_ctx.reset_phase + (- (_ctx.reset_phase * reset_speed)));
      }
      else
      {
         uint8_t _cond_17;
         _cond_17 = (_ctx.reset_phase > 10.f);
         if(bool_not(_cond_17)){
            _ctx.reset_phase = 0.0f;
         }
         _ctx.reset_state = false;
         _ctx.direction = true;
      }
      out = _ctx.reset_phase;
   }
   out = (_ctx.phase + out);
   return (0.0009765625f * out);
}

float Noise_pinker(Noise__ctx_type_0 &_ctx, float x){
   float b0;
   b0 = 0.0277129863191f;
   float b1;
   b1 = 0.0277129863191f;
   float a1;
   a1 = -0.944574027362f;
   float w0;
   w0 = (x + (- (_ctx.w1 * a1)));
   float y0;
   y0 = ((_ctx.w1 * b1) + (b0 * w0));
   _ctx.w1 = w0;
   return y0;
}

void Noise__ctx_type_1_init(Noise__ctx_type_1 &_output_){
   Noise__ctx_type_1 _ctx;
   _ctx.x2 = 0;
   _ctx.x1 = 0;
   Noise__ctx_type_0_init(_ctx._inst190);
   _output_ = _ctx;
   return ;
}

float Noise_process(Noise__ctx_type_1 &_ctx, float color){
   _ctx.x1 = ((7919 + (17389 * _ctx.x1)) % 32768);
   float y1;
   y1 = (3.0517578125e-005f * int_to_float(_ctx.x1));
   _ctx.x2 = ((12553 + (27449 * _ctx.x2)) % 32768);
   float y2;
   y2 = (3.0517578125e-005f * int_to_float(_ctx.x2));
   float out;
   if(color > 0.5f){
      out = Noise_pinker(_ctx._inst190,(y1 + (- y2)));
   }
   else
   {
      out = (y1 + (- y2));
   }
   return (2.f * out);
}

void Svf__ctx_type_4_init(Svf__ctx_type_4 &_output_){
   Svf__ctx_type_4 _ctx;
   _ctx.z2 = 0.0f;
   _ctx.z1 = 0.0f;
   _ctx.inv_den = 0.0f;
   _ctx.g = 0.0f;
   Util__ctx_type_1_init(_ctx._inst23b);
   Util__ctx_type_1_init(_ctx._inst13b);
   _ctx.R = 0.0f;
   Svf_default(_ctx);
   _output_ = _ctx;
   return ;
}

float Svf_process(Svf__ctx_type_4 &_ctx, float x, float cv, float q, int sel){
   q = (0.5f + q);
   if(Util_change(_ctx._inst13b,cv) || Util_change(_ctx._inst23b,q)){
      _ctx.g = Svf_calc_g(cv);
      _ctx.R = (1.f / (2.f * (1e-018f + q)));
      _ctx.inv_den = (1.f / (1.f + (2.f * _ctx.R * _ctx.g) + (_ctx.g * _ctx.g)));
   }
   float high;
   high = (_ctx.inv_den * (x + (- _ctx.z2) + (- (_ctx.z1 * (_ctx.g + (2.f * _ctx.R))))));
   float band;
   band = (_ctx.z1 + (_ctx.g * high));
   float low;
   low = (_ctx.z2 + (_ctx.g * band));
   float notch;
   notch = (high + low);
   _ctx.z1 = (band + (_ctx.g * high));
   _ctx.z2 = (low + (_ctx.g * band));
   float output;
   switch(sel) {
      case 0:
         output = low;
      break;
      case 1:
         output = high;
      break;
      case 2:
         output = band;
      break;
    default: 
      output = notch;
   }
   return Saturate_soft_process(output);
}

void Ladder__ctx_type_4_init(Ladder__ctx_type_4 &_output_){
   Ladder__ctx_type_4 _ctx;
   _ctx.p3 = 0.0f;
   _ctx.p2 = 0.0f;
   _ctx.p1 = 0.0f;
   _ctx.p0 = 0.0f;
   _output_ = _ctx;
   return ;
}

float Ladder_heun(Ladder__ctx_type_4 &_ctx, float input, float fh, float res){
   float wt0;
   wt0 = Util_cubic_clipper((input + (-4.f * _ctx.p3 * res)));
   float wt1;
   wt1 = Util_cubic_clipper(_ctx.p0);
   float dpt0;
   dpt0 = (fh * (wt0 + (- wt1)));
   float wt3;
   wt3 = Util_cubic_clipper(_ctx.p1);
   float dpt1;
   dpt1 = (fh * (wt1 + (- wt3)));
   float wt5;
   wt5 = Util_cubic_clipper(_ctx.p2);
   float dpt2;
   dpt2 = (fh * (wt3 + (- wt5)));
   float wt7;
   wt7 = Util_cubic_clipper(_ctx.p3);
   float dpt3;
   dpt3 = (fh * (wt5 + (- wt7)));
   float pt0;
   pt0 = (_ctx.p0 + dpt0);
   float pt1;
   pt1 = (_ctx.p1 + dpt1);
   float pt2;
   pt2 = (_ctx.p2 + dpt2);
   float pt3;
   pt3 = (_ctx.p3 + dpt3);
   float w0;
   w0 = Util_cubic_clipper((input + (-4.f * pt3 * res)));
   float w1;
   w1 = Util_cubic_clipper(pt0);
   float dp0;
   dp0 = (fh * (w0 + (- w1)));
   float w3;
   w3 = Util_cubic_clipper(pt1);
   float dp1;
   dp1 = (fh * (w1 + (- w3)));
   float w5;
   w5 = Util_cubic_clipper(pt2);
   float dp2;
   dp2 = (fh * (w3 + (- w5)));
   float w7;
   w7 = Util_cubic_clipper(pt3);
   float dp3;
   dp3 = (fh * (w5 + (- w7)));
   _ctx.p0 = (_ctx.p0 + (0.5f * (dp0 + dpt0)));
   _ctx.p1 = (_ctx.p1 + (0.5f * (dp1 + dpt1)));
   _ctx.p2 = (_ctx.p2 + (0.5f * (dp2 + dpt2)));
   _ctx.p3 = (_ctx.p3 + (0.5f * (dp3 + dpt3)));
   return _ctx.p3;
}

void Ladder__ctx_type_5_init(Ladder__ctx_type_5 &_output_){
   Ladder__ctx_type_5 _ctx;
   _ctx.p3 = 0.0f;
   _ctx.p2 = 0.0f;
   _ctx.p1 = 0.0f;
   _ctx.p0 = 0.0f;
   _output_ = _ctx;
   return ;
}

float Ladder_euler(Ladder__ctx_type_5 &_ctx, float input, float fh, float res){
   float w0;
   w0 = Util_cubic_clipper((input + (-4.f * _ctx.p3 * res)));
   float w1;
   w1 = Util_cubic_clipper(_ctx.p0);
   float dpt0;
   dpt0 = (fh * (w0 + (- w1)));
   float w3;
   w3 = Util_cubic_clipper(_ctx.p1);
   float dpt1;
   dpt1 = (fh * (w1 + (- w3)));
   float w5;
   w5 = Util_cubic_clipper(_ctx.p2);
   float dpt2;
   dpt2 = (fh * (w3 + (- w5)));
   float w7;
   w7 = Util_cubic_clipper(_ctx.p3);
   float dpt3;
   dpt3 = (fh * (w5 + (- w7)));
   _ctx.p0 = (_ctx.p0 + dpt0);
   _ctx.p1 = (_ctx.p1 + dpt1);
   _ctx.p2 = (_ctx.p2 + dpt2);
   _ctx.p3 = (_ctx.p3 + dpt3);
   return _ctx.p3;
}

void Ladder__ctx_type_6_init(Ladder__ctx_type_6 &_output_){
   Ladder__ctx_type_6 _ctx;
   _ctx.fh = 0.0f;
   Ladder__ctx_type_5_init(_ctx.e);
   Util__ctx_type_1_init(_ctx._inst13b);
   _output_ = _ctx;
   return ;
}

float Ladder_process_euler(Ladder__ctx_type_6 &_ctx, float input, float cut, float res){
   if(Util_change(_ctx._inst13b,cut)){
      _ctx.fh = Ladder_tune(cut);
   }
   Ladder_euler(_ctx.e,input,_ctx.fh,res);
   Ladder_euler(_ctx.e,input,_ctx.fh,res);
   Ladder_euler(_ctx.e,input,_ctx.fh,res);
   float out;
   out = Ladder_euler(_ctx.e,input,_ctx.fh,res);
   return out;
}

void Ladder__ctx_type_7_init(Ladder__ctx_type_7 &_output_){
   Ladder__ctx_type_7 _ctx;
   Ladder__ctx_type_4_init(_ctx.h);
   _ctx.fh = 0.0f;
   Util__ctx_type_1_init(_ctx._inst13b);
   _output_ = _ctx;
   return ;
}

float Ladder_process_heun(Ladder__ctx_type_7 &_ctx, float input, float cut, float res){
   if(Util_change(_ctx._inst13b,cut)){
      _ctx.fh = Ladder_tune(cut);
   }
   Ladder_heun(_ctx.h,input,_ctx.fh,res);
   Ladder_heun(_ctx.h,input,_ctx.fh,res);
   Ladder_heun(_ctx.h,input,_ctx.fh,res);
   float out;
   out = Ladder_heun(_ctx.h,input,_ctx.fh,res);
   return out;
}

float Bitcrush_process(float i, float cv){
   float out;
   out = i;
   if(cv == 0.0f){
      out = i;
   }
   else
   {
      float b;
      b = Bitcrush_factor(cv);
      int x;
      x = float_to_int((b * i));
      out = (int_to_float(x) / b);
   }
   return out;
}

void Phase__ctx_type_0_init(Phase__ctx_type_0 &_output_){
   Phase__ctx_type_0 _ctx;
   _ctx.rate = 0.0f;
   _ctx.phase = 0.0f;
   Util__ctx_type_0_init(_ctx._inst351);
   Util__ctx_type_1_init(_ctx._inst13b);
   Phase_default(_ctx);
   _output_ = _ctx;
   return ;
}

float Phase_process(Phase__ctx_type_0 &_ctx, float cv, float reset){
   if(Util_change(_ctx._inst13b,cv)){
      _ctx.rate = Util_cvToRate_1024(cv);
   }
   if(Util_edge(_ctx._inst351,(reset > 0.5f))){
      _ctx.phase = 0.0f;
   }
   else
   {
      _ctx.phase = (_ctx.phase + _ctx.rate);
   }
   if(_ctx.phase > 1024.f){
      _ctx.phase = (-1024.f + _ctx.phase);
   }
   return (0.0009765625f * _ctx.phase);
}

void Rescomb__ctx_type_0_init(Rescomb__ctx_type_0 &_output_){
  Rescomb__ctx_type_0 _ctx;
  _ctx.write_pos = 0;
  /* moved this to caller!
  _ctx.bufferptr = (float*)heaps::malloc(sizeof(float)*675, MALLOC_CAP_8BIT);
  float_init_array(675,0.0f,_ctx.bufferptr);
   */
  _output_ = _ctx;
  return ;
}

float Rescomb_delay(Rescomb__ctx_type_0 &_ctx, float x, float cv){
   _ctx.write_pos = ((1 + _ctx.write_pos) % 675);
   float_set(_ctx.bufferptr,_ctx.write_pos,x);
   float r_size;
   r_size = 675.f;
   float r_index;
   r_index = fmodf((int_to_float(_ctx.write_pos) + (- Util_cvToperiod(cv))),r_size);
   if(r_index < 0.0f){
      r_index = (r_index + r_size);
   }
   int t1;
   t1 = (float_to_int(floorf(r_index)) % 675);
   int t2;
   t2 = ((1 + t1) % 675);
   float decimal;
   decimal = (r_index + (- int_to_float(t1)));
   float x1;
   x1 = float_get(_ctx.bufferptr,t1);
   float x2;
   x2 = float_get(_ctx.bufferptr,t2);
   float ret;
   ret = (x1 + (decimal * (x2 + (- x1))));
   return ret;
}

void Rescomb__ctx_type_5_init(Rescomb__ctx_type_5 &_output_){
   Rescomb__ctx_type_5 _ctx;
   _ctx.stone = 0.0f;
   _ctx.output = 0.0f;
   Rescomb__ctx_type_0_init(_ctx._inst47a);
   Util__ctx_type_3_init(_ctx._inst37d);
   Util__ctx_type_1_init(_ctx._inst13b);
   _output_ = _ctx;
   return ;
}

float Rescomb_do(Rescomb__ctx_type_5 &_ctx, float in, float cv, float tone, float res){
   if(Util_change(_ctx._inst13b,tone)){
      _ctx.stone = Rescomb_toneCurve(tone);
   }
   float feedback;
   feedback = Util_dcblock(_ctx._inst37d,(_ctx.output * res));
   float saturated_input;
   saturated_input = Saturate_soft_process((feedback + in));
   _ctx.output = (in + (_ctx.stone * Rescomb_delay(_ctx._inst47a,saturated_input,cv)));
   return Saturate_soft_process(_ctx.output);
}

void Phasedist_real__ctx_type_2_init(Phasedist_real__ctx_type_2 &_output_){
   Phasedist_real__ctx_type_2 _ctx;
   _ctx.rate = 0.0f;
   _ctx.phase = 0.0f;
   Phasedist_real__ctx_type_0_init(_ctx._inst12a);
   _output_ = _ctx;
   return ;
}

float Phasedist_real_phasor(Phasedist_real__ctx_type_2 &_ctx, float pitch, uint8_t reset){
   if(Phasedist_real_change(_ctx._inst12a,pitch)){
      _ctx.rate = Phasedist_real_pitchToRate(pitch);
   }
   if(reset){
      _ctx.phase = 0.0f;
   }
   else
   {
      _ctx.phase = fmodf((_ctx.phase + _ctx.rate),1.f);
   }
   return _ctx.phase;
}

void Phasedist_real__ctx_type_3_init(Phasedist_real__ctx_type_3 &_output_){
   Phasedist_real__ctx_type_3 _ctx;
   _ctx.pre_phase1 = 0.0f;
   _ctx.pitch = 0.0f;
   _ctx.detune = 0.0f;
   Phasedist_real__ctx_type_2_init(_ctx._inst2bf);
   Phasedist_real__ctx_type_2_init(_ctx._inst1bf);
   _output_ = _ctx;
   return ;
}

float Phasedist_real_process(Phasedist_real__ctx_type_3 &_ctx, float input){
   float phase1;
   phase1 = Phasedist_real_phasor(_ctx._inst1bf,_ctx.pitch,false);
   float comp;
   comp = (1.f + (- phase1));
   uint8_t reset;
   reset = ((_ctx.pre_phase1 + (- phase1)) > 0.5f);
   _ctx.pre_phase1 = phase1;
   float phase2;
   phase2 = Phasedist_real_phasor(_ctx._inst2bf,(_ctx.pitch + (32.f * _ctx.detune)),reset);
   float sine;
   sine = sinf((6.28318530718f * phase2));
   return (comp * sine);
}

float Fold_do(float signal, float level){
   float sign;
   if(signal > 0.0f){
      sign = 1.f;
   }
   else
   {
      sign = -1.f;
   }
   float amp;
   amp = (fabsf(signal) * (1.f + (8.f * level)));
   float base;
   base = floorf(amp);
   float delta;
   delta = (amp + (- base));
   float out;
   if((float_to_int(base) % 2) != 0){
      out = (1.f + (- delta));
   }
   else
   {
      out = delta;
   }
   return (out * sign);
}

void Saw_eptr__ctx_type_0_init(Saw_eptr__ctx_type_0 &_output_){
   Saw_eptr__ctx_type_0 _ctx;
   _ctx.rate = 0.0f;
   _ctx.phase = 0.0f;
   Util__ctx_type_1_init(_ctx._inst13b);
   Saw_eptr_default(_ctx);
   _output_ = _ctx;
   return ;
}

float Saw_eptr_process(Saw_eptr__ctx_type_0 &_ctx, float cv){
   float y;
   y = 0.0f;
   if(Util_change(_ctx._inst13b,cv)){
      _ctx.rate = Util_cvToRate(cv);
   }
   _ctx.phase = (_ctx.phase + (2.f * _ctx.rate));
   if(_ctx.phase > (1.f + (- _ctx.rate))){
      y = (-1.f + _ctx.phase + (- (_ctx.phase / _ctx.rate)) + (1.f / _ctx.rate));
      _ctx.phase = (-2.f + _ctx.phase);
   }
   else
   {
      y = _ctx.phase;
   }
   return y;
}
float Saw_eptr_process_slow(Saw_eptr__ctx_type_0 &_ctx, float cv){
  float y;
  y = 0.0f;
  if(Util_change(_ctx._inst13b,cv)){
    _ctx.rate = Util_cvToRate(cv)/25.f;
  }
  _ctx.phase = (_ctx.phase + (2.f * _ctx.rate));
  if(_ctx.phase > (1.f + (- _ctx.rate))){
    y = (-1.f + _ctx.phase + (- (_ctx.phase / _ctx.rate)) + (1.f / _ctx.rate));
    _ctx.phase = (-2.f + _ctx.phase);
  }
  else
  {
    y = _ctx.phase;
  }
  return y;
}



void Blit__ctx_type_0_init(Blit__ctx_type_0 &_output_){
   Blit__ctx_type_0 _ctx;
   _ctx.scale = 0.0f;
   _ctx.m = 0;
   Util__ctx_type_1_init(_ctx._inst13b);
   Blit_pulse_start(_ctx);
   _output_ = _ctx;
   return ;
}

float Blit_pulse_train(Blit__ctx_type_0 &_ctx, float cv, float phase){
   if(Util_change(_ctx._inst13b,cv)){
      _ctx.m = (1 + (float_to_int(Util_cvToperiod(cv)) << 1));
      _ctx.scale = (2.f * Util_cvToperiod(cv));
   }
   float pi_phase;
   pi_phase = (0.5f * phase);
   float denominator;
   denominator = Tables_nsine(pi_phase);
   float out;
   out = 0.0f;
   if(denominator <= 1e-018f){
      out = 1.f;
   }
   else
   {
      float m_phase;
      m_phase = fmodf((pi_phase * int_to_float(_ctx.m)),1.f);
      float numerator;
      numerator = Tables_nsine(m_phase);
      out = (numerator / (_ctx.scale * denominator));
   }
   return out;
}

void Blit__ctx_type_1_init(Blit__ctx_type_1 &_output_){
   Blit__ctx_type_1 _ctx;
   _ctx.state_saw = 0.0f;
   _ctx.state_pulse = 0.0f;
   _ctx.rate = 0.0f;
   _ctx.output = 0.0f;
   Blit__ctx_type_0_init(_ctx._inst846);
   Blit__ctx_type_0_init(_ctx._inst746);
   Phase__ctx_type_0_init(_ctx._inst5cd);
   Util__ctx_type_1_init(_ctx._inst33b);
   Util__ctx_type_3_init(_ctx._inst117d);
   Blit_init(_ctx);
   _output_ = _ctx;
   return ;
}

float Blit_osc_blit(Blit__ctx_type_1 &_ctx, float cv, float pw, float wave){
   float fixed_cv;
   fixed_cv = 0.0f;
   if(wave < 1.f){
      fixed_cv = cv;
   }
   else
   {
      fixed_cv = (-0.1f + cv);
   }
   if(Util_change(_ctx._inst33b,fixed_cv)){
      _ctx.rate = Util_cvToRate(fixed_cv);
   }
   float phase;
   phase = Phase_process(_ctx._inst5cd,fixed_cv,0.0f);
   float shift05;
   shift05 = (0.5f + (0.49f * pw));
   float shift;
   shift = (phase + shift05);
   if(shift > 1.f){
      shift = (-1.f + shift);
   }
   float pulse1;
   pulse1 = Blit_pulse_train(_ctx._inst746,fixed_cv,phase);
   float pulse2;
   pulse2 = Blit_pulse_train(_ctx._inst846,fixed_cv,shift);
   _ctx.state_pulse = (pulse1 + (- pulse2) + (0.9995f * _ctx.state_pulse));
   _ctx.state_saw = ((0.5f * ((pulse1 + pulse2 + (-2.f * _ctx.rate)) / shift05)) + (0.9995f * _ctx.state_saw));
   if(wave < 1.f){
      _ctx.output = _ctx.state_pulse;
   }
   else
   {
      _ctx.output = (2.f * _ctx.state_saw);
   }
   _ctx.output = Util_dcblock(_ctx._inst117d,_ctx.output);
   return _ctx.output;
}


