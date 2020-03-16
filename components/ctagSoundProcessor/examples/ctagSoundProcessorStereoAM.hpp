#include "ctagSoundProcessor.hpp"

namespace CTAG{
    namespace SP{
        class ctagSoundProcessorStereoAM : public ctagSoundProcessor{
            public:
                virtual void Process(const ProcessData &);
                virtual ~ctagSoundProcessorStereoAM();
                ctagSoundProcessorStereoAM();
                virtual const char * GetCStrID() const;
            private:
                virtual void setParamValueInternal(const string id, const int val);
                const string id = "stereoam";
        };
    }
}