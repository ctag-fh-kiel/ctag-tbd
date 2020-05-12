// Copyright 2015 Matthias Puech.
//
// Author: Matthias Puech (matthias.puech@gmail.com)
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
// Smoothed random oscillator

#include "clouds/resources.h"
#include "stmlib/utils/random.h"
#include "stmlib/dsp/dsp.h"

#pragma once

#define LUT_RAISED_COS 1
#define LUT_RAISED_COS_SIZE 257

using namespace stmlib;

namespace mifx
{
    const float lut_raised_cos[] = {
            0.000000000e+00,  3.764908043e-05,  1.505906519e-04,  3.388077058e-04,
            6.022718974e-04,  9.409435499e-04,  1.354771661e-03,  1.843693909e-03,
            2.407636664e-03,  3.046514999e-03,  3.760232701e-03,  4.548682286e-03,
            5.411745018e-03,  6.349290921e-03,  7.361178806e-03,  8.447256284e-03,
            9.607359798e-03,  1.084131464e-02,  1.214893498e-02,  1.353002390e-02,
            1.498437340e-02,  1.651176448e-02,  1.811196710e-02,  1.978474029e-02,
            2.152983213e-02,  2.334697982e-02,  2.523590970e-02,  2.719633731e-02,
            2.922796741e-02,  3.133049404e-02,  3.350360058e-02,  3.574695976e-02,
            3.806023374e-02,  4.044307415e-02,  4.289512215e-02,  4.541600845e-02,
            4.800535344e-02,  5.066276715e-02,  5.338784940e-02,  5.618018980e-02,
            5.903936783e-02,  6.196495290e-02,  6.495650445e-02,  6.801357194e-02,
            7.113569500e-02,  7.432240345e-02,  7.757321738e-02,  8.088764722e-02,
            8.426519385e-02,  8.770534861e-02,  9.120759342e-02,  9.477140087e-02,
            9.839623426e-02,  1.020815477e-01,  1.058267862e-01,  1.096313857e-01,
            1.134947733e-01,  1.174163672e-01,  1.213955767e-01,  1.254318027e-01,
            1.295244373e-01,  1.336728642e-01,  1.378764585e-01,  1.421345874e-01,
            1.464466094e-01,  1.508118753e-01,  1.552297276e-01,  1.596995011e-01,
            1.642205226e-01,  1.687921112e-01,  1.734135785e-01,  1.780842286e-01,
            1.828033579e-01,  1.875702559e-01,  1.923842047e-01,  1.972444793e-01,
            2.021503478e-01,  2.071010713e-01,  2.120959043e-01,  2.171340946e-01,
            2.222148835e-01,  2.273375058e-01,  2.325011901e-01,  2.377051587e-01,
            2.429486279e-01,  2.482308081e-01,  2.535509039e-01,  2.589081140e-01,
            2.643016316e-01,  2.697306445e-01,  2.751943352e-01,  2.806918807e-01,
            2.862224533e-01,  2.917852200e-01,  2.973793430e-01,  3.030039800e-01,
            3.086582838e-01,  3.143414030e-01,  3.200524817e-01,  3.257906599e-01,
            3.315550733e-01,  3.373448539e-01,  3.431591298e-01,  3.489970253e-01,
            3.548576614e-01,  3.607401553e-01,  3.666436213e-01,  3.725671702e-01,
            3.785099100e-01,  3.844709459e-01,  3.904493799e-01,  3.964443119e-01,
            4.024548390e-01,  4.084800560e-01,  4.145190556e-01,  4.205709283e-01,
            4.266347628e-01,  4.327096457e-01,  4.387946624e-01,  4.448888964e-01,
            4.509914298e-01,  4.571013438e-01,  4.632177182e-01,  4.693396318e-01,
            4.754661628e-01,  4.815963885e-01,  4.877293857e-01,  4.938642309e-01,
            5.000000000e-01,  5.061357691e-01,  5.122706143e-01,  5.184036115e-01,
            5.245338372e-01,  5.306603682e-01,  5.367822818e-01,  5.428986562e-01,
            5.490085702e-01,  5.551111036e-01,  5.612053376e-01,  5.672903543e-01,
            5.733652372e-01,  5.794290717e-01,  5.854809444e-01,  5.915199440e-01,
            5.975451610e-01,  6.035556881e-01,  6.095506201e-01,  6.155290541e-01,
            6.214900900e-01,  6.274328298e-01,  6.333563787e-01,  6.392598447e-01,
            6.451423386e-01,  6.510029747e-01,  6.568408702e-01,  6.626551461e-01,
            6.684449267e-01,  6.742093401e-01,  6.799475183e-01,  6.856585970e-01,
            6.913417162e-01,  6.969960200e-01,  7.026206570e-01,  7.082147800e-01,
            7.137775467e-01,  7.193081193e-01,  7.248056648e-01,  7.302693555e-01,
            7.356983684e-01,  7.410918860e-01,  7.464490961e-01,  7.517691919e-01,
            7.570513721e-01,  7.622948413e-01,  7.674988099e-01,  7.726624942e-01,
            7.777851165e-01,  7.828659054e-01,  7.879040957e-01,  7.928989287e-01,
            7.978496522e-01,  8.027555207e-01,  8.076157953e-01,  8.124297441e-01,
            8.171966421e-01,  8.219157714e-01,  8.265864215e-01,  8.312078888e-01,
            8.357794774e-01,  8.403004989e-01,  8.447702724e-01,  8.491881247e-01,
            8.535533906e-01,  8.578654126e-01,  8.621235415e-01,  8.663271358e-01,
            8.704755627e-01,  8.745681973e-01,  8.786044233e-01,  8.825836328e-01,
            8.865052267e-01,  8.903686143e-01,  8.941732138e-01,  8.979184523e-01,
            9.016037657e-01,  9.052285991e-01,  9.087924066e-01,  9.122946514e-01,
            9.157348062e-01,  9.191123528e-01,  9.224267826e-01,  9.256775966e-01,
            9.288643050e-01,  9.319864281e-01,  9.350434956e-01,  9.380350471e-01,
            9.409606322e-01,  9.438198102e-01,  9.466121506e-01,  9.493372328e-01,
            9.519946466e-01,  9.545839915e-01,  9.571048779e-01,  9.595569258e-01,
            9.619397663e-01,  9.642530402e-01,  9.664963994e-01,  9.686695060e-01,
            9.707720326e-01,  9.728036627e-01,  9.747640903e-01,  9.766530202e-01,
            9.784701679e-01,  9.802152597e-01,  9.818880329e-01,  9.834882355e-01,
            9.850156266e-01,  9.864699761e-01,  9.878510650e-01,  9.891586854e-01,
            9.903926402e-01,  9.915527437e-01,  9.926388212e-01,  9.936507091e-01,
            9.945882550e-01,  9.954513177e-01,  9.962397673e-01,  9.969534850e-01,
            9.975923633e-01,  9.981563061e-01,  9.986452283e-01,  9.990590565e-01,
            9.993977281e-01,  9.996611923e-01,  9.998494093e-01,  9.999623509e-01,
            1.000000000e+00,
    };

    const float kOscillationMinimumGap = 0.3f;

    class RandomOscillator
    {
    public:

        void Init() {
            value_ = 0.0f;
            next_value_ = Random::GetFloat() * 2.0f - 1.0f;
        }

        inline void set_slope(float slope) {
            phase_increment_ = 1.0f / fabs(next_value_ - value_) * slope;
            if (phase_increment_ > 1.0f)
                phase_increment_ = 1.0f;
        }

        float Next() {
            phase_ += phase_increment_;
            if (phase_ > 1.0f) {
                phase_--;
                value_ = next_value_;
                direction_ = !direction_;
                float rnd = (1.0f - kOscillationMinimumGap) * Random::GetFloat() + kOscillationMinimumGap;
                next_value_ = direction_ ?
                              value_ + (1.0f - value_) * rnd :
                              value_ - (1.0f + value_) * rnd;
            }

            float sin = Interpolate(lut_raised_cos, phase_, LUT_RAISED_COS_SIZE-1);
            return value_ + (next_value_ - value_) * sin;
        }

    private:
        float phase_ = 0.f;
        float phase_increment_ = 0.f;
        float value_ = 0.f;
        float next_value_;
        bool direction_;
    };
}
