#ifndef CTAG_TBD_ES8311_HPP
#define CTAG_TBD_ES8311_HPP

#include <stdint.h>
#include "es8311.h"

class es8311 final{
    es8311_handle_t es_handle = nullptr;
public:
    es8311();
    ~es8311();
    void init();
    bool identify();
    bool setOutputVolume(uint8_t lvol, uint8_t rvol);
};


#endif //CTAG_TBD_ES8311_HPP
