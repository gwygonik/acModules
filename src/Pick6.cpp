#include "acModules.hpp"
#include "Pick6Display.cpp"

struct Pick6 : Module {

	struct pick6pMessage {
		float stepValues[16];
		int curCustomPattern;
	};

	enum ParamId {
		PRESET_PARAM,
		EOP_PARAM,
		RIFF_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		TRIGGER_INPUT,
		RESET_INPUT,
		PRESET_INPUT,
		POLYCV_INPUT,
		INPUTS_LEN
	};

	enum OutputId {
		MONOCV_OUTPUT,
		POLYTRIG_OUTPUT,
		MONOTRIG_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		RIFF_LIGHT,
		EXTENDER_LIGHT,
		LIGHTS_LEN
    };

	int curStep, steps, lastSteps, filter_steps, seed;
	int curPreset, basePreset, oldPreset;
	static constexpr float numPresets = 50.f;
	int presets[(int)numPresets][8] = {
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 1, 2, 3, 4, 5, 6, 0, 0 },
		{ 1, 3, 2, 5, 4, 6, 0, 0 },
		{ 6, 5, 4, 3, 2, 1, 0, 0 },
		{ 6, 4, 5, 3, 1, 2, 0, 0 },
		{ 3, 4, 5, 6, 0, 0, 0, 0 },
		{ 3, 4, 6, 5, 0, 0, 6, 5 },
		{ 4, 5, 6, 4, 5, 6, 4, 6 },
		{ 0, 3, 4, 5, 0, 3, 4, 6 },
		{ 0, 3, 4, 5, 0, 4, 3, 6 },
		{ 0, 3, 4, 5, 0, 6, 5, 4 },
		{ 0, 3, 4, 5, 0, 6, 5, 3 },
		{ 0, 3, 4, 5, 0, 6, 5, 6 },
		{ 0, 5, 3, 4, 0, 6, 5, 3 },
		{ 2, 5, 3, 4, 2, 6, 5, 3 },
		{ 1, 5, 6, 4, 1, 5, 4, 3 },
		{ 2, 5, 6, 4, 2, 5, 4, 5 },
		{ 6, 5, 6, 4, 6, 3, 4, 5 },
		{ 6, 5, 4, 3, 6, 3, 5, 4 },
		{ 1, 0, 0, 6, 0, 0, 5, 6 },
		{ 1, 1, 0, 0, 6, 0, 0, 0 },
		{ 1, 1, 5, 1, 1, 1, 6, 0 },
		{ 1, 1, 6, 1, 5, 1, 4, 0 },
		{ 1, 1, 6, 1, 5, 1, 4, 5 },
		{ 1, 1, 3, 1, 1, 2, 1, 1 },
		{ 1, 1, 3, 4, 1, 1, 4, 3 },
		{ 6, 5, 0, 4, 0, 0, 0, 0 },
		{ 4, 5, 0, 6, 0, 0, 0, 0 },
		{ 6, 5, 4, 3, 1, 2, 3, 4 },
		{ 0, 2, 0, 0, 1, 0, 3, 4 },
		{ 0, 3, 4, 0, 0, 2, 0, 3 },
		{ 1, 1, 0, 0, 0, 0, 4, 0 },
		{ 0, 0, 4, 0, 0, 0, 5, 0 },
		{ 0, 0, 5, 0, 0, 0, 4, 6 },
		{ 5, 3, 2, 4, 5, 2, 4, 3 },
		{ 5, 3, 2, 6, 5, 3, 4, 6 },
		{ 0, 4, 6, 0, 0, 5, 0, 0 },
		{ 0, 4, 6, 0, 0, 5, 0, 6 },
		{ 3, 2, 4, 0, 4, 3, 0, 5 },
		{ 3, 0, 0, 4, 0, 0, 5, 0 },
		{ 4, 0, 6, 0, 5, 3, 4, 0 },
		{ 2, 0, 6, 5, 2, 3, 6, 5 },
		{ 2, 0, 5, 4, 2, 0, 4, 5 },
		{ 2, 3, 5, 4, 2, 3, 6, 4 },
		{ 2, 4, 2, 5, 2, 4, 2, 5 },
		{ 5, 1, 6, 2, 5, 3, 6, 4 },
		{ 5, 1, 3, 5, 6, 4, 2, 1 },
		{ 1, 1, 2, 2, 0, 0, 5, 3 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // user
		{ 0, 0, 0, 0, 0, 0, 0, 0 }, // user
	};
	int userPresetID = (int)numPresets-2;
	pick6pMessage pick6Messages[2][1];

