
/* Code automatically generated by Vult https://github.com/modlfo/vult */
#ifndef VULT_FORMANTOR_H
#define VULT_FORMANTOR_H
#include <stdint.h>
#include <math.h>
#include "../vult/vultin.h"
#include "./vult/vult_formantor.tables.h"

typedef struct Util__ctx_type_0 {
   uint8_t pre;
} Util__ctx_type_0;

typedef Util__ctx_type_0 Util_edge_type;

static_inline void Util__ctx_type_0_init(Util__ctx_type_0 &_output_){
   Util__ctx_type_0 _ctx;
   _ctx.pre = false;
   _output_ = _ctx;
   return ;
}

static_inline void Util_edge_init(Util__ctx_type_0 &_output_){
   Util__ctx_type_0_init(_output_);
   return ;
}

static_inline uint8_t Util_edge(Util__ctx_type_0 &_ctx, uint8_t x){
   uint8_t ret;
   ret = (x && bool_not(_ctx.pre));
   _ctx.pre = x;
   return ret;
}

typedef struct Util__ctx_type_1 {
   float pre_x;
} Util__ctx_type_1;

typedef Util__ctx_type_1 Util_change_type;

static_inline void Util__ctx_type_1_init(Util__ctx_type_1 &_output_){
   Util__ctx_type_1 _ctx;
   _ctx.pre_x = 0.0f;
   _output_ = _ctx;
   return ;
}

static_inline void Util_change_init(Util__ctx_type_1 &_output_){
   Util__ctx_type_1_init(_output_);
   return ;
}

static_inline uint8_t Util_change(Util__ctx_type_1 &_ctx, float x){
   uint8_t v;
   v = (_ctx.pre_x != x);
   _ctx.pre_x = x;
   return v;
}

static_inline float Util_map(float x, float x0, float x1, float y0, float y1){
   return (y0 + (((x + (- x0)) * (y1 + (- y0))) / (x1 + (- x0))));
};

typedef struct Util__ctx_type_3 {
   float y1;
   float x1;
} Util__ctx_type_3;

typedef Util__ctx_type_3 Util_dcblock_type;

void Util__ctx_type_3_init(Util__ctx_type_3 &_output_);

static_inline void Util_dcblock_init(Util__ctx_type_3 &_output_){
   Util__ctx_type_3_init(_output_);
   return ;
}

float Util_dcblock(Util__ctx_type_3 &_ctx, float x0);

typedef struct Util__ctx_type_4 {
   float x;
} Util__ctx_type_4;

typedef Util__ctx_type_4 Util_smooth_type;

static_inline void Util__ctx_type_4_init(Util__ctx_type_4 &_output_){
   Util__ctx_type_4 _ctx;
   _ctx.x = 0.0f;
   _output_ = _ctx;
   return ;
}

static_inline void Util_smooth_init(Util__ctx_type_4 &_output_){
   Util__ctx_type_4_init(_output_);
   return ;
}

static_inline float Util_smooth(Util__ctx_type_4 &_ctx, float input){
   _ctx.x = (_ctx.x + (0.005f * (input + (- _ctx.x))));
   return _ctx.x;
}

typedef struct Util__ctx_type_5 {
   float x0;
} Util__ctx_type_5;

typedef Util__ctx_type_5 Util_average2_type;

static_inline void Util__ctx_type_5_init(Util__ctx_type_5 &_output_){
   Util__ctx_type_5 _ctx;
   _ctx.x0 = 0.0f;
   _output_ = _ctx;
   return ;
}

static_inline void Util_average2_init(Util__ctx_type_5 &_output_){
   Util__ctx_type_5_init(_output_);
   return ;
}

static_inline float Util_average2(Util__ctx_type_5 &_ctx, float x1){
   float result;
   result = (0.5f * (_ctx.x0 + x1));
   _ctx.x0 = x1;
   return result;
}

static_inline float Util_cubic_clipper(float x){
   if(x <= -0.666666666667f){
      return -0.666666666667f;
   }
   else
   {
      if(x >= 0.666666666667f){
         return 0.666666666667f;
      }
      else
      {
         return (x + (-0.333333333333f * x * x * x));
      }
   }
};

static_inline float Util_pitchToRate_1024_raw_c0(int index){
   return Util_pitchToRate_1024_c0[index];
};

