#include "acModules.hpp"
#include <math.hpp>

struct CRBViXL : Module {
	enum ParamId {
		PARAM_BASEOCT,
		PARAM_OCTAVES,
		PARAM_INPUTCURVE,
		PARAM_SNAP,
		PARAM_MODTOX,
		PARAM_MODTOY,
		PARAM_CURVEMODX,
		PARAM_CURVEMODY,
		PARAM_MODVCA,
		PARAM_MODRANGE,
		PARAMS_LEN
	};
	enum InputId {
		INPUT_VCA,
		INPUT_MOD,
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
		LIGHT_CURVEMODX,
		LIGHT_CURVEMODY,
		LIGHT_MODVCA,
		LIGHTS_LEN
	};

	float curSampleRate = 0.f;
	float padX = 0.f;
	float padY = 0.f;
	float curX = 0.f;
	float curY = 0.f;
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
	bool hasExtIn = false;
	int yAxisRangeMode = 0;
	int guideType = 0;

	bool curveModX = false;
	bool curveModY = false;
	bool modVCA = false;
	bool hasMod = false;
	float modVoltageIN = 0.f;
	float modVoltageIN2= 0.f;
	float modVoltageX  = 0.f;
	float modVoltageY  = 0.f;
	float modDisplayX  = 0.f;
	float modDisplayY  = 0.f;
	bool showModIndicators = false;

	dsp::BooleanTrigger snapTrigger;
	dsp::BooleanTrigger curveModXTrigger;
	dsp::BooleanTrigger curveModYTrigger;
	dsp::BooleanTrigger modVCATrigger;

	CRBViXL() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(PARAM_BASEOCT, -5.f, 4.f, 0.f, "Base Octave");
		configParam(PARAM_OCTAVES, 1.f, 5.f, 1.f, "Octaves");
		configParam(PARAM_INPUTCURVE, 0.f, 4.f, 0.f, "Input Curve");
		configSwitch(PARAM_SNAP, 0.f,1.f,0.f, "Snap To Notes");
		configParam(PARAM_MODTOX, -1.f, 1.f, 0.f, "Modulation To Note (X)");
		configParam(PARAM_MODTOY, -1.f, 1.f, 0.f, "Modulation To Volume (Y)");
		configSwitch(PARAM_CURVEMODX, 0.f,1.f,0.f, "Use Input Curve");
		configSwitch(PARAM_CURVEMODY, 0.f,1.f,0.f, "Use Input Curve");
		configSwitch(PARAM_MODVCA, 0.f,1.f,0.f, "Apply To VCA");
		configInput(INPUT_MOD, "Modulation");
		configSwitch(PARAM_MODRANGE, 0.f, 2.f, 0.f, "Modulation Voltage Range", {"-5V to +5V", "0V to +10V", "0V to +5V"});
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
		if (!curSampleRate || curSampleRate == 0.f) return;

		baseOctave = (int)params[PARAM_BASEOCT].getValue();
        numOctaves = (int)params[PARAM_OCTAVES].getValue();
		currentInputCurve = (int)params[PARAM_INPUTCURVE].getValue();

		hasExtIn = inputs[INPUT_VCA].isConnected();
		hasMod   = inputs[INPUT_MOD].isConnected();

		if (snapTrigger.process(params[PARAM_SNAP].getValue() > 0.f)) {
			isSnapped ^= true;
        }
		if (curveModXTrigger.process(params[PARAM_CURVEMODX].getValue() > 0.f)) {
			curveModX ^= true;
        }
		if (curveModYTrigger.process(params[PARAM_CURVEMODY].getValue() > 0.f)) {
			curveModY ^= true;
        }
		if (modVCATrigger.process(params[PARAM_MODVCA].getValue() > 0.f)) {
			modVCA ^= true;
        }

		if (!isDragging) {
			padY = 0.f;
        }

