#include "acModules.hpp"
#include "Rhythm1101Display.cpp"

struct Rhythm1101 : Module {

	enum ParamId {
		ENUMS(PARAM_STEP, 16),
		PRESET_PARAM,
		STEPS_PARAM,
		MUTATE_PARAM,
		MUTATETRACK_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIGGER_INPUT,
		RESET_INPUT,
		PRESET_INPUT,
		INPUTS_LEN
	};

	enum OutputId {
		ENUMS(MONOTRIG_OUTPUT, 4),
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(LIGHT_STEP, 16),
		MUTATE_LIGHT,
		ENUMS(LIGHT_MUTATETRACKS, 4),
		LIGHTS_LEN
    };

	int curStep, steps, lastSteps, filter_steps, seed;
	int curPreset, basePreset, oldPreset;
	static constexpr float numPresets = 16.f;
	int presets[(int)numPresets][16] = {
		{ 9, 0, 4, 0, 3, 0, 4, 0, 1, 0, 4, 0, 3, 0, 4, 0 },
		{ 9, 0, 4, 0, 3, 0, 4, 0, 1, 0, 4, 0, 3, 0, 5, 2 },
		{ 5, 1, 4, 0, 6, 0, 4, 1, 4, 0, 5, 1, 6, 0, 4, 0 },
		{ 5, 4, 5, 4, 6, 4, 5,10, 4, 6, 5, 6, 6, 9, 4, 6 },
		{ 5, 0, 5, 0, 6, 0, 4, 2, 4, 2, 5, 1, 6, 0, 4, 2 },
		{ 4, 2, 5, 1, 6, 0, 4, 2, 4, 2, 9, 0, 4, 0, 6, 0 },
		{ 5, 4, 4, 4,14, 4, 4, 5, 4, 5, 5, 4,14, 4, 5, 4 },
		{13, 0, 4,12, 4, 0,12, 1, 5, 0, 7, 6, 5, 0, 5, 4 },
		{13, 0,12, 4,14, 0,13,12, 4, 8, 5,12,14, 5,12, 4 },
		{ 5,12, 4,12, 7,12, 4, 4,13,12, 4,12, 7,12, 4, 4 },
		{13, 5, 4,13, 6, 4,12, 5, 4, 4,13,12,14, 4, 4, 4 },
		{ 5, 0, 4, 8, 6, 2, 4, 0, 4, 0, 5, 8, 6, 1, 4, 0 },
		{ 1, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 8, 0 },
		{ 5, 0, 8, 4, 0, 0, 5, 0, 2, 4, 0, 0, 4, 0, 4, 4 },
		{ 1, 0, 4, 0, 2, 0, 5, 1, 0, 0, 5, 0, 2, 4,12, 4 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	};
	int factoryPresets[(int)numPresets][16] = {
		{ 9, 0, 4, 0, 3, 0, 4, 0, 1, 0, 4, 0, 3, 0, 4, 0 },
		{ 9, 0, 4, 0, 3, 0, 4, 0, 1, 0, 4, 0, 3, 0, 5, 2 },
		{ 5, 1, 4, 0, 6, 0, 4, 1, 4, 0, 5, 1, 6, 0, 4, 0 },
		{ 5, 4, 5, 4, 6, 4, 5,10, 4, 6, 5, 6, 6, 9, 4, 6 },
		{ 5, 0, 5, 0, 6, 0, 4, 2, 4, 2, 5, 1, 6, 0, 4, 2 },
		{ 4, 2, 5, 1, 6, 0, 4, 2, 4, 2, 9, 0, 4, 0, 6, 0 },
		{ 5, 4, 4, 4,14, 4, 4, 5, 4, 5, 5, 4,14, 4, 5, 4 },
		{13, 0, 4,12, 4, 0,12, 1, 5, 0, 7, 6, 5, 0, 5, 4 },
		{13, 0,12, 4,14, 0,13,12, 4, 8, 5,12,14, 5,12, 4 },
		{ 5,12, 4,12, 7,12, 4, 4,13,12, 4,12, 7,12, 4, 4 },
		{13, 5, 4,13, 6, 4,12, 5, 4, 4,13,12,14, 4, 4, 4 },
		{ 5, 0, 4, 8, 6, 2, 4, 0, 4, 0, 5, 8, 6, 1, 4, 0 },
		{ 1, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 2, 0, 8, 0 },
		{ 5, 0, 8, 4, 0, 0, 5, 0, 2, 4, 0, 0, 4, 0, 4, 4 },
		{ 1, 0, 4, 0, 2, 0, 5, 1, 0, 0, 5, 0, 2, 4,12, 4 },
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	};
	int userPresetID = (int)numPresets-2;

	bool triggered = false;
	bool pulsed[4];
	int curTrig = 0;
	int numSteps = 16;
	bool isFirstPass = true;
	int delayCounter = 0;
	bool delayBeforePlay = true;
	int mutationFrequency = 2;
	int measureCount = 1;
	bool mutatePattern = false;
	int mutateCounts[4];
	int tempPattern[16];
	bool isMutated = false;
	bool canMutate[4];

	dsp::SchmittTrigger inTrigger;
	dsp::SchmittTrigger inReset;
	dsp::PulseGenerator monoStepOutput[4];
	dsp::BooleanTrigger mutateTrigger;

	Rhythm1101() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		for (int i = 0; i < 16; i++) {
			configParam(PARAM_STEP + i, 0.f, 15.f, 0.f, string::f("Step %d", i+1));
			paramQuantities[PARAM_STEP+i]->snapEnabled = true;
		}
		configParam(PRESET_PARAM, 1.f, numPresets, 1.f, "Pattern");
		configParam(STEPS_PARAM, 1.f, 16.f, 16.f, "Number of Steps");
		configParam(MUTATE_PARAM, 0,1,1, "Algorithmically Alter Pattern");
		configParam(MUTATETRACK_PARAM, 0.f, 15.f, 0.f, "Tracks to Mutate");
		paramQuantities[MUTATETRACK_PARAM]->snapEnabled = true;
		configInput(TRIGGER_INPUT, "Pick (Trigger Step)");
		configInput(RESET_INPUT, "Reset");
		configInput(PRESET_INPUT, "Preset 0-10v CV");
		for (int i = 0; i < 4; i++) {
			configOutput(MONOTRIG_OUTPUT + i, string::f("Trigger %d", i+1));
		}

		paramQuantities[PRESET_PARAM]->snapEnabled = true;
		paramQuantities[STEPS_PARAM]->snapEnabled = true;

		curStep = -1;
		for (int i=0;i<4;i++) {
			pulsed[i] = false;
			mutateCounts[i] = 0;
			canMutate[i] = false;
        }
		oldPreset = -1;
	}

