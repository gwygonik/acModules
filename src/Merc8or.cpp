#include "acModules.hpp"
#include "Merc8orDisplay.cpp"

struct Merc8or : Module {
	enum ParamId {
		PARAM_HIGH_IN,
		PARAM_LOW_IN,
		PARAM_HIGH_OUT,
		PARAM_LOW_OUT,
		PARAMS_LEN
	};
	enum InputId {
		INPUT_CV,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUT_CV,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

    float inCVlow = 0.f;
    float inCVhigh = 0.f;
    float outCVlow = 0.f;
    float outCVhigh = 0.f;
    bool hasCVin = false;
	int numChannels;
    float cvIN[16] = {};
	float cvOUT[16] = {};

	Merc8or() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PARAM_HIGH_IN, -10.f, 10.f, 5.f, "High voltage");
		configParam(PARAM_LOW_IN, -10.f, 10.f, -5.f, "Low voltage");
		configParam(PARAM_HIGH_OUT, -10.f, 10.f, 5.f, "High voltage");
		configParam(PARAM_LOW_OUT, -10.f, 10.f, -5.f, "Low voltage");
		configInput(INPUT_CV, "CV");
		configOutput(OUTPUT_CV, "CV");
	}

	void process(const ProcessArgs& args) override {
        inCVlow = params[PARAM_LOW_IN].getValue();
        inCVhigh  = params[PARAM_HIGH_IN].getValue();
        outCVlow = params[PARAM_LOW_OUT].getValue();
        outCVhigh  = params[PARAM_HIGH_OUT].getValue();

        // UI blockers
        getParamQuantity(PARAM_HIGH_IN)->minValue = inCVlow+0.05f;
        getParamQuantity(PARAM_LOW_IN)->maxValue = inCVhigh-0.05f;

        if (inputs[INPUT_CV].isConnected()) {
			numChannels = std::max(1, inputs[INPUT_CV].getChannels());
			hasCVin = true;
			for (int i=0;i<numChannels;i++) {
				cvIN[i] = rack::math::clamp(inputs[INPUT_CV].getPolyVoltage(i),inCVlow,inCVhigh);
				cvOUT[i] = rack::math::rescale(cvIN[i], inCVlow, inCVhigh, outCVlow, outCVhigh);
		        outputs[OUTPUT_CV].setVoltage(cvOUT[i],i);
            }
			outputs[OUTPUT_CV].setChannels(numChannels);
		} else {
			numChannels = 0;
			hasCVin = false;
			outputs[OUTPUT_CV].setChannels(0);
        }


        
	}

	void setInput(int presetRange) {
		switch (presetRange) {
			case 0:
				params[PARAM_LOW_IN].setValue(0.f);
				params[PARAM_HIGH_IN].setValue(10.f);
				break;
			case 1:
				params[PARAM_LOW_IN].setValue(0.f);
				params[PARAM_HIGH_IN].setValue(5.f);
				break;
			case 2:
				params[PARAM_LOW_IN].setValue(-5.f);
				params[PARAM_HIGH_IN].setValue(5.f);
				break;
			case 3:
				params[PARAM_LOW_IN].setValue(-10.f);
				params[PARAM_HIGH_IN].setValue(10.f);
				break;
        }
    }
	void setOutput(int presetRange) {
		switch (presetRange) {
			case 0:
				params[PARAM_LOW_OUT].setValue(0.f);
				params[PARAM_HIGH_OUT].setValue(10.f);
				break;
			case 1:
				params[PARAM_LOW_OUT].setValue(0.f);
				params[PARAM_HIGH_OUT].setValue(5.f);
				break;
			case 2:
				params[PARAM_LOW_OUT].setValue(-5.f);
				params[PARAM_HIGH_OUT].setValue(5.f);
				break;
			case 3:
				params[PARAM_LOW_OUT].setValue(-10.f);
				params[PARAM_HIGH_OUT].setValue(10.f);
				break;
			case 4:
				params[PARAM_LOW_OUT].setValue(1.f);
				params[PARAM_HIGH_OUT].setValue(2.f);
				break;
			case 5:
				params[PARAM_LOW_OUT].setValue(2.f);
				params[PARAM_HIGH_OUT].setValue(3.f);
				break;
			case 6:
				params[PARAM_LOW_OUT].setValue(1.f);
				params[PARAM_HIGH_OUT].setValue(3.f);
				break;
			case 7:
				params[PARAM_LOW_OUT].setValue(2.f);
				params[PARAM_HIGH_OUT].setValue(5.f);
				break;
        }
    }
	void invertOutput() {
		float tmp = params[PARAM_LOW_OUT].getValue();
		params[PARAM_LOW_OUT].setValue(params[PARAM_HIGH_OUT].getValue());
		params[PARAM_HIGH_OUT].setValue(tmp);
    }

};


struct Merc8orWidget : ModuleWidget {
	Merc8orWidget(Merc8or* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Merc8or-White.svg"), asset::plugin(pluginInstance, "res/Merc8or-Dark.svg")));

		Merc8orDisplay<Merc8or>* display = createWidget<Merc8orDisplay<Merc8or>>(mm2px(Vec(1.5, 7.144)));
		display->box.size = mm2px(Vec(27.48,58.0));
		display->displaySize = Vec(27.48,58.0);
		display->module = module;
		addChild(display);

		addChild(createWidget<ThemedScrew>(Vec(2, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 16, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.439, 76.817)), module, Merc8or::PARAM_HIGH_IN));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.439, 92.163)), module, Merc8or::PARAM_LOW_IN));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.041, 76.817)), module, Merc8or::PARAM_HIGH_OUT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(22.041, 92.163)), module, Merc8or::PARAM_LOW_OUT));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.439, 109.370)), module, Merc8or::INPUT_CV));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(22.041, 109.370)), module, Merc8or::OUTPUT_CV));
	}

	void appendContextMenu(Menu* menu) override {
		Merc8or* module = getModule<Merc8or>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Merc8or Options"));
		menu->addChild(createSubmenuItem("Quick Set Input Range...", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("0V to 10V", "", [=]() {module->setInput(0);}));
				menu->addChild(createMenuItem("0V to 5V", "", [=]() {module->setInput(1);}));
				menu->addChild(createMenuItem("-5V to 5V", "", [=]() {module->setInput(2);}));
				menu->addChild(createMenuItem("-10V to 10V", "", [=]() {module->setInput(3);}));
			}
		));
		menu->addChild(createSubmenuItem("Quick Set Output Range...", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("0V to 10V", "", [=]() {module->setOutput(0);}));
				menu->addChild(createMenuItem("0V to 5V", "", [=]() {module->setOutput(1);}));
				menu->addChild(createMenuItem("-5V to 5V", "", [=]() {module->setOutput(2);}));
				menu->addChild(createMenuItem("-10V to 10V", "", [=]() {module->setOutput(3);}));
				menu->addChild(createMenuItem("1 Octave @ 1V", "", [=]() {module->setOutput(4);}));
				menu->addChild(createMenuItem("1 Octave @ 2V", "", [=]() {module->setOutput(5);}));
				menu->addChild(createMenuItem("2 Octaves @ 1V", "", [=]() {module->setOutput(6);}));
				menu->addChild(createMenuItem("2 Octaves @ 2V", "", [=]() {module->setOutput(7);}));
			}
		));
		menu->addChild(createMenuItem("Invert Output", "", [=]() {module->invertOutput();}));
	}
};


Model* modelMerc8or = createModel<Merc8or, Merc8orWidget>("Merc8or");
