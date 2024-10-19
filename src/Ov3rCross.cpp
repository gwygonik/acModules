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
    float targetOut1, targetOut2, targetOut3;
    float currentOut1, currentOut2, currentOut3;
    dsp::SchmittTrigger inTrigger;
    dsp::PulseGenerator pulseOutputs[3];
    

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
        float cvIN   = rack::math::clamp(inputs[INPUT_CV].getVoltage(),-5.f,10.f);
        cvVal = cvIN;
        short curOut = 0;

        // UI blocks
        getParamQuantity(PARAM_CUTOFF_HIGH)->minValue = lowCut+0.05f;
        getParamQuantity(PARAM_CUTOFF_LOW)->maxValue = highCut-0.05f;

        if (cvIN <= lowCut) {
            lights[LIGHT_LOW].setBrightness(1.f);
            lights[LIGHT_MID].setBrightness(0.f);
            lights[LIGHT_HIGH].setBrightness(0.f);
            targetOut1 = 0.0f;
            targetOut2 = 0.0f;
            targetOut3 = 1.0f;
            curOut = 3;
        } else {
            if (cvIN > lowCut && cvIN < highCut) {
                lights[LIGHT_LOW].setBrightness(0.f);
                lights[LIGHT_MID].setBrightness(1.f);
                lights[LIGHT_HIGH].setBrightness(0.f);
                targetOut1 = 0.0f;
                targetOut2 = 1.0f;
                targetOut3 = 0.0f;
                curOut = 2;
            } else {
                if (cvIN >= highCut) {
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
        
        if (inTrigger.process(rack::math::rescale(inputs[INPUT_TRIGGER].getVoltage(), 0.1f, 2.f, 0.f, 1.f))) {
            pulseOutputs[0].reset();
            pulseOutputs[1].reset();
            pulseOutputs[2].reset();
            pulseOutputs[curOut-1].trigger(1e-3f);
        }

        if (inputs[INPUT_CVTHRU].isConnected()) {
            sendCVThru = true;
            cvThru = inputs[INPUT_CVTHRU].getVoltage();
        } else {
            sendCVThru = false;
        }

        currentOut3 += (targetOut3 - currentOut3) / 5.0f;
        currentOut2 += (targetOut2 - currentOut2) / 5.0f;
        currentOut1 += (targetOut1 - currentOut1) / 5.0f;
        if (abs(currentOut3 - targetOut3) < 0.05f) currentOut3 = targetOut3;
        if (abs(currentOut2 - targetOut2) < 0.05f) currentOut2 = targetOut2;
        if (abs(currentOut1 - targetOut1) < 0.05f) currentOut1 = targetOut1;

        bool pulse = false;
        float outVoltage = sendCVThru ? cvThru : cvIN;

        switch (curOut) {
            case 1:
                outputs[OUTPUT_CV_HIGH].setVoltage(outVoltage * currentOut1);
                outputs[OUTPUT_CV_MID].setVoltage(outVoltage * currentOut2);
                outputs[OUTPUT_CV_LOW].setVoltage(outVoltage * currentOut3);
                pulse = pulseOutputs[0].process(args.sampleTime);
                outputs[OUTPUT_TRIG_HIGH].setVoltage(pulse ? 10.f : 0.f);
                outputs[OUTPUT_TRIG_MID].setVoltage(0.f);
                outputs[OUTPUT_TRIG_LOW].setVoltage(0.f);
                break;
            case 2:
                outputs[OUTPUT_CV_HIGH].setVoltage(outVoltage * currentOut1);
                outputs[OUTPUT_CV_MID].setVoltage(outVoltage * currentOut2);
                outputs[OUTPUT_CV_LOW].setVoltage(outVoltage * currentOut3);
                pulse = pulseOutputs[1].process(args.sampleTime);
                outputs[OUTPUT_TRIG_HIGH].setVoltage(0.f);
                outputs[OUTPUT_TRIG_MID].setVoltage(pulse ? 10.f : 0.f);
                outputs[OUTPUT_TRIG_LOW].setVoltage(0.f);
                break;
            case 3:
                outputs[OUTPUT_CV_HIGH].setVoltage(outVoltage * currentOut1);
                outputs[OUTPUT_CV_MID].setVoltage(outVoltage * currentOut2);
                outputs[OUTPUT_CV_LOW].setVoltage(outVoltage * currentOut3);
                pulse = pulseOutputs[2].process(args.sampleTime);
                outputs[OUTPUT_TRIG_HIGH].setVoltage(0.f);
                outputs[OUTPUT_TRIG_MID].setVoltage(0.f);
                outputs[OUTPUT_TRIG_LOW].setVoltage(pulse ? 10.f : 0.f);
                break;
            default:
                break;
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
};


Model* modelOv3rCross = createModel<Ov3rCross, Ov3rCrossWidget>("Ov3rCross");
