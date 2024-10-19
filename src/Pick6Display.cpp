#include <rack.hpp>

template <class TModule>
struct Pick6Display : rack::LedDisplay {
	TModule* module;
	rack::Vec displaySize;
	int curstep = -1;
	int curpattern = 0;
	bool smartriffinblankpattern = false;
	short curpreset[8];
	bool presetMutes[6];
	bool isUsingSmartRiff = false;

	void drawLayer(const DrawArgs& args, int layer) override {

		if (layer == 1 && module) {
			curstep = module->curStep;
			curpattern = module->curPreset;
			smartriffinblankpattern = module->smartRiffInBlankPattern;

			for (int i=0;i<6;i++) {
				presetMutes[i] = module->muteValues[i];
            }
			for (int i=0;i<8;i++) {
				curpreset[i] = module->presets[curpattern][i];
            }
			isUsingSmartRiff = module->useSmartRiff;

			rack::Vec p;
			// Draw steps
			nvgScissor(args.vg, RECT_ARGS(args.clipBox));

			float stepX = (displaySize.x-2) / 8.0f;
			float stepY = (displaySize.y-2) / 7.0f;

			// middle lines
			nvgBeginPath(args.vg);
			nvgLineCap(args.vg, NVG_ROUND);
			nvgMiterLimit(args.vg, 2.f);
			nvgStrokeWidth(args.vg, 1.f);
			nvgStrokeColor(args.vg, nvgRGB(0x60,0x60,0x60));
			for (int i=0;i<6;i++) {
				nvgBeginPath(args.vg);
				p.x = rack::mm2px(1);
				p.y = rack::mm2px(stepY + i*stepY);
				nvgMoveTo(args.vg, VEC_ARGS(p));
				p.x = rack::mm2px(displaySize.x-1);
				nvgLineTo(args.vg, VEC_ARGS(p));
				nvgStroke(args.vg);
            }
		
			// steps

			for (int i=0;i<8;i++) {
				if (module) {
					int val = curpreset[i];
					if (val == 0) {
						// no pick
						nvgBeginPath(args.vg);
						p.x = rack::mm2px(1 + (i*stepX+2)-1);
						p.y = rack::mm2px(displaySize.y-3);
						nvgMoveTo(args.vg, VEC_ARGS(p));
						p.x = rack::mm2px(1 + (i*stepX+2)+1);
						p.y = rack::mm2px(displaySize.y-1);
						nvgLineTo(args.vg, VEC_ARGS(p));
						nvgLineCap(args.vg, NVG_BUTT);
						nvgMiterLimit(args.vg, 2.f);
						nvgStrokeWidth(args.vg, 1.f);
						if (isUsingSmartRiff && (curpattern > 0 || smartriffinblankpattern)) {
							nvgStrokeColor(args.vg, nvgRGBA(0xff,0xff,0xff,0xee));
                        } else {
							nvgStrokeColor(args.vg, nvgRGBA(0xff,0x66,0x00,0xa0));
                        }
						nvgStroke(args.vg);
						nvgBeginPath(args.vg);
						p.x = rack::mm2px(1 + (i*stepX+2)+1);
						p.y = rack::mm2px(displaySize.y-3);
						nvgMoveTo(args.vg, VEC_ARGS(p));
						p.x = rack::mm2px(1 + (i*stepX+2)-1);
						p.y = rack::mm2px(displaySize.y-1);
						nvgLineTo(args.vg, VEC_ARGS(p));
						nvgLineCap(args.vg, NVG_BUTT);
						nvgMiterLimit(args.vg, 2.f);
						nvgStrokeWidth(args.vg, 1.f);
						if (isUsingSmartRiff && (curpattern > 0 || smartriffinblankpattern)) {
							nvgStrokeColor(args.vg, nvgRGBA(0xff,0xff,0xff,0xee));
                        } else {
							nvgStrokeColor(args.vg, nvgRGBA(0xff,0x66,0x00,0xa0));
                        }
						nvgStroke(args.vg);
						// smart riff notes
						if (isUsingSmartRiff && (curpattern > 0 || smartriffinblankpattern)) {
							if ((module->isPlayingSmartRiffNote) && (curstep == i) ) {
								nvgBeginPath(args.vg);
								p.x = rack::mm2px(1 + (i*stepX+2));
								p.y = rack::mm2px((6-module->curTrig)*stepY);
								nvgMoveTo(args.vg, VEC_ARGS(p));
								p.x = rack::mm2px(1 + ((i+1)*stepX)-1);
								nvgLineTo(args.vg, VEC_ARGS(p));
								nvgLineCap(args.vg, NVG_BUTT);
								nvgMiterLimit(args.vg, 2.f);
								nvgStrokeWidth(args.vg, 5.f);
								nvgStrokeColor(args.vg, nvgRGBA(0xff,0xff,0xff,0xa0));
								nvgStroke(args.vg);
                            }
                        }
                    } else {
						// pick
						if (!presetMutes[val-1]) {
							// play this string
							nvgBeginPath(args.vg);
							p.x = rack::mm2px(1 + (i*stepX+2));
							p.y = rack::mm2px((7-val)*stepY);
							nvgMoveTo(args.vg, VEC_ARGS(p));
							p.x = rack::mm2px(1 + ((i+1)*stepX)-1);
							nvgLineTo(args.vg, VEC_ARGS(p));
							nvgLineCap(args.vg, NVG_BUTT);
							nvgMiterLimit(args.vg, 2.f);
							nvgStrokeWidth(args.vg, 5.f);
							nvgStrokeColor(args.vg, nvgRGB(0xff,0x66,0x00));
							nvgStroke(args.vg);
						} else {
							// muted string
							nvgBeginPath(args.vg);
							p.x = rack::mm2px(1 + (i*stepX+2));
							p.y = rack::mm2px((7-val)*stepY);
							nvgMoveTo(args.vg, VEC_ARGS(p));
							p.x = rack::mm2px(1 + ((i+1)*stepX)-1);
							nvgLineTo(args.vg, VEC_ARGS(p));
							nvgLineCap(args.vg, NVG_BUTT);
							nvgMiterLimit(args.vg, 2.f);
							nvgStrokeWidth(args.vg, 5.f);
							nvgStrokeColor(args.vg, nvgRGBA(0xff,0x66,0x00,0xa0));
							nvgStroke(args.vg);
							if (isUsingSmartRiff && (curpattern > 0 || smartriffinblankpattern)) {
								if ((module->isPlayingSmartRiffNote) && (curstep == i) ) {
									nvgBeginPath(args.vg);
									p.x = rack::mm2px(1 + (i*stepX+2));
									p.y = rack::mm2px((6-module->curTrig)*stepY);
									nvgMoveTo(args.vg, VEC_ARGS(p));
									p.x = rack::mm2px(1 + ((i+1)*stepX)-1);
									nvgLineTo(args.vg, VEC_ARGS(p));
									nvgLineCap(args.vg, NVG_BUTT);
									nvgMiterLimit(args.vg, 2.f);
									nvgStrokeWidth(args.vg, 5.f);
									nvgStrokeColor(args.vg, nvgRGBA(0xff,0xff,0xff,0xa0));
									nvgStroke(args.vg);
								}
							}
                        }
                    }
				}
			}

			// beat indicator

			if (curstep >= 0) {
				nvgBeginPath(args.vg);

				p.x = rack::mm2px(1 + ((curstep)*stepX));
				p.y = rack::mm2px(displaySize.y-2);
				nvgMoveTo(args.vg, VEC_ARGS(p));
				p.x = rack::mm2px(1 + ((curstep+1)*stepX));
				nvgLineTo(args.vg, VEC_ARGS(p));

				nvgLineCap(args.vg, NVG_BUTT);
				nvgMiterLimit(args.vg, 2.f);
				nvgStrokeWidth(args.vg, 3.f);
				nvgStrokeColor(args.vg, rack::SCHEME_WHITE);
				nvgStroke(args.vg);
			}
			
		}


		nvgResetScissor(args.vg);
		Widget::drawLayer(args, layer);
	}


};
