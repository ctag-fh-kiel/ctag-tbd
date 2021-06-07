/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020,2021 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/
/*
Phaser effect, based on the soundpipe port by Paul Bachelor generated from Faust code taken from the Guitarix project, as to be found here:
https://github.com/PaulBatchelor/Soundpipe
https://paulbatchelor.github.io/res/soundpipe/docs/phaser.html
Adapted by M. Brüssel from the Soundpipe version.
*/

class ctagSPphaser
{
  public:
    ctagSPphaser() {Init();}
    void Init();
    float Process(float in);  // Mono
    void Process(float* in_l, float* in_r ,float* out_l, float* out_r); // Stereo

    inline void SetSampleRate(float sr) { fSamplingFreq_ = sr;  }
    inline void SetMaxNotch1Freq(float maxNotch1Freq) {  maxNotch1Freq_ = maxNotch1Freq; }
    inline void SetMinNotch1Freq(float minNotch1Freq) {  minNotch1Freq_ = minNotch1Freq; }
    inline void SetNotchWidth(float notchWidth) {  notchWidth_ = notchWidth; }
    inline void SetNotchFreq(float notchFreq) {  notchFreq_ = notchFreq; }
    inline void SetVibratoMode(bool activate) {  vibratoMode_ = (float)activate; }
    inline void SetDepth(float depth) { depth_ = depth; }
    inline void SetFeedbackGain(float feedbackGain) { feedbackGain_ = feedbackGain; }
    inline void SetInvert(bool activate) { invert_ = (float)activate; }
    inline void SetLevel(float level) { level_ = level; }
    inline void SetLfoBpm(float lfobpm) { lfobpm_ = lfobpm; }

  private:
    float fRec4[3];
    float fRec3[3];
    float fRec2[3];
    float fRec1[3];
    float fRec11[3];
    float fRec10[3];
    float fRec9[3];
    float fRec8[3];
    int iVec0[2];
    float fRec5[2];
    float fRec6[2];
    float fRec0[2];
    float fRec7[2];

    float level_;
    float vibratoMode_;
    float depth_;
    int fSamplingFreq_;
    int iConst0;
    float fConst1;
    float notchWidth_;
    float notchFreq_;
    float minNotch1Freq_;
    float maxNotch1Freq_;
    float fConst2;
    float lfobpm_;
    float feedbackGain_;
    float invert_;
};