	bool triggered = false;
	float noteValues[6];
	bool muteValues[6];
	bool muteNotes = false;
	bool pulsed[6];
	bool monoPulsed;
	int curTrig = 0;
	float inCV = 0.f;
	bool canOutputCV = false;
	bool isFirstPass = true;
	int delayCounter = 0;
	bool delayBeforePlay = true;
	bool muteToZero = false;
	int measureCount = 1;
	bool useSmartRiff = false;
	int smartRiffMuteCount = 0;
	int smartRiffPickCount = 0;
	int smartRiffRepeatCount = 0;
	int smartRiffLastIndex = 0;
	bool isPlayingSmartRiffNote = false;
	bool smartRiffInBlankPattern = false;
	float voltPerNote = 0.08333f;
	float baseOctave = 0.0f;
	// EADGBE
	// 5,10,15,20,24,29 then -1
	float baseOffsets[6] = {4,9,14,19,23,28};


	dsp::SchmittTrigger inTrigger;
	dsp::SchmittTrigger inReset;
	dsp::PulseGenerator pickOutput[8];  // main
	dsp::PulseGenerator monoPickOutput;
	dsp::BooleanTrigger riffTrigger;

	Pick6() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		// set the right expander (pick6p) message instances
		rightExpander.producerMessage = pick6Messages[0];
		rightExpander.consumerMessage = pick6Messages[1];

		configParam(PRESET_PARAM, 1.f, numPresets, 1.f, "Preset");
		configParam(RIFF_PARAM, 0,1,1, "Smart Riff");
		configInput(TRIGGER_INPUT, "Pick (Trigger Step)");
		configInput(RESET_INPUT, "Reset");
		configInput(PRESET_INPUT, "Preset 0-10v CV");
		configInput(POLYCV_INPUT, "Polyphonic CV (Notes and Mutes)");
		configOutput(MONOCV_OUTPUT, "Switched CV");
		configOutput(POLYTRIG_OUTPUT, "Polyphonic Trigger");
		configOutput(MONOTRIG_OUTPUT, "Mono Trigger");
		configSwitch(EOP_PARAM, 0.f, 2.f, 2.f, "At End of Pattern", {"Silent Bar, Loop", "Next Pattern, Loop", "Loop"});

		paramQuantities[PRESET_PARAM]->snapEnabled = true;