static_inline float Util_pitchToRate_1024_raw_c1(int index){
   return Util_pitchToRate_1024_c1[index];
};

static_inline float Util_pitchToRate_1024_raw_c2(int index){
   return Util_pitchToRate_1024_c2[index];
};

static_inline float Util_pitchToRate_1024(float pitch){
   int index;
   index = int_clip(float_to_int((0.244094488189f * pitch)),0,31);
   return (float_wrap_array(Util_pitchToRate_1024_c0)[index] + (pitch * (float_wrap_array(Util_pitchToRate_1024_c1)[index] + (pitch * float_wrap_array(Util_pitchToRate_1024_c2)[index]))));
}

static_inline float Util_pitchToRate_raw_c0(int index){
   return Util_pitchToRate_c0[index];
};

static_inline float Util_pitchToRate_raw_c1(int index){
   return Util_pitchToRate_c1[index];
};

static_inline float Util_pitchToRate_raw_c2(int index){
   return Util_pitchToRate_c2[index];
};

static_inline float Util_pitchToRate(float pitch){
   int index;
   index = int_clip(float_to_int((0.244094488189f * pitch)),0,31);
   return (float_wrap_array(Util_pitchToRate_c0)[index] + (pitch * (float_wrap_array(Util_pitchToRate_c1)[index] + (pitch * float_wrap_array(Util_pitchToRate_c2)[index]))));
}

static_inline float Util_cvToPitch(float cv){
   return (24.f + (120.f * cv));
};

static_inline float Util_cvToRate_1024_raw_c0(int index){
   return Util_cvToRate_1024_c0[index];
};

static_inline float Util_cvToRate_1024_raw_c1(int index){
   return Util_cvToRate_1024_c1[index];
};

static_inline float Util_cvToRate_1024_raw_c2(int index){
   return Util_cvToRate_1024_c2[index];
};

static_inline float Util_cvToRate_1024(float cv){
   int index;
   index = int_clip(float_to_int((34.4444444444f * cv)),0,31);
   return (float_wrap_array(Util_cvToRate_1024_c0)[index] + (cv * (float_wrap_array(Util_cvToRate_1024_c1)[index] + (cv * float_wrap_array(Util_cvToRate_1024_c2)[index]))));
}

static_inline float Util_cvToRate_raw_c0(int index){
   return Util_cvToRate_c0[index];
};

static_inline float Util_cvToRate_raw_c1(int index){
   return Util_cvToRate_c1[index];
};

static_inline float Util_cvToRate_raw_c2(int index){
   return Util_cvToRate_c2[index];
};

static_inline float Util_cvToRate(float cv){
   int index;
   index = int_clip(float_to_int((141.111111111f * cv)),0,127);
   return (float_wrap_array(Util_cvToRate_c0)[index] + (cv * (float_wrap_array(Util_cvToRate_c1)[index] + (cv * float_wrap_array(Util_cvToRate_c2)[index]))));
}

static_inline float Util_pitchToCv(float pitch){
   return (0.00833333333333f * (-24.f + pitch));
};

static_inline float Util_cvToperiod_raw_c0(int index){
   return Util_cvToperiod_c0[index];
};

static_inline float Util_cvToperiod_raw_c1(int index){
   return Util_cvToperiod_c1[index];
};

static_inline float Util_cvToperiod_raw_c2(int index){
   return Util_cvToperiod_c2[index];
};

static_inline float Util_cvToperiod(float cv){
   int index;
   index = int_clip(float_to_int((31.f * cv)),0,31);
   return (float_wrap_array(Util_cvToperiod_c0)[index] + (cv * (float_wrap_array(Util_cvToperiod_c1)[index] + (cv * float_wrap_array(Util_cvToperiod_c2)[index]))));
}

static_inline float Util_cvTokHz_raw_c0(int index){
   return Util_cvTokHz_c0[index];
};

static_inline float Util_cvTokHz_raw_c1(int index){
   return Util_cvTokHz_c1[index];
};

static_inline float Util_cvTokHz_raw_c2(int index){
   return Util_cvTokHz_c2[index];
};

