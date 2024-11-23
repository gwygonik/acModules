#include "acModules.hpp"
#include <math.hpp>

struct CRBVi : Module {
	enum ParamId {
		PARAM_BASEOCT,
		PARAM_OCTAVES,
		PARAM_INPUTCURVE,
		PARAM_SNAP,
		PARAMS_LEN
	};
	enum InputId {
		INPUT_VCA,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUT_X,
		OUTPUT_Y,
		OUTPUT_GATE,
		OUTPUT_VCA,
		OUTPUT_POLY,
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHT_SNAP,
		LIGHTS_LEN
	};

	float curSampleRate = 0.f;
	float padX, padY;
	bool isDragging = false;
	int baseOctave = 0;
	int numOctaves = 1;
	int curKey = 0;
	float curVolt = 0.f;
	float rawVolt = 0.f;
	float voltPerNote = 0.08333f;
	float halfVoltPerNote = 0.041665f;
	bool isSnapped = false;
	int currentInputCurve = 4;
	bool showKeys = true;
	float curX, curY;
	bool hasExtIn = false;
	dsp::BooleanTrigger snapTrigger;

	CRBVi() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PARAM_BASEOCT, -5.f, 4.f, 0.f, "Base Octave");
		configParam(PARAM_OCTAVES, 1.f, 3.f, 1.f, "Octaves");
		configParam(PARAM_INPUTCURVE, 0.f, 4.f, 0.f, "Input Curve");
		configSwitch(PARAM_SNAP, 0.f,1.f,0.f, "Snap To Notes");
		configOutput(OUTPUT_X, "Note (X)");
		configOutput(OUTPUT_Y, "Volume (Y)");
		configOutput(OUTPUT_GATE, "Gate");
		configInput(INPUT_VCA, "External [POLY]");
		configOutput(OUTPUT_VCA, "VCA [POLY]");
		configOutput(OUTPUT_POLY, "Raw [POLY]");

		paramQuantities[PARAM_BASEOCT]->snapEnabled = true;
		paramQuantities[PARAM_OCTAVES]->snapEnabled = true;
		paramQuantities[PARAM_INPUTCURVE]->snapEnabled = true;

	}


	void process(const ProcessArgs& args) override {
		curSampleRate = args.sampleRate;

		baseOctave = (int)params[PARAM_BASEOCT].getValue();
        numOctaves = (int)params[PARAM_OCTAVES].getValue();
		currentInputCurve = (int)params[PARAM_INPUTCURVE].getValue();

		hasExtIn = inputs[INPUT_VCA].isConnected();

		if (snapTrigger.process(params[PARAM_SNAP].getValue() > 0.f)) {
			isSnapped ^= true;
        }

		if (!isDragging) {
			padY = 0.f;
        }

		if (!isNear(curX, padX)) {
			curX += (padX - curX)/100.f;
        } else {
			curX = padX;
        }
		if (!isNear(curY, padY)) {
			curY += (padY - curY)/500.f;
        } else {
			curY = padY;
        }

		if (isSnapped) {
			curVolt = (float)baseOctave + (float)curKey * voltPerNote + halfVoltPerNote;
			rawVolt = (float)baseOctave + (float)curKey * voltPerNote;
        } else {
			curVolt = rescale(curX,0.f,1.f,(float)baseOctave - halfVoltPerNote,(float)baseOctave + (float)numOctaves + voltPerNote - halfVoltPerNote);			
			rawVolt = rescale(curX,0.f,1.f,(float)baseOctave,(float)baseOctave + (float)numOctaves + voltPerNote);			
        }

		outputs[OUTPUT_X].setVoltage( curVolt );			
		outputs[OUTPUT_Y].setVoltage( getVoltageCurve() );
		outputs[OUTPUT_GATE].setVoltage( isDragging ? 10.f : 0.f);

		if (hasExtIn) {
			for (int i=0;i<inputs[INPUT_VCA].getChannels();i++) {
				outputs[OUTPUT_VCA].setVoltage(isDragging ? getVCAVoltageCurve(inputs[INPUT_VCA].getPolyVoltage(i)) : 0.f, i);
            }
			outputs[OUTPUT_VCA].setChannels(inputs[INPUT_VCA].getChannels());
        } else {
			outputs[OUTPUT_VCA].setVoltage(0.f,0);
			outputs[OUTPUT_VCA].setChannels(1);
        }
		
		outputs[OUTPUT_POLY].setVoltage( curVolt, 0 );				  // note (X) adjusted for screen space
		outputs[OUTPUT_POLY].setVoltage( getVoltageCurve(), 1 );	  // volume (Y with applied input curve)
		outputs[OUTPUT_POLY].setVoltage( isDragging ? 10.f : 0.f, 2 );// gate (on)
		outputs[OUTPUT_POLY].setVoltage( padX, 3 );					  // raw pad X position
		outputs[OUTPUT_POLY].setVoltage( padY, 4 );					  // raw pad Y position
		outputs[OUTPUT_POLY].setVoltage( rawVolt, 5 );				  // note (X) without screen adjustment
		outputs[OUTPUT_POLY].setChannels(6);

		lights[LIGHT_SNAP].setBrightness(isSnapped ? 0.95f : 0.f);


	}

	float getVoltageCurve() {
		float outVolt = curY/10.f;
		switch (currentInputCurve) {
		  case 1:
			outVolt = 1.f - sqrtf(1.f - (outVolt*outVolt));// powf(outVolt, 2.f));
			break;
		  case 2:
			outVolt = -(cosf(M_PI * outVolt)-1.f)/2.f;
			break;
		  case 3:
			outVolt = sinf((outVolt * M_PI)/2.f);
			break;
		  case 4:
			outVolt = (outVolt > 0.01f ? 1.f : 0.f);
			break;
		}
		return outVolt * 10.f;
    }
	float getVCAVoltageCurve(float inVoltage) {
		float outVolt = curY/10.f;
		switch (currentInputCurve) {
		  case 1:
			outVolt = 1.f - sqrtf(1.f - powf(outVolt, 2.f));
			break;
		  case 2:
			outVolt = -(cosf(M_PI * outVolt)-1.f)/2.f;
			break;
		  case 3:
			outVolt = sinf((outVolt * M_PI)/2.f);
			break;
		  case 4:
			outVolt = (outVolt > 0.01f ? 1.f : 0.f);
			break;
		}
		return outVolt * inVoltage;
    }

	void setPadInputs(float mX, float mY, int key) {
		padX = mX;
		padY = mY;
		curKey = key;
    }

	float snapToSemitone(float inCV) {
		return rack::math::clamp(round(inCV * 36.f / 3.f) * 3.f / 36.f,-10.f,10.f);
    }

	json_t* dataToJson() override {
		json_t* rootJ = json_object();

		json_t* val = json_boolean(isSnapped);
		json_object_set_new(rootJ, "isSnapped", val);
		val = json_boolean(showKeys);
		json_object_set_new(rootJ, "showKeys", val);

		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {
		// snapped?
		json_t* val = json_object_get(rootJ, "isSnapped");
		if (val) {
			isSnapped = json_boolean_value(val);
		}
		// showing keys
		val = json_object_get(rootJ, "showKeys");
		if (val) {
			showKeys = json_boolean_value(val);
        }
	}

};