	void process(const ProcessArgs& args) override {


		curPreset = params[PRESET_PARAM].getValue()-1;
		if (inputs[PRESET_INPUT].isConnected()) {
			curPreset = clamp(rescale(inputs[PRESET_INPUT].getVoltage(),0.f,10.f,1.f,numPresets),1.f,numPresets)-1;
			params[PRESET_PARAM].setValue(curPreset+1);
		}

		numSteps = params[STEPS_PARAM].getValue() - 1;

		if (inReset.process(inputs[RESET_INPUT].getVoltage(), 0.01f, 2.f)) {
			curStep = -1;
			curTrig = 0;
			delayCounter = 0;
			delayBeforePlay = true;
			measureCount = 1;
			// reset pattern to base pattern
			for (int i=0;i<16;i++) {
				tempPattern[i] = presets[curPreset][i];
            }
		}

		if (mutateTrigger.process(params[MUTATE_PARAM].getValue() > 0.f)) {
			mutatePattern ^= true;
			measureCount = 1;
        }


		if (inTrigger.process(inputs[TRIGGER_INPUT].getVoltage(), 0.01f, 2.f)) {
			triggered = true;
			delayCounter = 0;
			delayBeforePlay = true;
		}


		if (curPreset != oldPreset) {
			// reset knobs
			for (int i=0;i<16;i++) {
				params[PARAM_STEP+i].setValue(presets[curPreset][i]);
            }
			// copy base pattern into temp to maybe mutate
			for (int i=0;i<16;i++) {
				tempPattern[i] = presets[curPreset][i];
			}
			oldPreset = curPreset;
        }

		// set pattern to knobs
		for (int i=0;i<16;i++) {
			presets[curPreset][i] = params[PARAM_STEP+i].getValue();
        }
		
		int* marr = getBinaryOfNum((int)params[MUTATETRACK_PARAM].getValue());
		for (int i=0;i<4;i++) {
			lights[LIGHT_MUTATETRACKS+i].setBrightness(marr[i] == 1 ? 0.75f : 0.f);
			canMutate[i] = marr[i] == 1 ? true : false;
        }

		if (triggered) {
			// this delay accounts for changes in incoming CV but doesn't add noticeable latency
			if (delayBeforePlay) {
				delayCounter++;
				if (delayCounter > 5) {
					delayBeforePlay = false;
                }
			} else {
				curStep++;
				if (curStep > numSteps) {
					curStep = 0;
					// copy base pattern into temp to maybe mutate
					for (int i=0;i<16;i++) {
						tempPattern[i] = presets[curPreset][i];
                    }
					measureCount++;
					if (mutatePattern && (measureCount%(mutationFrequency+1) == 0)) {
						performMutation();
                    }
				}

				curTrig = tempPattern[curStep] - 1;
				for (int i=0;i<4;i++) {
					if (getBinaryOfNumAt(tempPattern[curStep],3-i) == 1) {
						monoStepOutput[i].trigger(1e-3f);
                    }
                }
				triggered = false;
			}
		}

		for (int i=0;i<4;i++) {
			pulsed[i] = monoStepOutput[i].process(args.sampleTime);
			outputs[MONOTRIG_OUTPUT+i].setVoltage(pulsed[i] ? 10.f : 0.f);
        }

		for (int i=0;i<16;i++) {
			lights[LIGHT_STEP+i].setBrightness(i == curStep ? .75f : 0.f);
		}
		lights[MUTATE_LIGHT].setBrightness(mutatePattern ? 0.95f : 0.f);
	}