static_inline float Util_cvTokHz(float cv){
   int index;
   index = int_clip(float_to_int((31.f * cv)),0,31);
   return (float_wrap_array(Util_cvTokHz_c0)[index] + (cv * (float_wrap_array(Util_cvTokHz_c1)[index] + (cv * float_wrap_array(Util_cvTokHz_c2)[index]))));
}

static_inline float Saturate_soft_tanh_table_raw_c0(int index){
   return Saturate_soft_tanh_table_c0[index];
};

static_inline float Saturate_soft_tanh_table_raw_c1(int index){
   return Saturate_soft_tanh_table_c1[index];
};

static_inline float Saturate_soft_tanh_table_raw_c2(int index){
   return Saturate_soft_tanh_table_c2[index];
};

static_inline float Saturate_soft_tanh_table(float x){
   int index;
   index = int_clip(float_to_int((5.f * (24.f + x))),0,240);
   return (float_wrap_array(Saturate_soft_tanh_table_c0)[index] + (x * (float_wrap_array(Saturate_soft_tanh_table_c1)[index] + (x * float_wrap_array(Saturate_soft_tanh_table_c2)[index]))));
}

static_inline float Saturate_soft_process(float x){
   return Saturate_soft_tanh_table(x);
};

static_inline void Saturate_soft_noteOn(int note, int velocity, int channel){
}

static_inline void Saturate_soft_noteOff(int note, int channel){
}

static_inline void Saturate_soft_controlChange(int control, int value, int channel){
}

static_inline void Saturate_soft_default(){
}

static_inline float Svf_calc_g_raw_c0(int index){
   return Svf_calc_g_c0[index];
};

static_inline float Svf_calc_g_raw_c1(int index){
   return Svf_calc_g_c1[index];
};

static_inline float Svf_calc_g_raw_c2(int index){
   return Svf_calc_g_c2[index];
};

static_inline float Svf_calc_g(float cv){
   int index;
   index = int_clip(float_to_int((141.111111111f * cv)),0,127);
   return (float_wrap_array(Svf_calc_g_c0)[index] + (cv * (float_wrap_array(Svf_calc_g_c1)[index] + (cv * float_wrap_array(Svf_calc_g_c2)[index]))));
}

typedef struct Svf__ctx_type_4 {
   float z2;
   float z1;
   float inv_den;
   float g;
   Util__ctx_type_1 _inst23b;
   Util__ctx_type_1 _inst13b;
   float R;
} Svf__ctx_type_4;

typedef Svf__ctx_type_4 Svf_process_type;

void Svf__ctx_type_4_init(Svf__ctx_type_4 &_output_);

static_inline void Svf_process_init(Svf__ctx_type_4 &_output_){
   Svf__ctx_type_4_init(_output_);
   return ;
}

float Svf_process(Svf__ctx_type_4 &_ctx, float x, float cv, float q, int sel);

typedef Svf__ctx_type_4 Svf_noteOn_type;

static_inline void Svf_noteOn_init(Svf__ctx_type_4 &_output_){
   Svf__ctx_type_4_init(_output_);
   return ;
}

static_inline void Svf_noteOn(Svf__ctx_type_4 &_ctx, int note, int velocity, int channel){
}

typedef Svf__ctx_type_4 Svf_noteOff_type;

static_inline void Svf_noteOff_init(Svf__ctx_type_4 &_output_){
   Svf__ctx_type_4_init(_output_);
   return ;
}

static_inline void Svf_noteOff(Svf__ctx_type_4 &_ctx, int note, int channel){
}

typedef Svf__ctx_type_4 Svf_controlChange_type;

static_inline void Svf_controlChange_init(Svf__ctx_type_4 &_output_){
   Svf__ctx_type_4_init(_output_);
   return ;
}

static_inline void Svf_controlChange(Svf__ctx_type_4 &_ctx, int control, int value, int channel){
}

typedef Svf__ctx_type_4 Svf_default_type;

static_inline void Svf_default_init(Svf__ctx_type_4 &_output_){
   Svf__ctx_type_4_init(_output_);
   return ;
}

static_inline void Svf_default(Svf__ctx_type_4 &_ctx){
   _ctx.g = 0.0023297121342f;
   _ctx.R = 1.f;
   _ctx.inv_den = (1.f / (1.f + (_ctx.g * _ctx.g)));
}

