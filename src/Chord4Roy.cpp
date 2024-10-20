#include "acModules.hpp"
#include "Chord4RoyDisplay.cpp"

struct Chord4Roy : Module {

	enum ParamId {
		PARAM_ROOTNOTE,
		PARAM_CHORD,
		PARAM_USEBAR,
		PARAM_MUTEOROPEN,
		PARAMS_LEN
	};
	enum InputId {
		INPUT_ROOTNOTE,
		INPUT_CHORD,
		INPUT_USEBAR,
		INPUTS_LEN
	};

	enum OutputId {
		OUTPUT_POLYCV,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
    };

	float voltPerNote = 0.08333f;
	float baseOctave = 0.0f;
	// EADGBE
	// 5,10,15,20,24,29 then -1
	float baseOffsets[6] = {4,9,14,19,23,28};
	bool isUsingBar = false;
	bool isUsingMutes = false;
	int curNote = 0;
	int curChord = 0;
	int oldNote = -1;
	int oldChord = -1;

	//         Root, min, 7, Maj7, min7, 6, min6*, Sus* (* = non-bar chords)
	int chordNoteOffsets[12][16][6] = {
		// c
		{ { 3,3,2,0,1,0 }, { -1,3,5,5,4,3}, {-1,3,2,3,1,0}, {-1,3,2,0,0,0}, {-1,-1,1,3,1,3}, {-1,-1,2,2,1,3}, {-1,-1,1,2,1,3}, {-1,-1,3,0,1,3},
		 { 8,10,10,9,8,8 }, { 8,10,10,8,8,8}, {8,10,8,9,8,8}, {8,10,9,9,8,8}, {8,10,8,8,8,8}, {8,10,8,9,10,8}, {-1,-1,1,2,1,3}, {-1,-1,3,0,1,3} }, 
		// c#
		{ { -1,-1,3,1,2,1 }, { -1,-1,2,1,2,0}, {-1,-1,3,4,2,4}, {-1,4,3,1,1,1}, {-1,-1,2,4,2,4}, {-1,-1,3,3,2,4}, {-1,-1,2,3,2,4}, {-1,-1,3,3,4,1},
		 { 9,11,11,10,9,9 }, { 9,11,11,9,9,9}, {9,11,9,10,9,9}, {9,11,10,10,9,9}, {9,11,9,9,9,9}, {9,11,9,10,11,9}, {-1,-1,2,3,2,4}, {-1,-1,3,3,4,1} }, 
		// d
		{ { -1,-1,0,2,3,2 }, { -1,-1,0,2,3,1}, {-1,-1,0,2,1,2}, {-1,-1,0,2,2,2}, {-1,-1,0,2,1,1}, {-1,0,0,2,0,2}, {-1,-1,0,2,0,1}, {-1,-1,0,2,3,3},
		 {10,12,12,11,10,10},{10,12,12,10,10,10},{10,12,10,11,10,10},{10,12,11,11,10,10}, {10,12,10,10,10,10}, {10,12,10,11,12,10}, {-1,-1,0,2,0,1}, {-1,-1,0,2,3,3} },
		// d#
		{ { -1,-1,5,3,4,3 }, { -1,-1,4,3,4,2}, {-1,-1,1,3,2,3}, {-1,-1,1,3,3,3}, {-1,-1,1,3,2,2}, {-1,-1,1,3,1,3}, {-1,-1,1,3,1,2}, {-1,-1,1,3,4,4},
		 {11,13,13,12,11,11},{11,13,13,11,11,11},{11,13,11,12,11,11},{11,13,12,12,11,11}, {11,13,11,11,11,11}, {11,13,11,12,13,11}, {-1,-1,1,3,1,2}, {-1,-1,1,3,4,4} },
		// e
		{ { 0,2,2,1,0,0 }, { 0,2,2,0,0,0}, {0,2,0,1,0,0}, {0,2,1,1,0,0}, {0,2,0,0,0,0}, {0,2,2,1,2,0}, {0,2,2,0,2,0}, {0,2,2,2,0,0},
		 { 4,7,6,4,5,4 }, { 0,2,2,4,5,3}, {0,7,6,7,5,0}, {0,7,6,4,4,4}, {0,2,2,4,3,3}, {7,7,9,9,9,9}, {0,2,2,0,2,0}, {0,2,2,2,0,0} },
		// f
		{ { 1,3,3,2,1,1 }, { 1,3,3,1,1,1}, {1,3,1,2,1,1}, {-1,-1,3,2,1,0}, {1,3,1,1,1,1}, {-1,-1,0,2,1,1}, {-1,-1,0,1,1,1}, {-1,-1,3,3,1,1},
		 { 1,3,3,2,1,1 }, { 1,3,3,1,1,1}, {1,3,1,2,1,1}, {1,3,2,2,1,1}, {1,3,1,1,1,1}, {1,3,1,2,3,1}, {-1,-1,0,1,1,1}, {-1,-1,3,3,1,1} },
		// f#
		{ { 2,4,4,3,2,2 }, { 2,4,4,2,2,2}, {-1,-1,4,3,2,0}, {-1,-1,4,3,2,1}, {-1,-1,2,2,2,2}, {-1,4,4,3,4,-1}, {-1,-1,1,2,2,2}, {-1,-1,4,4,2,2},
		 { 2,4,4,3,2,2 }, { 2,4,4,2,2,2}, {2,4,2,3,2,2}, {2,4,3,3,2,2}, {2,4,2,2,2,2}, {2,4,2,3,4,2}, {-1,-1,1,2,2,2}, {-1,-1,4,4,2,2} },
		// g
		{ { 3,2,0,0,0,3 }, { 3,5,5,3,3,3}, {3,2,0,0,0,1}, {-1,-1,5,4,3,2}, {3,5,3,3,3,3}, {3,2,0,0,0,0}, {-1,-1,2,3,3,3}, {-1,-1,0,0,1,3},
		 { 3,5,5,4,3,3 }, { 3,5,5,3,3,3}, {3,5,3,4,3,3}, {3,5,4,4,3,3}, {3,5,3,3,3,3}, {3,5,3,4,5,3}, {-1,-1,2,3,3,3}, {-1,-1,0,0,1,3} },
		// g#
		{ { 4,6,6,5,4,4 }, { 4,6,6,4,4,4}, {-1,-1,1,1,1,2}, {-1,-1,1,1,1,3}, {-1,-1,1,1,0,2}, {4,3,1,1,1,1}, {-1,-1,-1,4,4,4}, {-1,-1,1,1,2,4},
		 { 4,6,6,5,4,4 }, { 4,6,6,4,4,4}, {4,6,4,5,4,4}, {4,6,5,5,4,4}, {4,6,4,4,4,4}, {4,6,4,5,6,4}, {-1,-1,-1,4,4,4}, {-1,-1,1,1,2,4} },
		// a
		{ { -1,0,2,2,2,0 }, { -1,0,2,2,1,0}, {-1,0,2,2,2,3}, {-1,0,2,1,2,0}, {-1,0,2,0,1,0}, {-1,0,2,2,2,2}, {-1,0,2,2,1,2}, {-1,0,2,2,3,0},
		 { 5,7,7,6,5,5 }, { 5,7,7,5,5,5}, {5,7,5,6,5,5}, {-1,5,7,6,7,5}, {5,7,5,5,5,5}, {5,7,5,6,7,5}, {-1,0,2,2,1,2}, {-1,0,2,2,3,0} },
		// a#
		{ { -1,1,3,3,3,1 }, { -1,1,3,3,2,1}, {-1,1,3,3,3,4}, {-1,1,3,2,3,1}, {-1,-1,3,3,2,4}, {1,1,3,3,3,3}, {-1,-1,3,3,2,3}, {-1,-1,3,3,4,1},
		 { 6,8,8,7,6,6 }, { 6,8,8,6,6,6}, {6,8,6,7,6,6}, {-1,6,8,7,8,6}, {6,8,6,6,6,6}, {6,8,6,7,8,6}, {-1,-1,3,3,2,3}, {-1,-1,3,3,4,1} },
		// b
		{ { -1,2,4,4,4,2 }, {-1,2,4,4,3,2}, {-1,2,1,2,0,2}, {-1,2,4,3,4,-1}, {-1,2,4,2,3,2}, {2,2,4,4,4,4}, {-1,-1,4,4,3,4}, {-1,-1,4,4,5,2},
		 { 7,9,9,8,7,7 }, { 7,9,9,7,7,7}, {7,9,7,8,7,7}, {7,9,8,8,7,7}, {7,9,7,7,7,7}, {7,9,7,8,9,7}, {-1,-1,4,4,3,4}, {-1,-1,4,4,5,2} },
	};

