// Copyright 2014 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Reverb (used in the dedicated "Oliverb" playback mode).
// Based on https://github.com/mqtthiqs/parasites/blob/master/clouds/dsp/fx/oliverb.h
// By Matthias Puech 2016

#pragma once

#include "stmlib/stmlib.h"

#include "fx_engine.h"
#include "clouds/dsp/frame.h"
#include "random_oscillator.h"

namespace mifx {

    class Oliverb {
    public:
        Oliverb() { }
        ~Oliverb() { }

        void Init(float* buffer) {
            engine_.Init(buffer);
            diffusion_ = 0.625f;
            size_ = 0.5f;
            mod_amount_ = 0.0f;
            mod_rate_ = 0.0f;
            size_ = 0.5f;
            input_gain_ = 1.0f;
            decay_ = 0.5f;
            lp_ = 1.0f;
            hp_= 0.0f;
            phase_ = 0.0f;
            ratio_ = 0.0f;
            pitch_shift_amount_ = 1.0f;
            level_ = 0.0f;
            for (int i=0; i<9; i++)
                lfo_[i].Init();
        }

        void Process(clouds::FloatFrame* in_out, size_t size) {
            // This is the Griesinger topology described in the Dattorro paper
            // (4 AP diffusers on the input, then a loop of 2x 2AP+1Delay).
            // Modulation is applied in the loop of the first diffuser AP for additional
            // smearing; and to the two long delays for a slow shimmer/chorus effect.
            typedef E::Reserve<150,
                    E::Reserve<214,
                            E::Reserve < 319,
                            E::Reserve < 527,
                            E::Reserve < 2182,
                            E::Reserve < 2690,
                            E::Reserve < 4501,
                            E::Reserve < 2525,
                            E::Reserve < 2197,
                            E::Reserve < 6312> > > > > > > > > > Memory;
            E::DelayLine<Memory, 0> ap1;
            E::DelayLine<Memory, 1> ap2;
            E::DelayLine<Memory, 2> ap3;
            E::DelayLine<Memory, 3> ap4;
            E::DelayLine<Memory, 4> dap1a;
            E::DelayLine<Memory, 5> dap1b;
            E::DelayLine<Memory, 6> del1;
            E::DelayLine<Memory, 7> dap2a;
            E::DelayLine<Memory, 8> dap2b;
            E::DelayLine<Memory, 9> del2;
            E::Context c;

            const float kap = diffusion_;
            const float amount = amount_;

            float lp_1 = lp_decay_1_;
            float lp_2 = lp_decay_2_;
            float hp_1 = hp_decay_1_;
            float hp_2 = hp_decay_2_;

            /* Set frequency of LFOs */
            float slope = mod_rate_ * mod_rate_;
            slope *= slope * slope;
            slope /= 200.0f;
            for (int i=0; i<9; i++)
                lfo_[i].set_slope(slope);

            while (size--) {
                float wet;
                engine_.Start(&c);

                // Smooth parameters to avoid delay glitches
                ONE_POLE(smooth_size_, size_, 0.01f);

                // compute windowing info for the pitch shifter
                float ps_size = 128.0f + (3410.0f - 128.0f) * smooth_size_;
                phase_ += (1.0f - ratio_) / ps_size;
                if (phase_ >= 1.0f) phase_ -= 1.0f;
                if (phase_ <= 0.0f) phase_ += 1.0f;
                float tri = 2.0f * (phase_ >= 0.5f ? 1.0f - phase_ : phase_);
                tri = Interpolate(clouds::lut_window, tri, LUT_WINDOW_SIZE-1);
                float phase = phase_ * ps_size;
                float half = phase + ps_size * 0.5f;
                if (half >= ps_size) half -= ps_size;

#define INTERPOLATE_LFO(del, lfo, gain)                                 \
      {                                                                 \
        float offset = (del.length - 1) * smooth_size_;                 \
        offset += lfo.Next() * mod_amount_;                      \
        CONSTRAIN(offset, 1.0f, del.length - 1);                        \
        c.InterpolateHermite(del, offset, gain);                        \
      }

#define INTERPOLATE(del, gain)                                          \
      {                                                                 \
        float offset = (del.length - 1) * smooth_size_;                 \
        CONSTRAIN(offset, 1.0f, del.length - 1);                        \
        c.InterpolateHermite(del, offset, gain);                        \
      }

                // Smear AP1 inside the loop.
                c.Interpolate(ap1, 10.0f, LFO_1, 60.0f, 1.0f);
                c.Write(ap1, 100, 0.0f);

                c.Read(in_out->l + in_out->r, input_gain_);
                // Diffuse through 4 allpasses.
                INTERPOLATE_LFO(ap1, lfo_[1], kap);
                c.WriteAllPass(ap1, -kap);
                INTERPOLATE_LFO(ap2, lfo_[2], kap);
                c.WriteAllPass(ap2, -kap);
                INTERPOLATE_LFO(ap3, lfo_[3], kap);
                c.WriteAllPass(ap3, -kap);
                INTERPOLATE_LFO(ap4, lfo_[4], kap);
                c.WriteAllPass(ap4, -kap);

                float apout;
                c.Write(apout);

                INTERPOLATE_LFO(del2, lfo_[5], decay_ * (1.0f - pitch_shift_amount_));
                /* blend in the pitch shifted feedback */
                c.InterpolateHermite(del2, phase, tri * decay_ * pitch_shift_amount_);
                c.InterpolateHermite(del2, half, (1.0f - tri) * decay_ * pitch_shift_amount_);

                c.Lp(lp_1, lp_);
                c.Hp(hp_1, hp_);
                c.SoftLimit();
                INTERPOLATE_LFO(dap1a, lfo_[6], -kap);
                c.WriteAllPass(dap1a, kap);
                INTERPOLATE(dap1b, kap);
                c.WriteAllPass(dap1b, -kap);
                c.Write(del1, 2.0f);
                c.Write(wet, 0.0f);

                in_out->l += (wet - in_out->l) * amount;

                c.Load(apout);

                INTERPOLATE_LFO(del1, lfo_[7], decay_ * (1.0f - pitch_shift_amount_));
                /* blend in the pitch shifted feedback */
                c.InterpolateHermite(del1, phase, tri * decay_ * pitch_shift_amount_);
                c.InterpolateHermite(del1, half, (1.0f - tri) * decay_ * pitch_shift_amount_);
                c.Lp(lp_2, lp_);
                c.Hp(hp_2, hp_);
                c.SoftLimit();
                INTERPOLATE_LFO(dap2a, lfo_[8], kap);
                c.WriteAllPass(dap2a, -kap);
                INTERPOLATE(dap2b, -kap);
                c.WriteAllPass(dap2b, kap);
                c.Write(del2, 2.0f);
                c.Write(wet, 0.0f);

                in_out->r += (wet - in_out->r) * amount;

                ++in_out;
            }

            lp_decay_1_ = lp_1;
            lp_decay_2_ = lp_2;
            hp_decay_1_ = hp_1;
            hp_decay_2_ = hp_2;
        }

