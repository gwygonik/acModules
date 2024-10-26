#include "acModules.hpp"
#include "Ov3rCrossDisplay.cpp"

struct Ov3rCross : Module {
	enum ParamId {
		PARAM_CUTOFF_HIGH,
		PARAM_CUTOFF_LOW,
		PARAMS_LEN
	};
	enum InputId {
		INPUT_CV,
		INPUT_TRIGGER,
        INPUT_CVTHRU,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUT_CV_HIGH,
		OUTPUT_TRIG_HIGH,
		OUTPUT_CV_MID,
		OUTPUT_TRIG_MID,
		OUTPUT_CV_LOW,
		OUTPUT_TRIG_LOW,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHT_HIGH,
		LIGHT_MID,
		LIGHT_LOW,
		LIGHTS_LEN
	};

    float lowCut = 0.f;
    float highCut = 0.f;
    float cvVal = 0.f;
    float cvThru = 0.f;
    bool sendCVThru = false;
    bool triggered = false;
    float targetOut1, targetOut2, targetOut3;
    float currentOut1, currentOut2, currentOut3;
    float outVoltage1, outVoltage2, outVoltage3;
    bool hasCVin = false;
    bool hasPickedRandomCV = false;
    short curOut = 0;
    short lastOut = 0;
    float cvIN;
    float rtCVIN;
    dsp::SchmittTrigger inTrigger;
    dsp::PulseGenerator pulseOutputs[3];
    bool useSampleAndHold = false;
    bool muteToZero = false;
    int randomCVRangeMode = 0;
    bool trigOnZoneChange = false;