	void performMutation() {
		isMutated = false;
		int maxMutates = 0;
		for (int i=0;i<4;i++) {
			mutateCounts[i] = 0; // reset mutation counts
			for (int s=0;s<numSteps;s++) {
				int* barr = getBinaryOfNum(tempPattern[s]);
				if ((i == 0) && canMutate[0] && (random::uniform() > 0.2f)) {
					maxMutates = random::uniform() > 0.5f ? 3 : 1;
					if (mutateCounts[0] < maxMutates) {
						if ( ((s == 0) || (s == 2) || (s == 3) || (s == 5) || (s == 7) || (s == 8) ||  (s == 11) || (s == 14) || (s == 15)) && (barr[0] == 0) && (barr[1] == 0)) {
							// if random, trigger
							if (random::uniform() > 0.7f) {
								tempPattern[s] += 1;
								mutateCounts[0]++;
								isMutated = true;
                            }
						}
					}
				} else if ((i == 1) && canMutate[1] && (random::uniform() > 0.3f)) {
					maxMutates = random::uniform() > 0.5f ? 2 : 1;
					if (mutateCounts[1] < maxMutates) {
						if ( ((s == 3) || (s == 7) || (s == 9) || (s == 11) || (s == 13) || (s == 14) || (s == 15)) && (barr[1] == 0) && (barr[0] == 0) ) {
							// if random, trigger
							if (random::uniform() > 0.8f) {
								tempPattern[s] += 2;
								mutateCounts[1]++;
								isMutated = true;
                            }
						}
					}
				} else if ((i == 2) && canMutate[2] ) {
					maxMutates = random::uniform() > 0.5f ? 4 : 2;
					if (mutateCounts[2] < maxMutates) {
						if ( ((s == 2) || (s == 3) || (s == 6) || (s == 7) || (s == 8) || (s == 9) || (s == 11) || (s == 13) || (s == 14) || (s == 15)) && (barr[2] == 0)) {
							// if random, trigger
							if (random::uniform() > 0.6f) {
								tempPattern[s] += 4;
								mutateCounts[2]++;
								isMutated = true;
                            }
						}
					}
				} else if ((i == 3) && canMutate[3] && (random::uniform() > 0.3f)) {
					maxMutates = random::uniform() > 0.5f ? 3 : 2;
					if (mutateCounts[3] < maxMutates) {
						if ( ((s == 0) || (s == 3) || (s == 4) || (s == 6) || (s == 7) || (s == 10) || (s == 13) || (s == 15)) && (barr[3] == 0)) {
							// if random, trigger
							if (random::uniform() > 0.7f) {
								tempPattern[s] += 8;
								mutateCounts[3]++;
								isMutated = true;
                            }
						}
					}
				}
				tempPattern[s] = rack::math::clamp(tempPattern[s],0,15);
			}
		}
    }
	int* getBinaryOfNum(int num) {
		static int binaryNum[4];
		for (int i=0;i<4;i++) {
			binaryNum[i] = 0;
        }
 		int j = 0;
		while (num > 0) {
			binaryNum[j] = num % 2;
			num = num / 2;
			j++;
		}
		return binaryNum;
    }


