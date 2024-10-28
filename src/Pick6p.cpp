#include "acModules.hpp"

struct Pick6p : Module {

	struct pick6pMessage {
		float stepValues[16];
		int curCustomPattern;
	};

	enum ParamId {
		ENUMS(PARAM_STEP, 16),
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};

	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHT_PATTERN1,
		LIGHT_PATTERN2,
		LIGHT_EXTENDED,
		LIGHTS_LEN
    };

	bool isConnectedToP6 = false;
	float curP6StepValues[16];
	bool isOnCustomPattern = false;
	pick6pMessage leftMessages[2][1]; // messages from left module (controller module))


	Pick6p() {
		config(PARAMS_LEN, 0, 0, LIGHTS_LEN);

		for (int i = 0; i < 16; i++) {
			configParam(PARAM_STEP + i, 0.f, 6.f, 0.f, string::f("Pattern %d, Step %d", i < 8 ? 1 : 2 , (i + 1)%8));
			paramQuantities[PARAM_STEP+i]->snapEnabled = true;
			curP6StepValues[i] = 1.f;
		}

		// set the left expander message instances
		leftExpander.producerMessage = leftMessages[0];
		leftExpander.consumerMessage = leftMessages[1];

	}

	void process(const ProcessArgs& args) override {

		if (leftExpander.module) {
			if (leftExpander.module->model == modelPick6) {
				isConnectedToP6 = true;
				pick6pMessage *messageToModule = (pick6pMessage *)(leftExpander.producerMessage);
				for (int i = 0; i < 16; i++) {
					messageToModule->stepValues[i] = params[PARAM_STEP + i].getValue();
				}
				pick6pMessage *messageFromModule = (pick6pMessage *)(leftExpander.consumerMessage);
				if (messageFromModule->curCustomPattern > 0) {
					isOnCustomPattern = true;
					if (messageFromModule->curCustomPattern == 1) {
						lights[LIGHT_PATTERN1].setBrightness(0.95f);
						lights[LIGHT_PATTERN2].setBrightness(0.f);
                    } else {
						lights[LIGHT_PATTERN1].setBrightness(0.f);
						lights[LIGHT_PATTERN2].setBrightness(0.95f);
                    }
                } else {
					isOnCustomPattern = false;
					lights[LIGHT_PATTERN1].setBrightness(0.f);
					lights[LIGHT_PATTERN2].setBrightness(0.f);
					for (int i=0;i<16;i++) {
						curP6StepValues[i] = messageFromModule->stepValues[i];
                    }
                }
				leftExpander.messageFlipRequested = true;
				// turn on connected LED
				lights[LIGHT_EXTENDED].setBrightness(0.95f);
			} else {
				isOnCustomPattern = false;
				isConnectedToP6 = false;
				// turn off connected LED
				lights[LIGHT_EXTENDED].setBrightness(0.f);
				// and pattern LEDs
				lights[LIGHT_PATTERN1].setBrightness(0.f);
				lights[LIGHT_PATTERN2].setBrightness(0.f);
            }
		} else {
			isOnCustomPattern = false;
			isConnectedToP6 = false;
			// turn off connected LED
			lights[LIGHT_EXTENDED].setBrightness(0.f);
			// and pattern LEDs
			lights[LIGHT_PATTERN1].setBrightness(0.f);
			lights[LIGHT_PATTERN2].setBrightness(0.f);
        }
	}

	void copyPattern(int fromSide) {
		if (fromSide == 0) {
			for (int i=0;i<8;i++) {
				params[PARAM_STEP + (i+8)].setValue(params[PARAM_STEP + i].getValue());
			}
        } else {
			for (int i=0;i<8;i++) {
				params[PARAM_STEP + i].setValue(params[PARAM_STEP + (i+8)].getValue());
			}
        }
    }
	void copyPatternFromP6(int toSide) {
		if (isConnectedToP6 && !isOnCustomPattern) {
			for (int i=0;i<8;i++) {
				//DEBUG("%d, %f", i, curP6StepValues[i]);
				params[PARAM_STEP + (i + (toSide == 0 ? 0 : 8))].setValue(curP6StepValues[i]);
            }
        }
    }
	void clearCurrentPattern(int side) {
		for (int i=0;i<8;i++) {
			params[PARAM_STEP + (i+(side == 0 ? 0 : 8))].setValue(0.f);
		}
    }
	void clearBothPatterns() {
		for (int i=0;i<16;i++) {
			params[PARAM_STEP + i].setValue(0.f);
		}
    }
};


struct Pick6pWidget : ModuleWidget {
	Pick6pWidget(Pick6p* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Pick6p-White.svg"), asset::plugin(pluginInstance, "res/Pick6p-Dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(2, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 16, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 16, 0)));
		addChild(createWidget<ThemedScrew>(Vec(2, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// steps
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10.098, 20.200)), module, Pick6p::PARAM_STEP + 0));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10.098, 33.478)), module, Pick6p::PARAM_STEP + 1));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10.098, 46.707)), module, Pick6p::PARAM_STEP + 2));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10.098, 59.838)), module, Pick6p::PARAM_STEP + 3));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10.098, 73.069)), module, Pick6p::PARAM_STEP + 4));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10.098, 86.297)), module, Pick6p::PARAM_STEP + 5));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10.098, 99.526)), module, Pick6p::PARAM_STEP + 6));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(10.098, 112.755)), module, Pick6p::PARAM_STEP + 7));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.798, 20.200)), module, Pick6p::PARAM_STEP + 8));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.798, 33.478)), module, Pick6p::PARAM_STEP + 9));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.798, 46.707)), module, Pick6p::PARAM_STEP + 10));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.798, 59.838)), module, Pick6p::PARAM_STEP + 11));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.798, 73.069)), module, Pick6p::PARAM_STEP + 12));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.798, 86.297)), module, Pick6p::PARAM_STEP + 13));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.798, 99.526)), module, Pick6p::PARAM_STEP + 14));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(30.798, 112.755)), module, Pick6p::PARAM_STEP + 15));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(10.2, 5.999)), module, Pick6p::LIGHT_PATTERN1));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(30.798, 5.999)), module, Pick6p::LIGHT_PATTERN2));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(9.934, 121.322)), module, Pick6p::LIGHT_EXTENDED));
	}

	void appendContextMenu(Menu* menu) override {
		Pick6p* module = getModule<Pick6p>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Pick6p"));
		menu->addChild(createSubmenuItem("Copy...", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Left to Right", "", [=]() {module->copyPattern(0);}));
				menu->addChild(createMenuItem("Right to Left", "", [=]() {module->copyPattern(1);}));
				menu->addChild(createMenuItem("From Pick6 to Left", "", [=]() {module->copyPatternFromP6(0);}));
				menu->addChild(createMenuItem("From Pick6 to Right", "", [=]() {module->copyPatternFromP6(1);}));
			}
		));
		menu->addChild(createSubmenuItem("Clear...", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Left Pattern", "", [=]() {module->clearCurrentPattern(0);}));
				menu->addChild(createMenuItem("Right Pattern", "", [=]() {module->clearCurrentPattern(1);}));
				menu->addChild(createMenuItem("Both Pattern", "", [=]() {module->clearBothPatterns();}));
			}
		));
	}


};


Model* modelPick6p = createModel<Pick6p, Pick6pWidget>("Pick6p");