		if (!isNear(curX, padX)) {
			curX += (padX - curX)/500.f;
        } else {
			curX = padX;
        }
		if (!isNear(curY, padY)) {
			curY += (padY - curY)/500.f;
        } else {
			curY = padY;
        }

		if (isSnapped) {
			curVolt = clamp((float)baseOctave + (float)curKey * (guideType == 1 ? halfVoltPerNote : voltPerNote),-5.08f,5.08f);
        } else {
			// unsnapped has range added to lower and upper bounds to have the note be in center of onscreen keys
			curVolt = clamp(rescale(curX,0.f,1.f,(float)baseOctave - halfVoltPerNote,(float)baseOctave + (float)numOctaves + halfVoltPerNote),-5.08f,5.08f);			
        }

		bool polyMod = false;
		if (hasMod) {
			if (inputs[INPUT_MOD].getChannels() > 1) {
				polyMod = true;
				modVoltageIN = scaleModVoltage(inputs[INPUT_MOD].getPolyVoltage(0));
				modVoltageIN2= scaleModVoltage(inputs[INPUT_MOD].getPolyVoltage(1));
            } else {
				modVoltageIN = scaleModVoltage(inputs[INPUT_MOD].getVoltage());
				modVoltageIN2= scaleModVoltage(inputs[INPUT_MOD].getVoltage());
            }

			modVoltageX  = halfVoltPerNote * (modVoltageIN * params[PARAM_MODTOX].getValue() * (curveModX ? getVoltageCurveFor(curY/10.f) : 1.f));
			modVoltageY  = (modVoltageIN2 * params[PARAM_MODTOY].getValue() * (isDragging ? 1.f : 0.f));
			modDisplayX  = (modVoltageIN * params[PARAM_MODTOX].getValue() * (curveModX ? getVoltageCurveFor(curY/10.f) : 1.f));
			modDisplayY  = (modVoltageIN2 * params[PARAM_MODTOY].getValue() * (isDragging ? 1.f : 0.f) * (hasMod ? (curveModY ? getVoltageCurveFor(curY/10.f) : 1.f) : 1.f));
        }

		outputs[OUTPUT_X].setVoltage( clamp(curVolt + (hasMod ? modVoltageX : 0.f), -10.f, 10.f) );			
		outputs[OUTPUT_Y].setVoltage( voltToOutputRange(getVoltageCurveFor(clamp(curY+(hasMod ? (modVoltageY * (curveModY ? getVoltageCurveFor(curY/10.f) : 1.f)) : 0.f),0.f,10.f)/10.f)) );
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
		
		outputs[OUTPUT_POLY].setVoltage( clamp(curVolt + (hasMod ? modVoltageX : 0.f), -10.f, 10.f), 0 );						// note (X) adjusted for screen space
		outputs[OUTPUT_POLY].setVoltage( voltToOutputRange(getVoltageCurveFor(clamp(curY+(hasMod ? (modVoltageY * (curveModY ? getVoltageCurveFor(curY/10.f) : 1.f)) : 0.f),0.f,10.f)/10.f)) , 1 );			// volume (Y with applied input curve)
		outputs[OUTPUT_POLY].setVoltage( isDragging ? 10.f : 0.f, 2 );		// gate (on)
		outputs[OUTPUT_POLY].setVoltage( clamp(padX*10.f,0.f,10.f), 3 );	// raw pad X position (0-10V)
		outputs[OUTPUT_POLY].setVoltage( padY, 4 );							// raw pad Y position (0-10V)
		outputs[OUTPUT_POLY].setChannels(5);