	void onRandomize() override {
		for (int i = 0; i < 16; i++) {
			presets[curPreset][i] = rack::math::clamp(static_cast<int>(floor((random::uniform() * 16.0f)))-1,0,15);
		}
		params[PRESET_PARAM].setValue(curPreset+1);
	}

	json_t* dataToJson() override {
		json_t* root = json_object();
		json_t* val = json_boolean(mutatePattern);
		json_object_set_new(root, "mutatePattern", val);
		val = json_integer(mutationFrequency);
		json_object_set_new(root, "mutationFrequency", val);
		json_t* patternsj = json_array();
		for (int i=0;i<numPresets;i++) {
			json_t* patternj = json_array();
			for (int j=0;j<16;j++) {
				json_array_append_new(patternj, json_integer(presets[i][j]));
            }
			json_array_append_new(patternsj, patternj);
			// json_decref(patternj); // may not need
        }
		json_object_set_new(root, "patterns", patternsj);
		//json_decref(patternsj);

		return root;
    }

	void dataFromJson(json_t* root) override {
		json_t* val = json_object_get(root, "mutatePattern");
		if (val) {
			mutatePattern = json_boolean_value(val);
		}
		val = json_object_get(root, "mutationFrequency");
		if (val) {
			mutationFrequency = json_integer_value(val);
        }
		json_t* patterns = json_object_get(root, "patterns");
		if (patterns) {
			for (int i=0;i<numPresets;i++) {
				json_t* pattern = json_array_get(patterns,i);
				if (pattern) {
					for (int j=0;j<16;j++) {
						presets[i][j] = json_integer_value(json_array_get(pattern,j));
					}
				}
            }
        }
    }

	int getBinaryOfNumAt(int num, int at)
	{
		int binaryNum[4];
		for (int i=0;i<4;i++) {
			binaryNum[i] = 0;
        }
 		int j = 0;
		while (num > 0) {
			binaryNum[j] = num % 2;
			num = num / 2;
			j++;
		}
		return binaryNum[3-at];
	}

	void shiftPatternRight() {
		int s = 16;
		int last = presets[curPreset][s-1];
		for (int i=s-1;i>0;i--) {
			presets[curPreset][i] = presets[curPreset][i-1];
        }
		presets[curPreset][0] = last;
		oldPreset = -1;
    }
	void shiftPatternLeft() {
		int s = 16;
		int first = presets[curPreset][0];
		for (int i=0;i<s-1;i++) {
			presets[curPreset][i] = presets[curPreset][i+1];
        }
		presets[curPreset][s-1] = first;
		oldPreset = -1;
    }
	void clearCurrentPattern() {
		for (int i=0;i<16;i++) {
			presets[curPreset][i] = 0;
		}
		oldPreset = -1; // to refresh knobs
    }
	void clearAllPatterns() {
		for (int p=0;p<numPresets;p++) {
			for (int i=0;i<16;i++) {
				presets[p][i] = 0;
			}
        }
		oldPreset = -1; // refresh knobs
    }
	void resetToFactory() {
		for (int p=0;p<numPresets;p++) {
			for (int i=0;i<16;i++) {
				presets[p][i] = factoryPresets[p][i];
			}
		}
		oldPreset = -1; // knobs
    }
};


