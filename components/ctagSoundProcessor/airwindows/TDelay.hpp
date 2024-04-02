
namespace airwindows{
    class TDelay {
    public:
        void Process(float *buf, int sz, int ch);
        void SetWet(const float&v);
        void SetDry(const float&v);
        void SetLNFT(const float&v);
        void SetDepth(const float&v);
        void SetDelay(const float&v);
        void SetFeedback(const float&v);
        void SetBypass(const bool v);
        void SetBlockMem(void *blockMemPtr);
        TDelay();
        ~TDelay();
    private:
        int *p;
        float *d;
        int gcount;
        int delay;
        int maxdelay;
        int chase;
        bool isBypass;

        const int length = 44100*2; // max dly length

        float fpNShape;
        //default stuff

        float A;
        float B;
        float C;
        float D;
        float E;
        float F;
    };
}