		curStep = -1;
		for (int i=0;i<6;i++) {
			pulsed[i] = false;
        }
		oldPreset = -1;

	}

	void process(const ProcessArgs& args) override {

		int tmpPreset = params[PRESET_PARAM].getValue()-1;
		if (isFirstPass) {
			curPreset = params[PRESET_PARAM].getValue()-1;
			basePreset = curPreset;
        } else {
			if (tmpPreset != basePreset) {
				// we changed preset manually
				curPreset = basePreset = tmpPreset;
            }
        }
		if (inputs[PRESET_INPUT].isConnected() && isFirstPass) {
			curPreset = clamp(rescale(inputs[PRESET_INPUT].getVoltage(),0.f,10.f,1.f,numPresets),1.f,numPresets)-1;
			params[PRESET_PARAM].setValue(curPreset+1);
		}

		if (rightExpander.module) {
			if (rightExpander.module->model == modelPick6p) {
				if (curPreset >= userPresetID) {
					pick6pMessage *messageFromExpander = (pick6pMessage*)(rightExpander.module->leftExpander.consumerMessage);

					for (int i=0;i<8;i++) {
						presets[userPresetID][i] = messageFromExpander->stepValues[i];
						presets[userPresetID+1][i] = messageFromExpander->stepValues[i+8];
					}
								
					rightExpander.messageFlipRequested = true;
                }
				// set this regardless of in custom pattern so lights light correctly
				pick6pMessage *messageToExpander = (pick6pMessage*)(rightExpander.module->leftExpander.producerMessage);

				if (curPreset >= userPresetID) {
					messageToExpander->curCustomPattern = (curPreset == userPresetID ? 1 : 2);
                } else {
					messageToExpander->curCustomPattern = 0;
                }
				rightExpander.messageFlipRequested = true;

				// turn on light
				lights[EXTENDER_LIGHT].setBrightness(0.95f);
			} else {
				// turn off light
				lights[EXTENDER_LIGHT].setBrightness(0.f);
            }
		}


		if (riffTrigger.process(params[RIFF_PARAM].getValue() > 0.f)) {
			useSmartRiff ^= true;
			smartRiffRepeatCount = 0;
			measureCount = 0;
        }


		if (inReset.process(inputs[RESET_INPUT].getVoltage(), 0.01f, 2.f)) {
			curStep = -1;
			curTrig = 0;
			if (!isFirstPass) curPreset = basePreset;
			delayCounter = 0;
			delayBeforePlay = true;
			smartRiffRepeatCount = 0;
			oldPreset = -1;
			measureCount = 1;
			isFirstPass = true;
		}

		if (inTrigger.process(inputs[TRIGGER_INPUT].getVoltage(), 0.01f, 2.f)) {
			triggered = true;
			delayCounter = 0;
			delayBeforePlay = true;
		}

		if (inputs[POLYCV_INPUT].isConnected() && (inputs[POLYCV_INPUT].getChannels() >= 12)) {
			for (int i=0;i<6;i++) {
				noteValues[i] = inputs[POLYCV_INPUT].getPolyVoltage(i);
				muteValues[i] = inputs[POLYCV_INPUT].getPolyVoltage(i+6) > 4.9f;
            }
			canOutputCV = true;
		} else {
			// if no CV input connected, use E chord just for fun
			for (int i=0;i<6;i++) {
				noteValues[i] = baseOctave + (baseOffsets[i] * voltPerNote);
				muteValues[i] = false;
            }
			canOutputCV = true;
        }


		if (curPreset != oldPreset) {
			smartRiffMuteCount = 0;
			for (int i=0;i<8;i++) {
				if ((presets[curPreset][i] == 0) || (muteValues[i])) {
					smartRiffMuteCount++;
				} else {
					smartRiffPickCount++;
                }
            }
        }

		if (smartRiffRepeatCount > 12) {
			smartRiffRepeatCount = 0;
        }



		if (triggered) {
			//smartRiffLastIndex = -1;
			// this delay accounts for changes in incoming CV but doesn't add noticeable latency
			if (delayBeforePlay) {
				delayCounter++;
				if (delayCounter > 5) {
					delayBeforePlay = false;
                }
			} else {
				curStep++;
				if (curStep > 7) {
					curStep = 0;
					measureCount++;
					if (measureCount > 32) measureCount = 1;
					if (isFirstPass) {
						if (params[EOP_PARAM].getValue() == 1.0f) {
							// next then back
							basePreset = curPreset;
							curPreset++;
							if (curPreset > numPresets-1) {
								curPreset = 1;
							}
							isFirstPass = false;
						} else if (params[EOP_PARAM].getValue() == 0.0f) {
							// silent bar
							basePreset = curPreset;
							curPreset = 0;
							isFirstPass = false;
						} else {
							// just do the same loop
							isFirstPass = true;
                        }
					} else {
						// not first pass
						curPreset = basePreset;
						isFirstPass = true;
					}

				}
				//curStep %= 8;

				bool triggerOut = true;

				if (presets[curPreset][curStep] != 0) {
					// a normal note
					curTrig = presets[curPreset][curStep] - 1;
					if (muteValues[curTrig]) {
						// muted in chord
						triggerOut = false;
						// but... smart riff?
						if (useSmartRiff) {
							isPlayingSmartRiffNote = false;
							// move the muted note to a non-chord-muted note
							int nmidx = (int)floor((rand()%3) + 3);
							int prevStepNote = curStep > 0 ? (presets[curPreset][curStep-1]) : 0;
							int nextStepNote = curStep < 7 ? (presets[curPreset][curStep+1]) : 0;
							while (muteValues[nmidx] || nmidx == smartRiffLastIndex || nmidx == (prevStepNote) || nmidx == (nextStepNote)) {
								nmidx++;
                            }
							if ((nmidx < 7) && (nmidx > 0)) {
								curTrig = (nmidx-1);
								triggerOut = true;
								smartRiffLastIndex = nmidx;
								isPlayingSmartRiffNote = true;
                            }
                        }
                    } else {
						// mute normal notes when it seems musically appropriate
						if (useSmartRiff) {
							if(smartRiffPickCount > 6) { // patterns that are very full
								if (measureCount % 8==0) { // every 8th bar
									if ((curStep == 3) || (curStep > 5) ) {
										if ((int)floor(rand()%10) > (12-curStep)) { // 4 step rarely, 7th, more frequent, 8th most frequent
											triggerOut = false;
										}
									}
                                }
                            }
                        }
                    }
					inCV = (!triggerOut && muteToZero) ? 0.f : noteValues[curTrig];
				} else {
					// a pattern-skipped step
					triggerOut = false;
					if (muteToZero) inCV = 0.f;
					isPlayingSmartRiffNote = false;
					// unless...
					if (useSmartRiff && (curPreset > 0 || smartRiffInBlankPattern)) {
						isPlayingSmartRiffNote = false;
						if ((int)floor(rand()%40) > 34-smartRiffRepeatCount ) {
							smartRiffRepeatCount++;
							if (smartRiffRepeatCount > 2) {
								// play a non-chord-muted note
								int nmidx = (int)floor((rand()%3)+3);
								int prevStepNote = curStep > 0 ? (presets[curPreset][curStep-1]) : 0;
								int nextStepNote = curStep < 7 ? (presets[curPreset][curStep+1]) : 0;
								while ( (muteValues[nmidx] || nmidx == smartRiffLastIndex || nmidx == prevStepNote || nmidx == nextStepNote)) {
									nmidx++;
								}
								if ((nmidx < 7) && (nmidx > 0)) {
									curTrig = (nmidx-1);
									smartRiffLastIndex = nmidx;
									isPlayingSmartRiffNote = true;
									triggerOut = true;
									inCV = noteValues[curTrig]; // override in case muteToZero
								} else {
									// just in case
									triggerOut = false;
								}
							} else {
								triggerOut = false;
                            }
						}
					} else {
						isPlayingSmartRiffNote = true;
                    }
                }
				if (triggerOut) {
					pickOutput[curTrig].trigger(1e-3f);
					monoPickOutput.trigger(1e-3f);
                }
				triggered = false;
			}
		}


		for (int i=0;i<6;i++) {
			pulsed[i] = pickOutput[i].process(args.sampleTime);
			outputs[POLYTRIG_OUTPUT].setVoltage(pulsed[i] ? 10.f : 0.f, i);
        }
		outputs[POLYTRIG_OUTPUT].setChannels(6);

		monoPulsed = monoPickOutput.process(args.sampleTime);
		outputs[MONOTRIG_OUTPUT].setVoltage(monoPulsed ? 10.f : 0.f);

		if (canOutputCV) {
			outputs[MONOCV_OUTPUT].setVoltage(inCV);
        }

		lights[RIFF_LIGHT].setBrightness(useSmartRiff ? 0.95f : 0.f);
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* val = json_boolean(useSmartRiff);
		json_object_set_new(rootJ, "useSmartRiff", val);
		val = json_boolean(muteToZero);
		json_object_set_new(rootJ, "muteToZero", val);
		val = json_boolean(smartRiffInBlankPattern);
		json_object_set_new(rootJ, "smartRiffInBlankPattern", val);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		// smart riff
		json_t* val = json_object_get(rootJ, "useSmartRiff");
		if (val) {
			useSmartRiff = json_boolean_value(val);
		}
		// mute to zero
		val = json_object_get(rootJ, "smartRiffInBlankPattern");
		if (val) {
			smartRiffInBlankPattern = json_boolean_value(val);
		}
	}
};