struct Rhythm1101Widget : ModuleWidget {
	Rhythm1101Widget(Rhythm1101* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Rhythm1101-White.svg"), asset::plugin(pluginInstance, "res/Rhythm1101-Dark.svg")));

		Rhythm1101Display<Rhythm1101>* display = createWidget<Rhythm1101Display<Rhythm1101>>(mm2px(Vec(8., 7.3)));
		display->box.size = mm2px(Vec(80.5, 25.0));
		display->displaySize = Vec(80.5, 25.0);
		display->module = module;
		addChild(display);

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// preset
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(31.170, 90.744)), module, Rhythm1101::PRESET_PARAM));

		// pattern
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(31.170, 109.702)), module, Rhythm1101::PRESET_INPUT));

		// trigger in
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(12.051, 88.006)), module, Rhythm1101::TRIGGER_INPUT));

		// reset
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(12.051, 109.702)), module, Rhythm1101::RESET_INPUT));

		// steps
		addParam(createParamCentered<Trimpot>(mm2px(Vec(50.564, 87.79)), module, Rhythm1101::STEPS_PARAM));

		// mutate
		addParam(createParamCentered<LEDButton>(mm2px(Vec(48.697, 102.943)), module, Rhythm1101::MUTATE_PARAM));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(48.697, 102.943)), module, Rhythm1101::MUTATE_LIGHT));
		addParam(createParamCentered<Trimpot>(mm2px(Vec(48.697, 110.827)), module, Rhythm1101::MUTATETRACK_PARAM));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(54.307, 101.59)), module, Rhythm1101::LIGHT_MUTATETRACKS +3));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(54.307, 105.294)), module, Rhythm1101::LIGHT_MUTATETRACKS+2));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(54.307, 108.998)), module, Rhythm1101::LIGHT_MUTATETRACKS+1));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(54.307, 112.703)), module, Rhythm1101::LIGHT_MUTATETRACKS+0));

		// mono trig outs
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(70.181, 96.818)), module, Rhythm1101::MONOTRIG_OUTPUT + 0));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(80.714, 96.818)), module, Rhythm1101::MONOTRIG_OUTPUT + 1));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(70.181, 110.047)), module, Rhythm1101::MONOTRIG_OUTPUT+ 2));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(80.714, 110.047)), module, Rhythm1101::MONOTRIG_OUTPUT+ 3));

		// steps
		for (int i=0;i<8;i++) {
			addParam(createParamCentered<Trimpot>(mm2px(Vec(11.218+(i*10.584), 45.217)), module, Rhythm1101::PARAM_STEP + i));
			addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(11.218+(i*10.584), 50.542)), module, Rhythm1101::LIGHT_STEP + i));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(11.218+(i*10.584), 62.679)), module, Rhythm1101::PARAM_STEP + (i+8)));
			addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(11.218+(i*10.584), 68.005)), module, Rhythm1101::LIGHT_STEP + (i+8)));
		}

	}

	void appendContextMenu(Menu* menu) override {
		Rhythm1101* module = getModule<Rhythm1101>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Rhythm1101"));
		menu->addChild(createSubmenuItem("Shift Pattern", "", [=](Menu* menu) {
				menu->addChild(createMenuItem("Shift Right", "", [=]() {module->shiftPatternRight();}));
				menu->addChild(createMenuItem("Shift Left", "", [=]() {module->shiftPatternLeft();}));
			}
		));
		menu->addChild(createIndexPtrSubmenuItem("Mutation Frequency",
			{"1:1", "1:2", "1:3", "1:4"},
			&module->mutationFrequency
		));
		menu->addChild(createMenuItem("Clear Current Pattern", "",  [=]() {module->clearCurrentPattern();}));
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Rhythm1101 CAUTION"));
		menu->addChild(createMenuItem("Clear All Patterns", "",  [=]() {module->clearAllPatterns();}));
		menu->addChild(createMenuItem("Reset to Factory Patterns", "",  [=]() {module->resetToFactory();}));
	}

};


Model* modelRhythm1101 = createModel<Rhythm1101, Rhythm1101Widget>("Rhythm1101");
