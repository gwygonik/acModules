#include "acModules.hpp"

struct Pul5es : Module {
	enum ParamId {
		OUTON_PARAM,
		LOOP_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RESET_INPUT,
		PULSE_INPUT,
		OUTON_INPUT,
		LOOP_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUT_PULSE,
		OUTPUTS_LEN
	};
	enum LightId {
		LOOP_LIGHT,
		LIGHTS_LEN
	};

	int inPulseCount = 0;
	int pulseOnCount = 0;
	bool hasPulsed = false;
	bool hasPulsedTrig = false;
	bool invertPulseLogic = false;
	bool loopAround = false;
	bool triggered = false;
	int delayCounter = 0;
	bool delayBeforePlay = true;
	bool useGateForLoop = false;

	dsp::SchmittTrigger inReset;
	dsp::SchmittTrigger inPulse;
	dsp::SchmittTrigger inLoop;
	dsp::PulseGenerator pulseOutput;

	Pul5es() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OUTON_PARAM, 2.f, 256.f, 16.f, "Out on...");
		configSwitch(LOOP_PARAM, 0.f,1.f,0.f, "Loop");
		configInput(RESET_INPUT, "Reset Trigger");
		configInput(PULSE_INPUT, "Pulse");
		configInput(OUTON_INPUT, "Out On... (0-10V)");
		configInput(LOOP_INPUT, "Toggle Loop (Trigger)");
		configOutput(OUTPUT_PULSE, "Pulse");
		paramQuantities[OUTON_PARAM]->snapEnabled = true;
	}

	void process(const ProcessArgs& args) override {
        pulseOnCount = params[OUTON_PARAM].getValue();
		if (inputs[OUTON_INPUT].isConnected()) {
			pulseOnCount = clamp(rescale(inputs[OUTON_INPUT].getVoltage(),0.f,10.f,2.f,256.f),2.f,256.f);
			params[OUTON_PARAM].setValue(pulseOnCount);
        }

		if (inputs[LOOP_INPUT].isConnected()) {
			if (useGateForLoop) {
				// gate is loop on/off
				if (inputs[LOOP_INPUT].getVoltage() > 0.2f) {
					loopAround = true;
                } else {
					loopAround = false;
                }
				params[LOOP_PARAM].setValue(loopAround ? 1.f : 0.f);
            } else {
				// trigger toggles
				if(inLoop.process(inputs[LOOP_INPUT].getVoltage(), 0.01f, 2.f)) {
					loopAround ^= true;
					params[LOOP_PARAM].setValue(loopAround ? 1.f : 0.f);
                }

            }
		} else {
			loopAround = params[LOOP_PARAM].getValue() > 0.f;
        }

		if (inReset.process(inputs[RESET_INPUT].getVoltage(), 0.01f, 2.f)) {
			inPulseCount = 0;
			hasPulsed = false;
			delayCounter = 0;
			delayBeforePlay = true;
		}

		if (inPulse.process(inputs[PULSE_INPUT].getVoltage(), 0.01f, 2.f)) {
			if (!hasPulsed || loopAround) {
				triggered = true;
            } else {
				triggered = false;
            }
			delayCounter = 0;
			delayBeforePlay = true;
		}

		if (triggered) {
			// this delay accounts for changes in incoming CV but doesn't add noticeable latency
			if (delayBeforePlay) {
				delayCounter++;
				if (delayCounter > 5) {
					delayBeforePlay = false;
                }
			} else {
				inPulseCount++;
				if (invertPulseLogic) {
					if (inPulseCount >= pulseOnCount) {
						inPulseCount = 0;
						hasPulsed = true;
					} else {
						pulseOutput.trigger(1e-3f);
                    }
                } else {
					if (inPulseCount >= pulseOnCount) {
						inPulseCount = 0;
						hasPulsed = true;
						pulseOutput.trigger(1e-3f);
					}
                }
				triggered = false;
			}
		}

		hasPulsedTrig = pulseOutput.process(args.sampleTime);
		outputs[OUTPUT_PULSE].setVoltage(hasPulsedTrig ? 10.f : 0.f);

		lights[LOOP_LIGHT].setBrightness(loopAround);
        
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_t* val = json_boolean(loopAround);
		json_object_set_new(rootJ, "loopAround", val);
		val = json_boolean(useGateForLoop);
		json_object_set_new(rootJ, "useGateForLoop", val);
		val = json_boolean(invertPulseLogic);
		json_object_set_new(rootJ, "invertPulseLogic", val);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		// looping?
		json_t* val = json_object_get(rootJ, "loopAround");
		if (val) {
			loopAround = json_boolean_value(val);
		}
		// inverse pulse logic?
		val = json_object_get(rootJ, "invertPulseLogic");
		if (val) {
			invertPulseLogic = json_boolean_value(val);
		}
		// gate for loop?
		val = json_object_get(rootJ, "useGateForLoop");
		if (val) {
			useGateForLoop = json_boolean_value(val);
		}
	}
};


struct Pul5esWidget : ModuleWidget {
	Pul5esWidget(Pul5es* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Pul5es-White.svg"), asset::plugin(pluginInstance, "res/Pul5es-Dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(2, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x / 2 - 8, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(10.16, 15.611)), module, Pul5es::RESET_INPUT));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(10.16, 33.901)), module, Pul5es::PULSE_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(5.546, 51.454)), module, Pul5es::OUTON_PARAM));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(14.163, 51.454)), module, Pul5es::OUTON_INPUT));

		// loop
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(5.546, 69.743)), module, Pul5es::LOOP_PARAM, Pul5es::LOOP_LIGHT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(14.163, 69.743)), module, Pul5es::LOOP_INPUT));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(10.16, 89.153)), module, Pul5es::OUTPUT_PULSE));

	}

	void appendContextMenu(Menu* menu) override {
		Pul5es* module = getModule<Pul5es>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Pul5es Options"));
		menu->addChild(createBoolPtrMenuItem("Invert Pulse Logic", "", &module->invertPulseLogic));
		menu->addChild(createBoolPtrMenuItem("Use Gate For Loop On/Off", "", &module->useGateForLoop));
	}
};


Model* modelPul5es = createModel<Pul5es, Pul5esWidget>("Pul5es");
