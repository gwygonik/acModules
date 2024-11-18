#include "acModules.hpp"
#include "Merc8orDisplay.cpp"

struct Merc8or : Module {

	struct MOREc8orMessage {
		bool invert;
		bool linkHL;
		bool hasCVHigh;
		float cvHigh;
		bool hasCVLow;
		float cvLow;
	};

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
		LIGHT_EXTENDED,
		ENUMS(LIGHT_USEHIGH,2),
		ENUMS(LIGHT_USELOW,2),
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
	float curSampleRate = 0.f;
	bool isLinked = false;
	bool isInverted = false;
	float linkDiff = -1.f;
	bool hasExpander = false;
	bool usingExpanderCVhigh = false;
	bool usingExpanderCVlow  = false;
	bool isSnapping = false;
	bool snapToSemi = false;
	bool addSnapOffset = false;
	float voltPerNote = 0.08333f;
	float halfVoltPerNote = 0.041665f;


	MOREc8orMessage MOREc8orMessages[2][1]; // messages from right module (MOREc8or module))


	Merc8or() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PARAM_HIGH_IN, -10.f, 10.f, 5.f, "High voltage");
		configParam(PARAM_LOW_IN, -10.f, 10.f, -5.f, "Low voltage");
		configParam(PARAM_HIGH_OUT, -10.f, 10.f, 5.f, "High voltage");
		configParam(PARAM_LOW_OUT, -10.f, 10.f, -5.f, "Low voltage");
		configInput(INPUT_CV, "CV");
		configOutput(OUTPUT_CV, "CV");

		// set the right expander (MOREc8or) message instances
		rightExpander.producerMessage = MOREc8orMessages[0];
		rightExpander.consumerMessage = MOREc8orMessages[1];

	}

	void process(const ProcessArgs& args) override {
		curSampleRate = args.sampleRate;

        inCVlow = params[PARAM_LOW_IN].getValue();
        inCVhigh  = params[PARAM_HIGH_IN].getValue();
        // UI blockers
        getParamQuantity(PARAM_HIGH_IN)->minValue = inCVlow+0.05f;
        getParamQuantity(PARAM_LOW_IN)->maxValue = inCVhigh-0.05f;
		usingExpanderCVhigh = false;
		usingExpanderCVlow  = false;

		if (rightExpander.module) {
			if (rightExpander.module->model == modelMOREc8or) {
				hasExpander = true;

				MOREc8orMessage *messageFromExpander = (MOREc8orMessage*)(rightExpander.module->leftExpander.producerMessage);

				isLinked = messageFromExpander->linkHL;

				if (messageFromExpander->invert) {
					invertOutput();
                }

				if (messageFromExpander->hasCVHigh && !isLinked) {
					// ignore high value from expander if linked
					outCVhigh = messageFromExpander->cvHigh;
					params[PARAM_HIGH_OUT].setValue(outCVhigh);
					usingExpanderCVhigh = true;
                }
				if (messageFromExpander->hasCVLow) {
					usingExpanderCVlow = true;
					if (isLinked && linkDiff >= 0.f) {
						// we need to make sure incoming low doesn't go outside the range
						if (isInverted) {
							outCVlow = clamp(messageFromExpander->cvLow, -10.f+linkDiff, 10.f);
                        } else {
							outCVlow = clamp(messageFromExpander->cvLow, -10.f, 10.f-linkDiff);
                        }
                    } else {
						outCVlow = messageFromExpander->cvLow; // expander clamps to full range
                    }
					params[PARAM_LOW_OUT].setValue(outCVlow);
                }

				rightExpander.messageFlipRequested = true;

				// turn on light
				lights[LIGHT_EXTENDED].setBrightness(0.95f);
			} else {
				// turn off light
				lights[LIGHT_EXTENDED].setBrightness(0.f);
            }
		} else {
			// turn off light
			lights[LIGHT_EXTENDED].setBrightness(0.f);
			hasExpander = false;
        }

        outCVlow = params[PARAM_LOW_OUT].getValue();
		if (isLinked && !hasExpander) {
			// we loaded settings but don't have the expander
			isLinked = false;
        }

		if (!isLinked || linkDiff < 0.f) {
			outCVhigh  = params[PARAM_HIGH_OUT].getValue();
			if (outCVhigh < outCVlow) {
				isInverted = true;
			} else {
				isInverted = false;
			}
        }

		if (isSnapping) {
			float tmpOutLow = 0.f;
			float tmpOutHigh = 0.f;
			if (snapToSemi) {
				// semitones
				tmpOutLow = snapToSemitone(outCVlow);
				tmpOutHigh = snapToSemitone(outCVhigh);
            } else {
				// octaves
				tmpOutLow = rack::math::clamp(round(outCVlow),-10.f,10.f);
				tmpOutHigh = rack::math::clamp(round(outCVhigh),-10.f,10.f);
            }
			if (addSnapOffset) {
				tmpOutLow += halfVoltPerNote;
				tmpOutHigh += halfVoltPerNote;
            }
			outCVlow = tmpOutLow;
			outCVhigh = tmpOutHigh;
        }

		if (isLinked) {
			if (linkDiff < 0.f) {
				linkDiff = abs(outCVlow - outCVhigh);
				if (isInverted) {
					// inverted
			        getParamQuantity(PARAM_LOW_OUT)->minValue = -10.f+linkDiff;
			        getParamQuantity(PARAM_LOW_OUT)->maxValue = 10.f;
				} else {
			        getParamQuantity(PARAM_LOW_OUT)->minValue = -10.f;
			        getParamQuantity(PARAM_LOW_OUT)->maxValue = 10.f-linkDiff;
				}
            }
			if (abs(outCVhigh - outCVlow) != linkDiff) {
				if (isInverted) {
					// we're inverted
					if (outCVlow - linkDiff >= -10.0f) {
						outCVhigh = outCVlow - linkDiff;
					}
                } else {
					if (outCVlow + linkDiff <= 10.0f) {
						outCVhigh = outCVlow + linkDiff;
					}
                }
            }
			params[PARAM_HIGH_OUT].setValue(outCVhigh);
        } else {
			linkDiff = -1.f;
	        getParamQuantity(PARAM_LOW_OUT)->maxValue = 10.f;
	        getParamQuantity(PARAM_LOW_OUT)->minValue = -10.f;
	        getParamQuantity(PARAM_HIGH_OUT)->maxValue = 10.f;
	        getParamQuantity(PARAM_HIGH_OUT)->minValue = -10.f;
        }

        if (inputs[INPUT_CV].isConnected()) {
			numChannels = std::max(1, inputs[INPUT_CV].getChannels());
			hasCVin = true;
			for (int i=0;i<numChannels;i++) {
				cvIN[i] = rack::math::clamp(inputs[INPUT_CV].getPolyVoltage(i),inCVlow,inCVhigh);
				cvOUT[i] = rack::math::rescale(cvIN[i], inCVlow, inCVhigh, outCVlow, outCVhigh);
				if (isSnapping) {
					if (snapToSemi) {
						cvOUT[i] = snapToSemitone(cvOUT[i]);
                    } else {
						cvOUT[i] = rack::math::clamp(round(cvOUT[i]),-10.f,10.f);
                    }
					if (addSnapOffset) {
						cvOUT[i] += halfVoltPerNote;
                    }
                }
		        outputs[OUTPUT_CV].setVoltage(cvOUT[i],i);
            }
			outputs[OUTPUT_CV].setChannels(numChannels);
		} else {
			numChannels = 0;
			hasCVin = false;
			outputs[OUTPUT_CV].setChannels(0);
        }

        if (isLinked || usingExpanderCVhigh) {
			lights[LIGHT_USEHIGH+0].setBrightness(0.f);
			lights[LIGHT_USEHIGH+1].setBrightness(1.f);
        } else {
			lights[LIGHT_USEHIGH+0].setBrightness(1.f);
			lights[LIGHT_USEHIGH+1].setBrightness(0.f);
        }
        if (usingExpanderCVlow) {
			lights[LIGHT_USELOW+0].setBrightness(0.f);
			lights[LIGHT_USELOW+1].setBrightness(1.f);
        } else {
			lights[LIGHT_USELOW+0].setBrightness(1.f);
			lights[LIGHT_USELOW+1].setBrightness(0.f);
        }
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* val = json_boolean(isLinked);
		json_object_set_new(rootJ, "isLinked", val);
		val = json_real(linkDiff);
		json_object_set_new(rootJ, "linkDiff", val);
		val = json_boolean(isInverted);
		json_object_set_new(rootJ, "isInverted", val);
		val = json_boolean(isSnapping);
		json_object_set_new(rootJ, "isSnapping", val);
		val = json_boolean(snapToSemi);
		json_object_set_new(rootJ, "snapToSemi", val);
		val = json_boolean(addSnapOffset);
		json_object_set_new(rootJ, "addSnapOffset", val);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		// linked?
		json_t* val = json_object_get(rootJ, "isLinked");
		if (val) {
			isLinked = json_boolean_value(val);
		}
		val = json_object_get(rootJ, "linkDiff");
		if (val) {
			linkDiff = json_number_value(val);
        }
		val = json_object_get(rootJ, "isInverted");
		if (val) {
			isInverted = json_boolean_value(val);
        }
		val = json_object_get(rootJ, "isSnapping");
		if (val) {
			isSnapping = json_boolean_value(val);
        }
		val = json_object_get(rootJ, "snapToSemi");
		if (val) {
			snapToSemi = json_boolean_value(val);
        }
		val = json_object_get(rootJ, "addSnapOffset");
		if (val) {
			addSnapOffset = json_boolean_value(val);
        }

		if (isLinked) {
			outCVlow = params[PARAM_LOW_OUT].getValue();
			outCVhigh  = params[PARAM_HIGH_OUT].getValue();

			if (isInverted) {
				// inverted
				getParamQuantity(PARAM_LOW_OUT)->minValue = -10.f+linkDiff;
				getParamQuantity(PARAM_LOW_OUT)->maxValue = 10.f;
				params[PARAM_LOW_OUT].setValue(0.f);
				params[PARAM_HIGH_OUT].setValue(0.f - linkDiff);
			} else {
				getParamQuantity(PARAM_LOW_OUT)->minValue = -10.f;
				getParamQuantity(PARAM_LOW_OUT)->maxValue = 10.f-linkDiff;
				params[PARAM_LOW_OUT].setValue(0.f);
				params[PARAM_HIGH_OUT].setValue(0.f + linkDiff);
			}
        }
	}

	float snapToSemitone(float inCV) {
		return rack::math::clamp(round(inCV * 36.f / 3.f) * 3.f / 36.f,-10.f,10.f);
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
		// invert from context menu only in the right circumstances
		// (expander will handle its own case)

		if (!isLinked && !usingExpanderCVhigh && !usingExpanderCVlow) {
			// reset clamped ranges
			getParamQuantity(PARAM_LOW_OUT)->maxValue = 10.f;
			getParamQuantity(PARAM_LOW_OUT)->minValue = -10.f;
			getParamQuantity(PARAM_HIGH_OUT)->maxValue = 10.f;
			getParamQuantity(PARAM_HIGH_OUT)->minValue = -10.f;

			float tmp = params[PARAM_LOW_OUT].getValue();
			params[PARAM_LOW_OUT].setValue(params[PARAM_HIGH_OUT].getValue());
			params[PARAM_HIGH_OUT].setValue(tmp);

			linkDiff = -1.f; // reset link diff since we've inverted
		}

    }
	void setSnapMode(int mode) {
		switch (mode) {
			case 0:
				// none
				isSnapping = false;
				break;
			case 1:
				// octaves
				isSnapping = true;
				snapToSemi = false;
				break;
			case 2:
				// semitones
				isSnapping = true;
				snapToSemi = true;
				break;
        }
    }
	bool getIsSnapping() {
		return !isSnapping;
    }
	bool getIsSnappingToOctaves() {
		return (isSnapping && !snapToSemi);
    }
	bool getIsSnappingToSemitones() {
		return (isSnapping && snapToSemi);
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

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(22.725, 121.290)), module, Merc8or::LIGHT_EXTENDED));

		addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(27.572, 76.817)), module, Merc8or::LIGHT_USEHIGH));
		addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(27.572, 92.163)), module, Merc8or::LIGHT_USELOW));

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

		menu->addChild(createSubmenuItem("Output Snapping...", "", [=](Menu* menu) {
				menu->addChild(createCheckMenuItem("None", "", [=]() {return module->getIsSnapping();},[=](){module->setSnapMode(0);}));
				menu->addChild(createCheckMenuItem("Octaves", "", [=]() {return module->getIsSnappingToOctaves();},[=](){module->setSnapMode(1);}));
				menu->addChild(createCheckMenuItem("Semitones", "", [=]() {return module->getIsSnappingToSemitones();},[=](){module->setSnapMode(2);}));
				menu->addChild(createBoolPtrMenuItem("Add Half-Semitone Offset", "", &module->addSnapOffset));
			}
		));

	}
};


Model* modelMerc8or = createModel<Merc8or, Merc8orWidget>("Merc8or");
