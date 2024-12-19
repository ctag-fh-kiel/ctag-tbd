#include <tbd/drivers/indicator.hpp>

#include <tbd/logging.hpp>

namespace {
    constexpr char tag[] = "sim indicator";
    int _r = 0;
    int _g = 0;
    int _b = 0;
}


namespace tbd::drivers {

void Indicator::init() {
    TBD_LOGI(tag, "initializing indicator");
}

void Indicator::GetLedRGB(int &r, int &g, int &b) {
    r = _r;
    g = _g;
    b = _b;
}

void Indicator::SetLedRGB(int r, int g, int b) {
    SetLedR(r);
    SetLedG(g);
    SetLedB(b);
}

void Indicator::SetLedR(int r) {
    TBD_LOGD(tag, "set r %i", r);
    _r = r;
}

void Indicator::SetLedG(int g) {
    TBD_LOGD(tag, "set r %i", g);
    _g = g;
}

void Indicator::SetLedB(int b) {
    TBD_LOGD(tag, "set r %i", b);
    _b = b;
}

}