	Chord4Roy() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, 0);
		configParam(PARAM_ROOTNOTE, 1.f, 12.f, 1.f, "Root Note");
		configParam(PARAM_CHORD, 1.f, 8.f, 1.f, "Chord");
		configSwitch(PARAM_USEBAR, 0.f, 1.f, 1.f, "Use Bar Chord Style", {"Bar", "Open"});
		configSwitch(PARAM_MUTEOROPEN, 0.f, 1.f, 1.f, "Mute or Open Strings", {"Open Strings", "Muted"});
		configInput(INPUT_ROOTNOTE, "Root Note CV 0-10v");
		configInput(INPUT_CHORD, "Chord Type CV 0-10v");
		configInput(INPUT_USEBAR, "Open or Bar CV 0-10v");
		configOutput(OUTPUT_POLYCV, "Chord String Notes CV (POLY)");

		paramQuantities[PARAM_ROOTNOTE]->snapEnabled = true;
		paramQuantities[PARAM_CHORD]->snapEnabled = true;

	}

	void process(const ProcessArgs& args) override {

		curNote = static_cast<int>(params[PARAM_ROOTNOTE].getValue());
		curChord = static_cast<int>(params[PARAM_CHORD].getValue());

		isUsingBar = params[PARAM_USEBAR].getValue() < 0.5f ? true : false;
		isUsingMutes=params[PARAM_MUTEOROPEN].getValue() < 0.5f ? false : true;

        if (inputs[INPUT_ROOTNOTE].isConnected()) {
			curNote = static_cast<int>(clamp(rescale(inputs[INPUT_ROOTNOTE].getVoltage(),0.f,10.f,1.f,12.f),1.f,12.f));
		}
        if (inputs[INPUT_CHORD].isConnected()) {
			curChord = static_cast<int>(clamp(rescale(inputs[INPUT_CHORD].getVoltage(),0.f,10.f,1.f,8.f),1.f,8.f));
		}
        if (inputs[INPUT_USEBAR].isConnected()) {
			isUsingBar = inputs[INPUT_USEBAR].getVoltage() < 5.f ? false : true;
			params[PARAM_USEBAR].setValue(isUsingBar ? 0.0f : 1.0f);
		}

		int chordIndex = (curChord-1) + (isUsingBar ? 8 : 0);

		// set current chord notes
		for (int i=0;i<6;i++) {
			if (chordNoteOffsets[curNote-1][chordIndex][i] >= 0) {
				// not muted
				outputs[OUTPUT_POLYCV].setVoltage(baseOctave + ((baseOffsets[i] + chordNoteOffsets[curNote-1][chordIndex][i]) * voltPerNote),i);
				outputs[OUTPUT_POLYCV].setVoltage(0.1f,i+6);
			} else {
				// muted
				outputs[OUTPUT_POLYCV].setVoltage(baseOctave + (baseOffsets[i] * voltPerNote),i);
				if (isUsingMutes) {
					// mute the string
					outputs[OUTPUT_POLYCV].setVoltage(10.f,i+6);
                } else {
					// open string note
					outputs[OUTPUT_POLYCV].setVoltage(0.f,i+6);
                }
            }
		}
		// add root note on channel 13
		outputs[OUTPUT_POLYCV].setVoltage(baseOctave + ((curNote-1) * voltPerNote),12); // note 12 is 13 (0-based)
		outputs[OUTPUT_POLYCV].setChannels(13);
	}

};


