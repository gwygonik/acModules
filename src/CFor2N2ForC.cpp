#include "acModules.hpp"

struct CFor2N2ForC : Module {
	enum ParamId {
		ENUMS(KNOB_NOTECV, 12),
		PARAMS_LEN
	};
	enum InputId {
		INPUT_CV,
		INPUT_TRIGGER,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUT_CV,
		OUTPUT_TRIGGER,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(LED_NOTE, 12),
		LIGHTS_LEN
	};

	bool hasCV = false;
	float inCV = 0.f;
	bool triggered = false;
	int delayCounter = 0;
	bool delayBeforePlay = true;
	bool hasPulsed[12];
	int curNote = 0;
	int oldNote = 0;
	float voltPerNote = 0.08333f;
	bool outputTrigOnNoteChange = false;

	dsp::SchmittTrigger inTrigger;
	dsp::PulseGenerator outTrigger[12];

	const char* noteNameLabels[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

	CFor2N2ForC() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		for (int i = 0; i < 12; i++) {
			configParam(KNOB_NOTECV + i, -5.f, 10.f, 0.f, string::f("Note (%s) CV", noteNameLabels[i]));
		}

		configInput(INPUT_CV, "CV");
		configInput(INPUT_TRIGGER, "Trigger");
		configOutput(OUTPUT_CV, "CV");
		configOutput(OUTPUT_TRIGGER, "Trigger");
	}

	void process(const ProcessArgs& args) override {
        //pulseOnCount = params[OUTON_PARAM].getValue();

		hasCV = inputs[INPUT_CV].isConnected();

		if (hasCV) {
			double tmp;
			float tmpV = clamp(inputs[INPUT_CV].getVoltage(),-5.f,10.f);
			if (tmpV < 0.f) tmpV += 10.0f; // push to between 0 and 10
			curNote = static_cast<int>(clamp((modf(tmpV,&tmp)/voltPerNote)+1.f,1.f,12.f));
			if (curNote != oldNote) {
				// new note!
				if (outputTrigOnNoteChange) {
					triggered = true;
                }
				oldNote = curNote;
            }

	        inCV = params[KNOB_NOTECV+(curNote-1)].getValue();

			if (inTrigger.process(inputs[INPUT_TRIGGER].getVoltage(), 0.01f, 2.f)) {
				triggered = true;
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
					outTrigger[curNote-1].trigger(1e-3f);
					triggered = false;
				}
			}
        } else {
			inCV = 0.f;
        }


		outputs[OUTPUT_CV].setVoltage(inCV);

		for (int i=0;i<12;i++) {
			hasPulsed[i] = outTrigger[i].process(args.sampleTime);
			outputs[OUTPUT_TRIGGER].setVoltage(hasPulsed[i] ? 10.f : 0.f,i);
			lights[LED_NOTE+i].setBrightness(((hasCV) && (curNote == i+1)) ? 1.f : 0.f);
        }

		outputs[OUTPUT_TRIGGER].setChannels(12);
        
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_t* val = json_boolean(outputTrigOnNoteChange);
		json_object_set_new(rootJ, "outputTrigOnNoteChange", val);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		// trig on note change?
		json_t* val = json_object_get(rootJ, "outputTrigOnNoteChange");
		if (val) {
			outputTrigOnNoteChange = json_boolean_value(val);
		}
	}

};


struct CFor2N2ForCWidget : ModuleWidget {
	CFor2N2ForCWidget(CFor2N2ForC* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/CFor2N2ForC-White.svg"), asset::plugin(pluginInstance, "res/CFor2N2ForC-Dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(2, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 16, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// input
		// cv
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(10.857, 23.440)), module, CFor2N2ForC::INPUT_CV));
		// trig
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(10.857, 37.219)), module, CFor2N2ForC::INPUT_TRIGGER));

		// output
		// cv
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(10.857, 93.2)), module, CFor2N2ForC::OUTPUT_CV));
		// trig
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(10.857, 106.98)), module, CFor2N2ForC::OUTPUT_TRIGGER));


		// notes
		for (int i=0;i<12;i++) {
			addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(45.374, 13.413+(i*8.692))), module, CFor2N2ForC::LED_NOTE + i));
			addParam(createParamCentered<Trimpot>(mm2px(Vec(36.965, 13.546+(i*8.692))), module, CFor2N2ForC::KNOB_NOTECV+i));
		}

		// There's always the sun

	}

	void appendContextMenu(Menu* menu) override {
		CFor2N2ForC* module = getModule<CFor2N2ForC>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("CFor2N2ForC Options"));
		menu->addChild(createBoolPtrMenuItem("Output Trigger On Note Change", "", &module->outputTrigOnNoteChange));
	}
};


Model* modelCFor2N2ForC = createModel<CFor2N2ForC, CFor2N2ForCWidget>("CFor2N2ForC");