struct Pick6Widget : ModuleWidget {
	Pick6Widget(Pick6* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Pick6-White.svg"), asset::plugin(pluginInstance, "res/Pick6-Dark.svg")));

		Pick6Display<Pick6>* display = createWidget<Pick6Display<Pick6>>(mm2px(Vec(4.4, 7.144)));
		display->box.size = mm2px(Vec(42.0, 24.0));
		display->displaySize = Vec(42.0, 24.0);
		display->module = module;
		addChild(display);

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// preset
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(25.283, 45.783)), module, Pick6::PRESET_PARAM));

		// pattern
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(42.217, 45.783)), module, Pick6::PRESET_INPUT));

		// trigger in
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.739, 45.783)), module, Pick6::TRIGGER_INPUT));

		// reset
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(8.739, 69.752)), module, Pick6::RESET_INPUT));

		// end of pattern switch
		addParam(createParamCentered<CKSSThree>(mm2px(Vec(39.960, 69.173)), module, Pick6::EOP_PARAM));

		// smart riff
		addParam(createParamCentered<LEDButton>(mm2px(Vec(25.225, 63.722)), module, Pick6::RIFF_PARAM));
		addChild(createLightCentered<MediumLight<WhiteLight>>(mm2px(Vec(25.225, 63.722)), module, Pick6::RIFF_LIGHT));

		// poly CV input
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(14.817, 93.131)), module, Pick6::POLYCV_INPUT));

		// switched mono CV out
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(14.817, 109.006)), module, Pick6::MONOCV_OUTPUT));

		// mono trig out
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(35.983, 109.006)), module, Pick6::MONOTRIG_OUTPUT));
		// poly trig out
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(35.983, 93.131)), module, Pick6::POLYTRIG_OUTPUT));

		// expander connected light
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(35.124, 121.322)), module, Pick6::EXTENDER_LIGHT));
	}

	void appendContextMenu(Menu* menu) override {
		Pick6* module = getModule<Pick6>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Pick6 Preferences"));
		menu->addChild(createBoolPtrMenuItem("Muted Note CV to 0V", "", &module->muteToZero));
		menu->addChild(createBoolPtrMenuItem("Smart Riff in Blank Pattern", "", &module->smartRiffInBlankPattern));
	}

};


Model* modelPick6 = createModel<Pick6, Pick6Widget>("Pick6");