	Ov3rCross() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PARAM_CUTOFF_HIGH, -5.f, 10.f, 0.f, "High cutoff voltage");
		configParam(PARAM_CUTOFF_LOW, -5.f, 10.f, 0.f, "Low cutoff voltage");
		configInput(INPUT_CV, "CV");
		configInput(INPUT_TRIGGER, "Trigger");
		configInput(INPUT_CVTHRU, "CV Output Signal (Optional)");
		configOutput(OUTPUT_CV_HIGH, "CV High");
		configOutput(OUTPUT_TRIG_HIGH, "Trigger High");
		configOutput(OUTPUT_CV_MID, "CV Middle");
		configOutput(OUTPUT_TRIG_MID, "Trigger Middle");
		configOutput(OUTPUT_CV_LOW, "CV Low");
		configOutput(OUTPUT_TRIG_LOW, "Trigger Low");

        targetOut1 = targetOut2 = targetOut3 = currentOut1 = currentOut2 = currentOut3 = 0.f;
	}

	void process(const ProcessArgs& args) override {
        lowCut = params[PARAM_CUTOFF_LOW].getValue();
        highCut  = params[PARAM_CUTOFF_HIGH].getValue();
        if (inputs[INPUT_CV].isConnected()) {
            cvIN = rack::math::clamp(inputs[INPUT_CV].getVoltage(),-5.f,10.f);
            hasCVin = true;
        } else {
            hasCVin = false;
        }
        curOut = 0;

        // UI blockers
        getParamQuantity(PARAM_CUTOFF_HIGH)->minValue = lowCut+0.05f;
        getParamQuantity(PARAM_CUTOFF_LOW)->maxValue = highCut-0.05f;

        if (hasCVin && !useSampleAndHold) {
            // update here for visual updates while not running
            setState();
        }

        if (inputs[INPUT_CVTHRU].isConnected()) {
            sendCVThru = true;
            cvThru = inputs[INPUT_CVTHRU].getVoltage();
            setState();
        } else {
            sendCVThru = false;
        }

        triggered = inTrigger.process(rack::math::rescale(inputs[INPUT_TRIGGER].getVoltage(), 0.1f, 2.f, 0.f, 1.f));


        // normal trigger mode
        if (triggered) {
            if (!hasCVin) {
                cvIN = rack::math::clamp((rand()) / ((RAND_MAX/15.f)) - 5.f, -5.f, 10.f); // -5 to 10
                if (randomCVRangeMode == 1) {
                    cvIN = rack::math::rescale(cvIN, -5.f, 10.f, -5.f, 5.f); // -5 to 5
                } else if (randomCVRangeMode == 2) {
                    cvIN = rack::math::rescale(cvIN, -5.f, 10.f, 0.f, 10.f); // 0 to 10
                } else if (randomCVRangeMode == 3) {
                    cvIN = rack::math::rescale(cvIN, -5.f, 10.f, 0.f, 5.f); // 0 to 5
                }
                setState();
            } else {
                if (useSampleAndHold) {
                    setState();
                }
            }
            pulseOutputs[0].reset();
            pulseOutputs[1].reset();
            pulseOutputs[2].reset();
            pulseOutputs[curOut-1].trigger(1e-3f);
        }

        if (!useSampleAndHold && hasCVin && !triggered && trigOnZoneChange) {
            // zone has changed - output a trigger on new zone
            // NOTE: if we don't gate by all the if options, we get into a weird frozen state under certain conditions
            if (curOut != lastOut) {
                pulseOutputs[curOut-1].reset();
                pulseOutputs[curOut-1].trigger(1e-3f);
                lastOut = curOut;
            }
        }
        
        // for display - use whatever the current set CV is (input or random)
        rtCVIN = cvIN;

        if (muteToZero) {
            currentOut3 += (targetOut3 - currentOut3) / 5.0f;
            currentOut2 += (targetOut2 - currentOut2) / 5.0f;
            currentOut1 += (targetOut1 - currentOut1) / 5.0f;
            if (abs(currentOut3 - targetOut3) < 0.05f) currentOut3 = targetOut3;
            if (abs(currentOut2 - targetOut2) < 0.05f) currentOut2 = targetOut2;
            if (abs(currentOut1 - targetOut1) < 0.05f) currentOut1 = targetOut1;
        }

        outputs[OUTPUT_CV_HIGH].setVoltage(outVoltage1 * (muteToZero ? currentOut1 : 1.f));
        outputs[OUTPUT_CV_MID].setVoltage(outVoltage2 * (muteToZero ? currentOut2 : 1.f));
        outputs[OUTPUT_CV_LOW].setVoltage(outVoltage3 * (muteToZero ? currentOut3 : 1.f));
        outputs[OUTPUT_TRIG_HIGH].setVoltage(pulseOutputs[0].process(args.sampleTime) ? 10.f : 0.f);
        outputs[OUTPUT_TRIG_MID].setVoltage(pulseOutputs[1].process(args.sampleTime) ? 10.f : 0.f);
        outputs[OUTPUT_TRIG_LOW].setVoltage(pulseOutputs[2].process(args.sampleTime) ? 10.f : 0.f);
        
	}

    void setState() {
        if (cvIN <= lowCut) {
            outVoltage3 = sendCVThru ? cvThru : cvIN;
            lights[LIGHT_LOW].setBrightness(1.f);
            lights[LIGHT_MID].setBrightness(0.f);
            lights[LIGHT_HIGH].setBrightness(0.f);
            targetOut1 = 0.0f;
            targetOut2 = 0.0f;
            targetOut3 = 1.0f;
            curOut = 3;
        } else {
            if (cvIN > lowCut && cvIN < highCut) {
                outVoltage2 = sendCVThru ? cvThru : cvIN;
                lights[LIGHT_LOW].setBrightness(0.f);
                lights[LIGHT_MID].setBrightness(1.f);
                lights[LIGHT_HIGH].setBrightness(0.f);
                targetOut1 = 0.0f;
                targetOut2 = 1.0f;
                targetOut3 = 0.0f;
                curOut = 2;
            } else {
                if (cvIN >= highCut) {
                    outVoltage1 = sendCVThru ? cvThru : cvIN;
                    lights[LIGHT_LOW].setBrightness(0.f);
                    lights[LIGHT_MID].setBrightness(0.f);
                    lights[LIGHT_HIGH].setBrightness(1.f);
                    targetOut1 = 1.0f;
                    targetOut2 = 0.0f;
                    targetOut3 = 0.0f;
                    curOut = 1;
                }
            }
        }
        cvVal = cvIN;
    }

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* val = json_boolean(useSampleAndHold);
		json_object_set_new(rootJ, "useSampleAndHold", val);
		val = json_boolean(muteToZero);
		json_object_set_new(rootJ, "muteToZero", val);
		val = json_boolean(trigOnZoneChange);
		json_object_set_new(rootJ, "trigOnZoneChange", val);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		// sample and hold
		json_t* val = json_object_get(rootJ, "useSampleAndHold");
		if (val) {
			useSampleAndHold = json_boolean_value(val);
		}
        // mute to zero
		val = json_object_get(rootJ, "muteToZero");
		if (val) {
			muteToZero = json_boolean_value(val);
		}
        // output trigger on zone change
		val = json_object_get(rootJ, "trigOnZoneChange");
		if (val) {
			trigOnZoneChange = json_boolean_value(val);
		}
	}

};


