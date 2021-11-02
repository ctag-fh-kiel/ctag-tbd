#include "plugin.hpp"
#include "WebServer.hpp"
#include <iostream>


struct tbd4vcv : Module {
	enum ParamIds {
		BTN_TRIG_0_PARAM,
		BTN_TRIG_1_PARAM,
		POT0_PARAM,
		POT1_PARAM,
		GAIN0_PARAM,
		GAIN1_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		IN0_INPUT,
		IN1_INPUT,
		TRIG0_INPUT,
		TRIG1_INPUT,
		CV0_INPUT,
		CV1_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		OUT0_OUTPUT,
		OUT1_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BTN_TRIG_0_LIGHT,
		NUM_LIGHTS
	};

	tbd4vcv() {
        instanceCount++;
        std::cerr << "Instance number " << instanceCount << std::endl;
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(BTN_TRIG_0_PARAM, 0.f, 1.f, 0.f, "");
		configParam(BTN_TRIG_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(POT0_PARAM, 0.f, 1.f, 0.f, "");
		configParam(POT1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN0_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN1_PARAM, 0.f, 1.f, 0.f, "");
	}
    ~tbd4vcv(){
        server.Stop();
        rack::logger::log(Level::DEBUG_LEVEL, "tbd4vcv.cpp", 48, "Destructor called");
        instanceCount--;
        std::cerr << "module destructor called" << std::endl;
    }

	void process(const ProcessArgs& args) override {

	}

    static tbd4vcv* activeServerInstance;
    WebServer server;
    static int instanceCount;

private:

};

int tbd4vcv::instanceCount {0};
tbd4vcv* tbd4vcv::activeServerInstance {nullptr};

struct tbd4vcvWidget : ModuleWidget {
	tbd4vcvWidget(tbd4vcv* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/tbd4vcv.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<BefacoPush>(mm2px(Vec(7.406, 49.539)), module, tbd4vcv::BTN_TRIG_0_PARAM));
		addParam(createParamCentered<BefacoPush>(mm2px(Vec(32.806, 49.539)), module, tbd4vcv::BTN_TRIG_1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(7.406, 62.333)), module, tbd4vcv::POT0_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(32.806, 62.333)), module, tbd4vcv::POT1_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(7.406, 78.933)), module, tbd4vcv::GAIN0_PARAM));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(32.806, 78.933)), module, tbd4vcv::GAIN1_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(4.897, 96.524)), module, tbd4vcv::IN0_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.057, 96.524)), module, tbd4vcv::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.217, 96.524)), module, tbd4vcv::TRIG0_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.377, 96.524)), module, tbd4vcv::TRIG1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.217, 109.478)), module, tbd4vcv::CV0_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35.377, 109.478)), module, tbd4vcv::CV1_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(4.897, 109.478)), module, tbd4vcv::OUT0_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.057, 109.478)), module, tbd4vcv::OUT1_OUTPUT));

		addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(20.123, 57.802)), module, tbd4vcv::BTN_TRIG_0_LIGHT));
	}

    void appendContextMenu(Menu* menu) override {
        rack::logger::log(Level::DEBUG_LEVEL, "tbd4vcv.cpp", 98, "appendContextMenu called");
        tbd4vcv* module = dynamic_cast<tbd4vcv*>(this->module);

        menu->addChild(new MenuEntry);
        menu->addChild(createMenuLabel("Enable Web Server"));

        struct ModeItem : MenuItem {
            tbd4vcv* module;
            void onAction(const event::Action& e) override {
                if(module->activeServerInstance == module){
                    module->activeServerInstance = nullptr;
                    module->server.Stop();
                }else{
                    module->activeServerInstance = module;
                }
            }
        };

        std::string modeName = {"Active"};
        ModeItem* modeItem = createMenuItem<ModeItem>(modeName);
        modeItem->rightText = CHECKMARK(module == module->activeServerInstance);
        modeItem->module = module;
        menu->addChild(modeItem);

        if(module == module->activeServerInstance){
            module->server.Stop();
            module->server.Start(3000);
        }
    }
};


Model* modeltbd4vcv = createModel<tbd4vcv, tbd4vcvWidget>("tbd4vcv");