struct acTouchRibbon : rack::OpaqueWidget {
	//std::string fontPath = rack::asset::system("res/fonts/ShareTechMono-Regular.ttf");

	CRBVi* module;

    acTouchRibbon() : OpaqueWidget() { }

    virtual ~acTouchRibbon() { }

	float padX;
	float padY;
	double curKey;
	double frac; // this is not really used, but necessary for modf
	bool isDragging = false;
	int keyColors[12] = {2,0,1,0,1,1,0,1,0,1,0,1};

	void setModule(CRBVi* module) {
		this->module = module;
    }

	void step() override {
		if (module) {
			frac = modf(double((padX/box.size.x)*(float)(module->numOctaves*12+1)),&curKey);
			module->setPadInputs(padX/box.size.x, 10.0f-clamp((padY/(box.size.y-22.f))*10.1f,0.f,10.f), curKey);
        }
	}

	void drawLayer(const DrawArgs& args, int layer) override {

		auto vg = args.vg;

		nvgBeginPath(vg);
		nvgFillColor(vg, nvgRGB(0x20,0x20,0x20));
		nvgRect(vg, rack::mm2px(0.f), rack::mm2px(0.f), (box.size.x), (box.size.y));
		nvgFill(vg);

		if (layer == 1 && module) {

			if (module->curSampleRate == 0.f) return;

			nvgScissor(vg, RECT_ARGS(args.clipBox));

			if (module->showKeys) {

				nvgStrokeColor(vg, nvgRGBA(0xff,0xff,0xff,0x40));
				nvgStrokeWidth(vg, 2);
				int keys = module->numOctaves*12+1;
				float xinc = box.size.x/(float)keys;
				for (int i=0;i<keys;i++) {
					int keyCol = keyColors[i%12];
					nvgBeginPath(vg);
					if (keyCol == 1) {
						// white key
						nvgFillColor(vg, nvgRGBA(0xe6,0xe6,0xe6,0xa0));
					} else if (keyCol == 2) {
						// C key
						nvgFillColor(vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
					} else {
						// black key
						nvgFillColor(vg, nvgRGBA(0xd5,0xd5,0xd5,0x73));
					}
					nvgRect(vg, xinc*i+1.f,1.f,xinc-1.5f,box.size.y-25.f);
					nvgFill(vg);
				}
			
				nvgStrokeColor(vg, nvgRGBA(0x00,0x00,0x00,0x40));
				for (int i=1;i<keys;i++) {
					nvgBeginPath(vg);
					nvgMoveTo(vg, (xinc*i), box.size.y-25);
					nvgLineTo(vg, (xinc*i), 0.f);
					nvgStroke(vg);
				}
			}			

			if (isDragging) {
				nvgBeginPath(vg);
				
				nvgMoveTo(vg, padX, box.size.y-5);
				nvgLineTo(vg, padX, padY + 22.f);
				nvgStrokeColor(vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
				nvgStrokeWidth(vg, 2);
				nvgStroke(vg);
				
				nvgBeginPath(vg);
				nvgCircle(vg, padX, padY, 20.f);
				nvgFillColor(vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
				nvgFill(vg);
			}

		} else {
			// no module (probably in library)
			nvgStrokeColor(vg, nvgRGBA(0xff,0xff,0xff,0x40));
			nvgStrokeWidth(vg, 2);
			int keys = 13;
			float xinc = box.size.x/(float)keys;
			for (int i=0;i<keys;i++) {
				int keyCol = keyColors[i%12];
				nvgBeginPath(vg);
				if (keyCol == 1) {
					// white key
					nvgFillColor(vg, nvgRGBA(0xe6,0xe6,0xe6,0xa0));
				} else if (keyCol == 2) {
					// C key
					nvgFillColor(vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
				} else {
					// black key
					nvgFillColor(vg, nvgRGBA(0xd5,0xd5,0xd5,0x73));
				}
				nvgRect(vg, xinc*i+1.f,1.f,xinc-1.5f,box.size.y-25.f);
				nvgFill(vg);
			}
			
			nvgStrokeColor(vg, nvgRGBA(0x00,0x00,0x00,0x40));
			for (int i=1;i<keys;i++) {
				nvgBeginPath(vg);
				nvgMoveTo(vg, (xinc*i), box.size.y-25);
				nvgLineTo(vg, (xinc*i), 0.f);
				nvgStroke(vg);
			}

			nvgBeginPath(vg);
			nvgMoveTo(vg, box.size.x*.33f, box.size.y-5);
			nvgLineTo(vg, box.size.x*.33f, box.size.y*.45f + 22.f);
			nvgStrokeColor(vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
			nvgStrokeWidth(vg, 2);
			nvgStroke(vg);
				
			nvgBeginPath(vg);
			nvgCircle(vg, box.size.x*.33f, box.size.y*.45f, 20.f);
			nvgFillColor(vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
			nvgFill(vg);

        }

		// finally the rest
		nvgBeginPath(vg);
		nvgStrokeColor(vg, nvgRGB(0xff,0xcc,0xaa));
		nvgMoveTo(vg, 0, box.size.y-24);
		nvgLineTo(vg, box.size.x, box.size.y-24);
		nvgStrokeWidth(vg, 1);
		nvgStroke(vg);

		nvgResetScissor(args.vg);
		Widget::drawLayer(args, layer);
	}


	void onDragStart(const DragStartEvent &e) override {
		module->isDragging = true;
		isDragging = true;
		e.consume(this);
    }
	void onDragEnd(const DragEndEvent &e) override {
		module->isDragging = false;
		isDragging = false;
		e.consume(this);
    }
	void onDragHover(const DragHoverEvent &e) override {
		if (isDragging) {
			padX = e.pos.x;
			padY = e.pos.y;
        }
		e.consume(this);
    }

    void onButton(const ButtonEvent& e) override
    {
		padX = e.pos.x;
		padY = e.pos.y;
     	OpaqueWidget::onButton(e);
        if (!(e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0)) {
            return;
        }
        e.consume(this);

    }
	
};


struct CRBViWidget : ModuleWidget {

	acTouchRibbon* ribbon;

	CRBViWidget(CRBVi* module) {
		setModule(module);

		ribbon = createWidget<acTouchRibbon>(mm2px(Vec(1.5, 7.144)));//, module, CRBVi::PARAM_RIBBON);
		ribbon->box.pos = mm2px(Vec(7.2f,15.f));
		ribbon->box.size = mm2px(Vec(138.f, 80.f));
		ribbon->setModule(module);
		
		setPanel(createPanel(asset::plugin(pluginInstance, "res/CRBVi-White.svg"), asset::plugin(pluginInstance, "res/CRBVi-Dark.svg")));
		addChild(createWidget<ThemedScrew>(Vec(2, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x / 2 - 8, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addChild(ribbon);

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(111.373, 105.013)), module, CRBVi::OUTPUT_X));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(126.190, 105.013)), module, CRBVi::OUTPUT_Y));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(140.477, 105.013)), module, CRBVi::OUTPUT_GATE));
		addInput(createInputCentered<ThemedPJ301MPort>( mm2px(Vec(111.373, 118.772)), module, CRBVi::INPUT_VCA));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(126.190, 118.772)), module, CRBVi::OUTPUT_VCA));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(140.477, 118.772)), module, CRBVi::OUTPUT_POLY));

		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(20.014, 108.75)), module, CRBVi::PARAM_BASEOCT));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(44.107, 108.75)), module, CRBVi::PARAM_OCTAVES));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(86.456, 108.75)), module, CRBVi::PARAM_INPUTCURVE));

		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(65.251, 108.75)), module, CRBVi::PARAM_SNAP, CRBVi::LIGHT_SNAP));

	}

	void appendContextMenu(Menu* menu) override {
		CRBVi* module = getModule<CRBVi>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("CRBVi Options"));
		menu->addChild(createBoolPtrMenuItem("Show Keys in Pad", "", &module->showKeys));
	}

};

Model* modelCRBVi = createModel<CRBVi, CRBViWidget>("CRBVi");