typedef struct Rescomb__ctx_type_0 {
   int write_pos;
   float* bufferptr;
} Rescomb__ctx_type_0;


typedef Rescomb__ctx_type_0 Rescomb_delay_type;

void Rescomb__ctx_type_0_init(Rescomb__ctx_type_0 &_output_);

static_inline void Rescomb_delay_init(Rescomb__ctx_type_0 &_output_){
   Rescomb__ctx_type_0_init(_output_);
   return ;
}

float Rescomb_delay(Rescomb__ctx_type_0 &_ctx, float x, float cv);

static_inline float Rescomb_toneCurve_raw_c0(int index){
   return Rescomb_toneCurve_c0[index];
};

static_inline float Rescomb_toneCurve_raw_c1(int index){
   return Rescomb_toneCurve_c1[index];
};

static_inline float Rescomb_toneCurve_raw_c2(int index){
   return Rescomb_toneCurve_c2[index];
};

static_inline float Rescomb_toneCurve(float tone){
   int index;
   index = int_clip(float_to_int((26.25f * (1.2f + tone))),0,63);
   return (float_wrap_array(Rescomb_toneCurve_c0)[index] + (tone * (float_wrap_array(Rescomb_toneCurve_c1)[index] + (tone * float_wrap_array(Rescomb_toneCurve_c2)[index]))));
}

typedef struct Rescomb__ctx_type_5 {
   float stone;
   float output;
   Rescomb__ctx_type_0 _inst47a;
   Util__ctx_type_3 _inst37d;
   Util__ctx_type_1 _inst13b;
} Rescomb__ctx_type_5;

typedef Rescomb__ctx_type_5 Rescomb_do_type;

void Rescomb__ctx_type_5_init(Rescomb__ctx_type_5 &_output_);

static_inline void Rescomb_do_init(Rescomb__ctx_type_5 &_output_){
   Rescomb__ctx_type_5_init(_output_);
   return ;
}

float Rescomb_do(Rescomb__ctx_type_5 &_ctx, float in, float cv, float tone, float res);

typedef struct Rescomb__ctx_type_6 {
   Rescomb__ctx_type_5 _inst179;
} Rescomb__ctx_type_6;

typedef Rescomb__ctx_type_6 Rescomb_process_type;

static_inline void Rescomb__ctx_type_6_init(Rescomb__ctx_type_6 &_output_){
   Rescomb__ctx_type_6 _ctx;
   Rescomb__ctx_type_5_init(_ctx._inst179);
   _output_ = _ctx;
   return ;
}

static_inline void Rescomb_process_init(Rescomb__ctx_type_6 &_output_){
   Rescomb__ctx_type_6_init(_output_);
   return ;
}

static_inline float Rescomb_process(Rescomb__ctx_type_6 &_ctx, float in, float cv, float tone, float res){
   return Rescomb_do(_ctx._inst179,in,cv,tone,res);
};

typedef Rescomb__ctx_type_6 Rescomb_noteOn_type;

static_inline void Rescomb_noteOn_init(Rescomb__ctx_type_6 &_output_){
   Rescomb__ctx_type_6_init(_output_);
   return ;
}

static_inline void Rescomb_noteOn(Rescomb__ctx_type_6 &_ctx, int note, int velocity, int channel){
}

typedef Rescomb__ctx_type_6 Rescomb_noteOff_type;

static_inline void Rescomb_noteOff_init(Rescomb__ctx_type_6 &_output_){
   Rescomb__ctx_type_6_init(_output_);
   return ;
}

static_inline void Rescomb_noteOff(Rescomb__ctx_type_6 &_ctx, int note, int channel){
}

typedef Rescomb__ctx_type_6 Rescomb_controlChange_type;

static_inline void Rescomb_controlChange_init(Rescomb__ctx_type_6 &_output_){
   Rescomb__ctx_type_6_init(_output_);
   return ;
}

static_inline void Rescomb_controlChange(Rescomb__ctx_type_6 &_ctx, int control, int value, int channel){
}

typedef Rescomb__ctx_type_6 Rescomb_default_type;

static_inline void Rescomb_default_init(Rescomb__ctx_type_6 &_output_){
   Rescomb__ctx_type_6_init(_output_);
   return ;
}

static_inline void Rescomb_default(Rescomb__ctx_type_6 &_ctx){
}