		lights[LIGHT_SNAP].setBrightness(isSnapped ? 0.95f : 0.f);
		lights[LIGHT_CURVEMODX].setBrightness(curveModX ? 0.95f : 0.f);
		lights[LIGHT_CURVEMODY].setBrightness(curveModY ? 0.95f : 0.f);
		lights[LIGHT_MODVCA].setBrightness(modVCA ? 0.95f : 0.f);

	}

	float scaleModVoltage(float inVoltage) {
		if (params[PARAM_MODRANGE].getValue() == 0) {
			// -5 to 5
			inVoltage /= 5.0f;
		} else if (params[PARAM_MODRANGE].getValue() == 1) {
			// 0 to 10
			inVoltage /= 5.0f;
			inVoltage -= 1.f;
		} else {
			// 0 to 5
			inVoltage /= 2.5f;
			inVoltage -= 1.f;
        }
		return clamp(inVoltage,-1.f,1.f);
    }

	float getVoltageCurveFor(float inVolt) {
		float outVolt = inVolt;// clamp(curY+(hasMod ? modVoltageY : 0.f),0.f,10.f)/10.f;
		switch (currentInputCurve) {
		  case 1:
			outVolt = outVolt*outVolt*(outVolt+0.25f);
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
		return outVolt;
	}
	float voltToOutputRange(float inVolt) {
		float outVolt = inVolt;
		switch (yAxisRangeMode) {
			case 0:
				// 0-10
				outVolt *= 10.f;
				outVolt = clamp(outVolt,0.0f,10.0f);
				break;
			case 1:
				// 0-5
				outVolt *= 5.0f;
				outVolt = clamp(outVolt,0.0f,5.0f);
				break;
			case 2:
				// -5 - 5
				outVolt *= 10.f;
				outVolt -= 5.f;
				outVolt = clamp(outVolt,-5.0f,5.0f);
				break;
        }
		return outVolt;
    }

	float getVCAVoltageCurve(float inVoltage) {
		float outVolt = getVoltageCurveFor(curY/10.f);
		if (hasMod && modVCA) {
			outVolt *= modVoltageY;
        }
		return clamp(outVolt * inVoltage, -5.f,5.f);
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
		val = json_integer(yAxisRangeMode);
		json_object_set_new(rootJ, "yAxisRangeMode", val);
		val = json_boolean(curveModX);
		json_object_set_new(rootJ, "curveModX", val);
		val = json_boolean(curveModY);
		json_object_set_new(rootJ, "curveModY", val);
		val = json_boolean(modVCA);
		json_object_set_new(rootJ, "modVCA", val);
		val = json_boolean(showModIndicators);
		json_object_set_new(rootJ, "showModIndicators", val);
		val = json_integer(guideType);
		json_object_set_new(rootJ, "guideType", val);

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
		// y axis volt range
		val = json_object_get(rootJ, "yAxisRangeMode");
		if (val) {
			yAxisRangeMode = json_integer_value(val);
        }
		// mod X curve
		val = json_object_get(rootJ, "curveModX");
		if (val) {
			curveModX = json_boolean_value(val);
        }
		// mod Y curve
		val = json_object_get(rootJ, "curveModY");
		if (val) {
			curveModY = json_boolean_value(val);
        }
		// mod VCA
		val = json_object_get(rootJ, "modVCA");
		if (val) {
			modVCA = json_boolean_value(val);
        }
		// mod indicators
		val = json_object_get(rootJ, "showModIndicators");
		if (val) {
			showModIndicators = json_boolean_value(val);
        }
		// guide type
		val = json_object_get(rootJ, "guideType");
		if (val) {
			guideType = json_integer_value(val);
        }

	}

};


struct acTouchRibbonXL : rack::OpaqueWidget {

	CRBViXL* module = NULL;

	float padX;
	float padY;
	double curKey;
	double frac; // this is not really used except for in modf
	bool isDragging = false;
	int keyColors[12] = {2,0,1,0,1,1,0,1,0,1,0,1};
	int keyColorsQ[24] = {4,1,0,3,2,1,0,3,2,3,2,1,0,3,2,1,0,3,2,1,0,3,2,3}; // 0 b, 1 w<>b, 2 w, 3 w<>w, 4 C

