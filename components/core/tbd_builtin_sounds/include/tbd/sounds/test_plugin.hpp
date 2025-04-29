#include <atomic>
#include <tbd/sound_processor.hpp>

namespace tbd::sounds {

struct ReusableGroup {
    [[tbd(name="Some Group Param2", description="This trigger has no effect")]]
    uint_par some_group2_param1;
    trigger_par     some_group2_param2;

    [[tbd(name="Another Group Param2", description="This trigger has no effect")]]
    float_par some_group2_param3;
};


[[tbd(name="TestPlugin", description="Output input signal unaltered")]]
struct TestPlugin : audio::SoundProcessor {

    virtual void Process(const audio::ProcessData&) override;
    virtual void Init(std::size_t blockSize, void *blockPtr) override;
    virtual ~TestPlugin();

protected:
    [[tbd(name="Some Param", min=12, description="This input has no effect")]]
    uint_par some_param;

    [[tbd(name="Foo thing")]]
    int_par foo; 
    
    [[tbd(name="Bar thing")]] 
    float_par bar; 

    [[tbd(name="Bla thing")]] ufloat_par bla;

    [[tbd(name="Some Trigger", description="This trigger has no effect")]]
    bool some_trigger;
        // sectionHpp

    [[tbd(name="Some Group")]]
    struct {
        [[tbd(name="Some Group Param", description="This trigger has no effect")]]
        int_par some_group1_param1;

        [[tbd(name="Another Group Param", description="This trigger has no effect")]]
        ufloat_par some_group1_param2;
    } group1;

    [[tbd(name="Another Group")]]
    ReusableGroup group2;

};  

}
