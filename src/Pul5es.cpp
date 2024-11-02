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

	dsp::SchmittTrigger inReset;
	dsp::SchmittTrigger inPulse;
	dsp::PulseGenerator pulseOutput;
	dsp::BooleanTrigger loopTrigger;

	Pul5es() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OUTON_PARAM, 2.f, 256.f, 16.f, "Out on...");
		configParam(LOOP_PARAM, 0,1,1, "Loop");
		configInput(RESET_INPUT, "Reset Trigger");
		configInput(PULSE_INPUT, "Pulse");
		configOutput(OUTPUT_PULSE, "Pulse");
		paramQuantities[OUTON_PARAM]->snapEnabled = true;
	}

	void process(const ProcessArgs& args) override {
        pulseOnCount = params[OUTON_PARAM].getValue();

		if (loopTrigger.process(params[LOOP_PARAM].getValue() > 0.f)) {
			loopAround ^= true;
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

		lights[LOOP_LIGHT].setBrightness(loopAround ? 0.95f : 0.f);
        
	}

};


struct Pul5esWidget : ModuleWidget {
	Pul5esWidget(Pul5es* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Pul5es-White.svg"), asset::plugin(pluginInstance, "res/Pul5es-Dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(2, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 16, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(5.08, 15.611)), module, Pul5es::RESET_INPUT));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(5.08, 32.204)), module, Pul5es::PULSE_INPUT));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(5.08, 52.242)), module, Pul5es::OUTON_PARAM));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(5.08, 67.81)), module, Pul5es::OUTPUT_PULSE));

		// loop
		addParam(createParamCentered<LEDButton>(mm2px(Vec(5.08, 84.99)), module, Pul5es::LOOP_PARAM));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(5.08, 84.99)), module, Pul5es::LOOP_LIGHT));

	}

	void appendContextMenu(Menu* menu) override {
		Pul5es* module = getModule<Pul5es>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Pul5es Options"));
		menu->addChild(createBoolPtrMenuItem("Invert Pulse Logic", "", &module->invertPulseLogic));
	}
};


Model* modelPul5es = createModel<Pul5es, Pul5esWidget>("Pul5es");