typedef struct Phasedist_real__ctx_type_0 {
   float pre_x;
} Phasedist_real__ctx_type_0;

typedef Phasedist_real__ctx_type_0 Phasedist_real_change_type;

static_inline void Phasedist_real__ctx_type_0_init(Phasedist_real__ctx_type_0 &_output_){
   Phasedist_real__ctx_type_0 _ctx;
   _ctx.pre_x = 0.0f;
   _output_ = _ctx;
   return ;
}

static_inline void Phasedist_real_change_init(Phasedist_real__ctx_type_0 &_output_){
   Phasedist_real__ctx_type_0_init(_output_);
   return ;
}

static_inline uint8_t Phasedist_real_change(Phasedist_real__ctx_type_0 &_ctx, float x){
   uint8_t v;
   v = (_ctx.pre_x != x);
   _ctx.pre_x = x;
   return v;
}

static_inline float Phasedist_real_pitchToRate(float d){
   return (0.000185392290249f * expf((0.0577623f * d)));
};

typedef struct Phasedist_real__ctx_type_2 {
   float rate;
   float phase;
   Phasedist_real__ctx_type_0 _inst12a;
} Phasedist_real__ctx_type_2;

typedef Phasedist_real__ctx_type_2 Phasedist_real_phasor_type;

void Phasedist_real__ctx_type_2_init(Phasedist_real__ctx_type_2 &_output_);

static_inline void Phasedist_real_phasor_init(Phasedist_real__ctx_type_2 &_output_){
   Phasedist_real__ctx_type_2_init(_output_);
   return ;
}

float Phasedist_real_phasor(Phasedist_real__ctx_type_2 &_ctx, float pitch, uint8_t reset);

typedef struct Phasedist_real__ctx_type_3 {
   float pre_phase1;
   float pitch;
   float detune;
   Phasedist_real__ctx_type_2 _inst2bf;
   Phasedist_real__ctx_type_2 _inst1bf;
} Phasedist_real__ctx_type_3;

typedef Phasedist_real__ctx_type_3 Phasedist_real_process_type;

void Phasedist_real__ctx_type_3_init(Phasedist_real__ctx_type_3 &_output_);

static_inline void Phasedist_real_process_init(Phasedist_real__ctx_type_3 &_output_){
   Phasedist_real__ctx_type_3_init(_output_);
   return ;
}

float Phasedist_real_process(Phasedist_real__ctx_type_3 &_ctx, float input);

typedef Phasedist_real__ctx_type_3 Phasedist_real_noteOn_type;

static_inline void Phasedist_real_noteOn_init(Phasedist_real__ctx_type_3 &_output_){
   Phasedist_real__ctx_type_3_init(_output_);
   return ;
}

static_inline void Phasedist_real_noteOn(Phasedist_real__ctx_type_3 &_ctx, float note, int velocity, int channel){
   _ctx.pitch = note;
};

typedef Phasedist_real__ctx_type_3 Phasedist_real_noteOff_type;

static_inline void Phasedist_real_noteOff_init(Phasedist_real__ctx_type_3 &_output_){
   Phasedist_real__ctx_type_3_init(_output_);
   return ;
}

static_inline void Phasedist_real_noteOff(Phasedist_real__ctx_type_3 &_ctx, int note, int channel){
}

typedef Phasedist_real__ctx_type_3 Phasedist_real_controlChange_type;

static_inline void Phasedist_real_controlChange_init(Phasedist_real__ctx_type_3 &_output_){
   Phasedist_real__ctx_type_3_init(_output_);
   return ;
}

static_inline void Phasedist_real_controlChange(Phasedist_real__ctx_type_3 &_ctx, int control, float value, int channel){
   if(control == 31){
      _ctx.detune = value;
   }
};

typedef Phasedist_real__ctx_type_3 Phasedist_real_default_type;

static_inline void Phasedist_real_default_init(Phasedist_real__ctx_type_3 &_output_){
   Phasedist_real__ctx_type_3_init(_output_);
   return ;
}

static_inline void Phasedist_real_default(Phasedist_real__ctx_type_3 &_ctx){
   _ctx.pitch = 45.f;
   _ctx.detune = 0.0f;
}



#endif // VULT_FORMANTOR_H