        inline void set_input_gain(float input_gain) {
            input_gain_ = input_gain;
        }

        inline void set_amount(float amount) {
            amount_ = amount;
        }

        inline void set_decay(float decay) {
            decay_ = decay;
        }

        inline void set_diffusion(float diffusion) {
            diffusion_ = diffusion;
        }

        inline void set_lp(float lp) {
            lp_ = lp;
        }

        inline void set_hp(float hp) {
            hp_ = hp;
        }

        inline void set_size(float size) {
            size_ = size;
        }

        inline void set_mod_amount(float mod_amount) {
            mod_amount_ = mod_amount;
        }

        inline void set_mod_rate(float mod_rate) {
            mod_rate_ = mod_rate;
        }

        inline void set_ratio(float ratio) {
            ratio_ = ratio;
        }

        inline void set_pitch_shift_amount(float pitch_shift) {
            pitch_shift_amount_ = pitch_shift;
        }

    private:
        typedef FxEngine<32768, FORMAT_32_BIT> E;
        E engine_;

        float input_gain_ = 0.f;
        float decay_ = 0.f;
        float diffusion_ = 0.f;
        float lp_ = 0.f;
        float hp_ = 0.f;
        float size_ = 0.f, smooth_size_ = 0.f;
        float mod_amount_ = 0.f;
        float mod_rate_ = 0.f;
        float pitch_shift_amount_ = 0.f;

        float lp_decay_1_ = 0.f;
        float lp_decay_2_ = 0.f;
        float hp_decay_1_ = 0.f;
        float hp_decay_2_ = 0.f;

        float phase_ = 0.f;
        float ratio_ = 0.f;
        float level_ = 0.f;
        float amount_ = 0.f;

        RandomOscillator lfo_[9];

        DISALLOW_COPY_AND_ASSIGN(Oliverb);
    };

}  // namespace