	void step() override {
		if (module) {
			if (module->curSampleRate == 0.f) return;
			frac = modf(double((padX/box.size.x)*(float)(module->numOctaves*(module->guideType == 1 ? 24 : 12)+1)),&curKey);
			module->setPadInputs(padX/box.size.x, 10.0f-clamp((padY/(box.size.y-22.f))*10.1f,0.f,10.f), curKey);
        }
	}

	void drawLayer(const DrawArgs& args, int layer) override {

		nvgBeginPath(args.vg);
		nvgFillColor(args.vg, nvgRGB(0x20,0x20,0x20));
		nvgRect(args.vg, rack::mm2px(0.f), rack::mm2px(0.f), (box.size.x), (box.size.y));
		nvgFill(args.vg);

		if (layer == 1 && module) {

			if (module->curSampleRate == 0.f) return;

			nvgScissor(args.vg, RECT_ARGS(args.clipBox));

			if (module->showKeys) {
				int keys;
				float xinc;
				switch(module->guideType) {
					case 0:
						// semitones
						nvgStrokeColor(args.vg, nvgRGBA(0xff,0xff,0xff,0x40));
						nvgStrokeWidth(args.vg, 2);
						keys = module->numOctaves*12+1;
						xinc = box.size.x/(float)keys;
						for (int i=0;i<keys;i++) {
							int keyCol = keyColors[i%12];
							nvgBeginPath(args.vg);
							if (keyCol == 1) {
								// white key
								nvgFillColor(args.vg, nvgRGBA(0xf6,0xf6,0xf6,0xa0));
							} else if (keyCol == 2) {
								// C key
								nvgFillColor(args.vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
							} else {
								// black key
								nvgFillColor(args.vg, nvgRGBA(0x40,0x40,0x40,0x73));
							}
							nvgRect(args.vg, xinc*i+1.f,1.f,xinc-1.5f,box.size.y-25.f);
							nvgFill(args.vg);
						}
			
						nvgStrokeColor(args.vg, nvgRGBA(0x00,0x00,0x00,0x40));
						for (int i=1;i<keys;i++) {
							nvgBeginPath(args.vg);
							nvgMoveTo(args.vg, (xinc*i), box.size.y-25);
							nvgLineTo(args.vg, (xinc*i), 0.f);
							nvgStroke(args.vg);
						}
						break;
					case 1:
						// quartertones
						nvgStrokeColor(args.vg, nvgRGBA(0xff,0xff,0xff,0x40));
						nvgStrokeWidth(args.vg, 2);
						keys = module->numOctaves*24+1;
						xinc = box.size.x/(float)keys;
						for (int i=0;i<keys;i++) {
							int keyCol = keyColorsQ[i%24];
							nvgBeginPath(args.vg);
							if (keyCol == 0) {
								// black key
								nvgFillColor(args.vg, nvgRGBA(0x40,0x40,0x40,0x73));
							} else if (keyCol == 1) {
								// dark gray
								nvgFillColor(args.vg, nvgRGBA(0xd5,0xd5,0xd5,0x50));
							} else if (keyCol == 2) {
								// white
								nvgFillColor(args.vg, nvgRGBA(0xf6,0xf6,0xf6,0xa0));
							} else if (keyCol == 3) {
								// light gray
								nvgFillColor(args.vg, nvgRGBA(0xd5,0xd5,0xd5,0x50));
							} else if (keyCol == 4){
								// C key
								nvgFillColor(args.vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
                            }
							nvgRect(args.vg, ((xinc*i))+1.f,1.f,(xinc)-1.5f,box.size.y-25.f);
							nvgFill(args.vg);
						}
						
						nvgStrokeColor(args.vg, nvgRGBA(0x00,0x00,0x00,0x40));
						for (int i=1;i<keys;i++) {
							nvgBeginPath(args.vg);
							nvgMoveTo(args.vg, (xinc*i), box.size.y-25);
							nvgLineTo(args.vg, (xinc*i), 0.f);
							nvgStroke(args.vg);
						}
						
						break;
					case 2:
						// octaves (offset to match up with actual voltages)
						keys = module->numOctaves*12+1;
						xinc = box.size.x/(float)keys;
						for (int i=0;i<keys;i++) {
							if (i%12 == 0) {
								nvgStrokeColor(args.vg, nvgRGBA(0xff,0xff,0xff,0x40));
								nvgBeginPath(args.vg);
								nvgMoveTo(args.vg, (xinc*i)+(xinc/2.f), box.size.y-25);
								nvgLineTo(args.vg, (xinc*i)+(xinc/2.f), 0.f);
								nvgStroke(args.vg);
                            }
						}
						break;
                }
			}			

			if (isDragging) {
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, padX, box.size.y-5);
				nvgLineTo(args.vg, padX, padY + 22.f);
				nvgStrokeColor(args.vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
				nvgStrokeWidth(args.vg, 2);
				nvgStroke(args.vg);
				
				nvgBeginPath(args.vg);
				nvgCircle(args.vg, padX, padY, 20.f);
				nvgFillColor(args.vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
				nvgFill(args.vg);

				if (module->showModIndicators) {
					nvgBeginPath(args.vg);
					nvgCircle(args.vg, padX + rack::mm2px(module->modDisplayX*6.f), padY, 6.f);
					nvgFillColor(args.vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
					nvgFill(args.vg);

					nvgBeginPath(args.vg);
					nvgCircle(args.vg, padX, padY + rack::mm2px(module->modDisplayY*6.f), 6.f);
					nvgFillColor(args.vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
					nvgFill(args.vg);
				}				

			}

		} else {
			// no module (probably in library)
			nvgStrokeColor(args.vg, nvgRGBA(0xff,0xff,0xff,0x40));
			nvgStrokeWidth(args.vg, 2);
			int keys = 13;
			float xinc = box.size.x/(float)keys;
			for (int i=0;i<keys;i++) {
				int keyCol = keyColors[i%12];
				nvgBeginPath(args.vg);
				if (keyCol == 1) {
					// white key
					nvgFillColor(args.vg, nvgRGBA(0xe6,0xe6,0xe6,0xa0));
				} else if (keyCol == 2) {
					// C key
					nvgFillColor(args.vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
				} else {
					// black key
					nvgFillColor(args.vg, nvgRGBA(0xd5,0xd5,0xd5,0x73));
				}
				nvgRect(args.vg, xinc*i+1.f,1.f,xinc-1.5f,box.size.y-25.f);
				nvgFill(args.vg);
			}
			
			nvgStrokeColor(args.vg, nvgRGBA(0x00,0x00,0x00,0x40));
			for (int i=1;i<keys;i++) {
				nvgBeginPath(args.vg);
				nvgMoveTo(args.vg, (xinc*i), box.size.y-25);
				nvgLineTo(args.vg, (xinc*i), 0.f);
				nvgStroke(args.vg);
			}

			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, box.size.x*.33f, box.size.y-5);
			nvgLineTo(args.vg, box.size.x*.33f, box.size.y*.45f + 22.f);
			nvgStrokeColor(args.vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
			nvgStrokeWidth(args.vg, 2);
			nvgStroke(args.vg);
				
			nvgBeginPath(args.vg);
			nvgCircle(args.vg, box.size.x*.33f, box.size.y*.45f, 20.f);
			nvgFillColor(args.vg, nvgRGBA(0xff,0xcc,0xaa,0xa0));
			nvgFill(args.vg);

        }

		// finally the rest
		nvgBeginPath(args.vg);
		nvgStrokeColor(args.vg, nvgRGB(0xff,0xcc,0xaa));
		nvgMoveTo(args.vg, 0, box.size.y-24);
		nvgLineTo(args.vg, box.size.x, box.size.y-24);
		nvgStrokeWidth(args.vg, 1);
		nvgStroke(args.vg);

		nvgResetScissor(args.vg);
		Widget::drawLayer(args, layer);
	}


	void onDragStart(const DragStartEvent &e) override {
		if (module) {
			module->isDragging = true;
			isDragging = true;
        }
		e.consume(this);
    }
	void onDragEnd(const DragEndEvent &e) override {
		if (module) {
			module->isDragging = false;
			isDragging = false;
        }
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


struct CRBViXLWidget : ModuleWidget {

	CRBViXLWidget(CRBViXL* module) {
		setModule(module);

		setPanel(createPanel(asset::plugin(pluginInstance, "res/CRBViXL-White.svg"), asset::plugin(pluginInstance, "res/CRBViXL-Dark.svg")));

		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ThemedScrew>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ThemedScrew>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(263.497, 105.013)), module, CRBViXL::OUTPUT_X));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(278.314, 105.013)), module, CRBViXL::OUTPUT_Y));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(292.601, 105.013)), module, CRBViXL::OUTPUT_GATE));
		addInput(createInputCentered<ThemedPJ301MPort>( mm2px(Vec(263.497, 118.772)), module, CRBViXL::INPUT_VCA));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(278.314, 118.772)), module, CRBViXL::OUTPUT_VCA));
		addOutput(createOutputCentered<ThemedPJ301MPort>(mm2px(Vec(292.601, 118.772)), module, CRBViXL::OUTPUT_POLY));

		addInput(createInputCentered<ThemedPJ301MPort>(mm2px(Vec(135.151, 105.013)), module, CRBViXL::INPUT_MOD));
		addParam(createParamCentered<CKSSThreeHorizontal>(mm2px(Vec(135.151, 117.535)), module, CRBViXL::PARAM_MODRANGE));

		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(20.014, 108.75)), module, CRBViXL::PARAM_BASEOCT));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(49.398, 108.75)), module, CRBViXL::PARAM_OCTAVES));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(102.331, 108.75)), module, CRBViXL::PARAM_INPUTCURVE));

		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(75.834, 108.75)), module, CRBViXL::PARAM_SNAP, CRBViXL::LIGHT_SNAP));

		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(157.967, 108.75)), module, CRBViXL::PARAM_MODTOX));
		addParam(createParamCentered<Davies1900hWhiteKnob>(mm2px(Vec(195.009, 108.75)), module, CRBViXL::PARAM_MODTOY));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(173.842, 108.75)), module, CRBViXL::PARAM_CURVEMODX, CRBViXL::LIGHT_CURVEMODX));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(210.884, 108.75)), module, CRBViXL::PARAM_CURVEMODY, CRBViXL::LIGHT_CURVEMODY));
		addParam(createLightParamCentered<VCVLightButton<MediumSimpleLight<WhiteLight>>>(mm2px(Vec(226.759, 108.75)), module, CRBViXL::PARAM_MODVCA, CRBViXL::LIGHT_MODVCA));

		acTouchRibbonXL* ribbon = createWidget<acTouchRibbonXL>(mm2px(Vec(7.2f, 15.f)));
		ribbon->box.pos = mm2px(Vec(7.2f,15.f));
		ribbon->box.size = mm2px(Vec(289.71f, 80.f));
		ribbon->module = module;
		addChild(ribbon);
		
	}

	void appendContextMenu(Menu* menu) override {
		CRBViXL* module = getModule<CRBViXL>();
		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("CRB Vi XL Options"));
		menu->addChild(createBoolPtrMenuItem("Show Guides", "", &module->showKeys));
		menu->addChild(createIndexPtrSubmenuItem("Guide Type...",
			{"Semitones", "Quartertones", "Octaves"},
			&module->guideType
		));
		menu->addChild(createIndexPtrSubmenuItem("Y-Axis Range (Non-VCA)",
			{"0V to 10V (Default)", "0V to 5V", "-5V to 5V"},
			&module->yAxisRangeMode
		));
		menu->addChild(createBoolPtrMenuItem("Show Modulation While Playing", "", &module->showModIndicators));
	}

};

Model* modelCRBViXL = createModel<CRBViXL, CRBViXLWidget>("CRBViXL");