struct Chord4RoyWidget : ModuleWidget {
	Chord4RoyWidget(Chord4Roy* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Chord4Roy-White.svg"), asset::plugin(pluginInstance, "res/Chord4Roy-Dark.svg")));

		Chord4RoyDisplay<Chord4Roy>* display = createWidget<Chord4RoyDisplay<Chord4Roy>>(mm2px(Vec(4.24, 7.319)));
		display->box.size = mm2px(Vec(22.0, 13.0));
		display->displaySize = Vec(22.0, 13.0);
		display->module = module;
		addChild(display);

		addChild(createWidget<ThemedScrew>(Vec(2, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 16, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		// root note
		addParam(createParamCentered<Trimpot>(mm2px(Vec(20.532, 36.134)), module, Chord4Roy::PARAM_ROOTNOTE));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(9.948, 36.134)), module, Chord4Roy::INPUT_ROOTNOTE));

		// chord
		addParam(createParamCentered<Trimpot>(mm2px(Vec(20.532, 52.009)), module, Chord4Roy::PARAM_CHORD));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(9.948, 52.009)), module, Chord4Roy::INPUT_CHORD));

		// bar
		addParam(createParamCentered<CKSS>(mm2px(Vec(6.354, 68.257)), module, Chord4Roy::PARAM_USEBAR));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(6.354, 76.954)), module, Chord4Roy::INPUT_USEBAR));

		// mutes
		addParam(createParamCentered<CKSS>(mm2px(Vec(20.642, 68.257)), module, Chord4Roy::PARAM_MUTEOROPEN));

		// outputs
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(15.240, 104.412)), module, Chord4Roy::OUTPUT_POLYCV));
	}


};


Model* modelChord4Roy = createModel<Chord4Roy, Chord4RoyWidget>("Chord4Roy");