struct Ov3rCrossWidget : ModuleWidget {
	Ov3rCrossWidget(Ov3rCross* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Ov3rCross-White.svg"), asset::plugin(pluginInstance, "res/Ov3rCross.svg")));

		Ov3rCrossDisplay<Ov3rCross>* display = createWidget<Ov3rCrossDisplay<Ov3rCross>>(mm2px(Vec(15.4, 9.546)));
		display->box.size = mm2px(Vec(18.0,104.0));
		display->displaySize = Vec(18.0,104.0);
		display->module = module;
		addChild(display);

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.343, 91.467)), module, Ov3rCross::PARAM_CUTOFF_HIGH));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(8.343, 102.373)), module, Ov3rCross::PARAM_CUTOFF_LOW));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.343, 23.726)), module, Ov3rCross::INPUT_CV));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.343, 42.247)), module, Ov3rCross::INPUT_TRIGGER));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.343, 61.447)), module, Ov3rCross::INPUT_CVTHRU));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(42.379, 24.774)), module, Ov3rCross::OUTPUT_CV_HIGH));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(42.379, 38.532)), module, Ov3rCross::OUTPUT_TRIG_HIGH));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(42.379, 58.906)), module, Ov3rCross::OUTPUT_CV_MID));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(42.379, 72.664)), module, Ov3rCross::OUTPUT_TRIG_MID));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(42.379, 93.038)), module, Ov3rCross::OUTPUT_CV_LOW));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(42.379, 106.796)), module, Ov3rCross::OUTPUT_TRIG_LOW));

		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(46.246, 12.547)), module, Ov3rCross::LIGHT_HIGH));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(46.246, 46.679)), module, Ov3rCross::LIGHT_MID));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(46.246, 80.811)), module, Ov3rCross::LIGHT_LOW));
	}

	void appendContextMenu(Menu* menu) override {
		Ov3rCross* module = getModule<Ov3rCross>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Ov3rCross Preferences"));
		menu->addChild(createBoolPtrMenuItem("Sample and Hold Control CV", "", &module->useSampleAndHold));
		menu->addChild(createBoolPtrMenuItem("Mute Non-Active Outputs To 0V", "", &module->muteToZero));
		menu->addChild(createIndexPtrSubmenuItem("Random CV Range", {"-5V to 10V", "-5V to 5V", "0V to 10V", "0V to 5V"}, &module->randomCVRangeMode));
		menu->addChild(createBoolPtrMenuItem("Output Trigger on CV Zone Change", "", &module->trigOnZoneChange));
	}

};


Model* modelOv3rCross = createModel<Ov3rCross, Ov3rCrossWidget>("Ov3rCross");
