#include "acModules.hpp"

struct MOREc8or : Module {

	struct MOREc8orMessage {
		bool invert;
		bool linkHL;
		bool hasCVHigh;
		float cvHigh;
		bool hasCVLow;
		float cvLow;
	};

	enum ParamId {
		INVERT_PARAM,
		INVERT_TYPE_PARAM,
		LINK_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUT_INVERT,
		INPUT_HIGH,
		INPUT_LOW,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUT_CV,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHT_EXTENDED,
		ENUMS(INVERT_LIGHT,3),
		ENUMS(LIGHT_CANINVERT,2),
		ENUMS(LIGHT_CANLINK,2),
		LINK_LIGHT,
		LIGHTS_LEN
	};

	MOREc8orMessage leftMessages[2][1]; // messages from left module (Merc8or module))
	bool isConnectedToMerc8 = false;
	bool canLink = true;
	bool isHLLinked = false;
	bool canInvert = true;
	bool isInvert = false;
	bool hasCVHIGH = false;
	bool hasCVLOW = false;
	float inCVHIGH = 0.f;
	float inCVLOW = 0.f;
	float linkDiff = 0.f;
	bool isUsingGate = false;
	bool gateState = false;
	bool oldGateState = false;
	dsp::SchmittTrigger trigInvert;
	dsp::BooleanTrigger invertTrigger;
	dsp::BooleanTrigger linkTrigger;

	MOREc8or() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configSwitch(INVERT_PARAM, 0.f,1.f,0.f, "Invert Output Range");
		configSwitch(INVERT_TYPE_PARAM, 0.f, 1.f, 0.f, "Invert CV Type", {"Trigger", "Gate"});
		configSwitch(LINK_PARAM, 0.f,1.f,0.f, "Link High and Low");
		configInput(INPUT_INVERT, "Invert Switch");
		configInput(INPUT_HIGH, "Output Range High CV");
		configInput(INPUT_LOW, "Output Range Low CV");

		// set the left expander message instances
		leftExpander.producerMessage = leftMessages[0];
		leftExpander.consumerMessage = leftMessages[1];
	}

	void process(const ProcessArgs& args) override {
		canInvert = true;
		canLink = true;


		if (inputs[INPUT_HIGH].isConnected()) {
			hasCVHIGH = true;
			inCVHIGH = rack::math::clamp(inputs[INPUT_HIGH].getVoltage(),-10.f,10.f);
			canInvert = false;
        } else {
			hasCVHIGH = false;
        }
		if (inputs[INPUT_LOW].isConnected()) {
			hasCVLOW = true;
			inCVLOW = rack::math::clamp(inputs[INPUT_LOW].getVoltage(),-10.f,10.f);
			canInvert = false;
        } else {
			hasCVLOW = false;
        }

		if (hasCVLOW && hasCVHIGH) {
			canLink = false;
			isHLLinked = false;
        }

		if (canLink && linkTrigger.process(params[LINK_PARAM].getValue() > 0.f)) {
			isHLLinked ^= true;
			params[LINK_PARAM].setValue(isHLLinked ? 1.f : 0.f);
		}
		lights[LINK_LIGHT].setBrightness(canLink && isHLLinked);

		if (isHLLinked) {
			hasCVHIGH = false; // disable high CV -- everything linked is driven by low
			canInvert = false;
        }

		if (canInvert && invertTrigger.process(params[INVERT_PARAM].getValue() > 0.f)) {
			isInvert = true;
			params[INVERT_PARAM].setValue(isInvert ? 1.f : 0.f);
		}
		if (params[INVERT_TYPE_PARAM].getValue() > 0.5f) {
			isUsingGate = true;
        } else {
			isUsingGate = false;
        }

		if (inputs[INPUT_INVERT].isConnected() && canInvert) {
			if (isUsingGate) {
				gateState = inputs[INPUT_INVERT].getVoltage() > 0.f;
				if (gateState != oldGateState) {
					isInvert = true;
					oldGateState = gateState;
                } else {
					isInvert = false;
                }
            } else {
				if (trigInvert.process(inputs[INPUT_INVERT].getVoltage(), 0.01f, 2.f)) {
					isInvert = true;
				} else {
					isInvert = false;
                }
            }
        }



		if (leftExpander.module) {
			if (leftExpander.module->model == modelMerc8or) {
				isConnectedToMerc8 = true;
				// turn on connected LED
				lights[LIGHT_EXTENDED].setBrightness(0.95f);
				// send data to module
				MOREc8orMessage *messageToModule = (MOREc8orMessage *)(leftExpander.producerMessage);
				messageToModule->invert = isInvert;
				messageToModule->linkHL = isHLLinked;
				messageToModule->hasCVHigh = hasCVHIGH;
				messageToModule->cvHigh = inCVHIGH;
				messageToModule->hasCVLow = hasCVLOW;
				messageToModule->cvLow = inCVLOW;
				// flip the script
				leftExpander.messageFlipRequested = true;

			}
		} else {
			isConnectedToMerc8 = false;
			// turn off connected LED
			lights[LIGHT_EXTENDED].setBrightness(0.f);
        }

		lights[LIGHT_CANINVERT+0].setBrightness(canInvert);
		lights[LIGHT_CANINVERT+1].setBrightness(!canInvert);

		lights[LIGHT_CANLINK+0].setBrightness(canLink);
		lights[LIGHT_CANLINK+1].setBrightness(!canLink);


		// reset
		isInvert = false;
        
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* val = json_boolean(isHLLinked);
		json_object_set_new(rootJ, "isHLLinked", val);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		// linked?
		json_t* val = json_object_get(rootJ, "isHLLinked");
		if (val) {
			isHLLinked = json_boolean_value(val);
		}
	}

};


struct MOREc8orWidget : ModuleWidget {
	MOREc8orWidget(MOREc8or* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/MOREc8or-White.svg"), asset::plugin(pluginInstance, "res/MOREc8or-Dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(2, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 16, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<VCVButton>(mm2px(Vec(7.62, 15.319)), module, MOREc8or::INVERT_PARAM));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 25.196)), module, MOREc8or::INPUT_INVERT));
		addParam(createParamCentered<CKSS>(mm2px(Vec(4.924, 35.471)), module, MOREc8or::INVERT_TYPE_PARAM));

		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(7.62, 56.738)), module, MOREc8or::LINK_PARAM, MOREc8or::LINK_LIGHT));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 76.772)), module, MOREc8or::INPUT_HIGH));
		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(7.62, 92.037)), module, MOREc8or::INPUT_LOW));

		addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(12.366, 8.589)), module, MOREc8or::LIGHT_CANINVERT));
		addChild(createLightCentered<SmallLight<GreenRedLight>>(mm2px(Vec(10.9, 50.643)), module, MOREc8or::LIGHT_CANLINK));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.355, 121.290)), module, MOREc8or::LIGHT_EXTENDED));
	}

};


Model* modelMOREc8or = createModel<MOREc8or, MOREc8orWidget>("MOREc8or");
