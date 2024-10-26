#include <rack.hpp>
#include "acModules.hpp"

template <class TModule>
struct Rhythm1101Display : rack::LedDisplay {
	TModule* module;
	rack::Vec displaySize;
	int curstep = -1;
	int curpattern = 0;
	short curpreset[16];
	short curpresetbase[16];
	bool presetMutes[6];

	const char* WhyDoesVCVCrashIfThisIsntHere[8] = { "1", "5", "9", "13","","","","" }; // it does...
	const char* stepLabels[8] = { "1", "5", "9", "13", "", "", "", ""};

	std::string fontPath = rack::asset::system("res/fonts/ShareTechMono-Regular.ttf");


	void drawLayer(const DrawArgs& args, int layer) override {

		if (layer == 1 && module) {
			curstep = module->curStep;
			curpattern = module->curPreset;

			for (int i=0;i<16;i++) {
				curpreset[i] = module->tempPattern[i];
				curpresetbase[i] = module->presets[curpattern][i];
            }

			rack::Vec p;
			// Draw steps
			nvgScissor(args.vg, RECT_ARGS(args.clipBox));

			float stepX = (displaySize.x-2.f) / 16.0f;
			float stepY = (displaySize.y-6.f) / 4.0f;
		
			// steps

			for (int i=0;i<16;i++) {
				if (module) {
					int val = curpreset[i];
					// step
					if (i <= module->numSteps) {
						for (int j=3;j>=0;j--) {
							nvgBeginPath(args.vg);
							p.x = rack::mm2px(0.5f+(i*stepX+1.75f));
							p.y = rack::mm2px((stepY/2.0f) + (j*stepY)-1.25f);
							nvgRect(args.vg, p.x, p.y, stepX*2, stepX*2);
							nvgLineCap(args.vg, NVG_BUTT);
							nvgMiterLimit(args.vg, 2.f);
							nvgStrokeWidth(args.vg, 1.0f);

							// this isn't great, but it works. will make better in the future. :-(
							if ( (getBinaryOfNumAt(val,j)) != (getBinaryOfNumAt(curpresetbase[i],j))) {
								// we've mutated here
								nvgStrokeColor(args.vg, nvgRGBA(0xff,0x66,0x00,0x90));
								nvgFillColor(args.vg, nvgRGBA(0xdd,0xdd,0xdd,0x99));
                            } else {
								nvgStrokeColor(args.vg, nvgRGBA(0xff,0x66,0x00,0x90));
								nvgFillColor(args.vg, nvgRGBA(0xdd,0xdd,0xdd,0xff));
                            }

							if ( (getBinaryOfNumAt(val,j) == 1) || (getBinaryOfNumAt(curpresetbase[i],j) == 1) ) {
								nvgFill(args.vg);
							} else {
								nvgStroke(args.vg);
							}
						}
					} else {
							for (int j=3;j>=0;j--) {
							nvgBeginPath(args.vg);
							p.x = rack::mm2px(0.5f+(i*stepX+1.75f));
							p.y = rack::mm2px((stepY/2.0f) + (j*stepY)-1.25f);
							nvgRect(args.vg, p.x, p.y, stepX*2, stepX*2);
							nvgLineCap(args.vg, NVG_BUTT);
							nvgMiterLimit(args.vg, 2.f);
							nvgStrokeWidth(args.vg, 1.0f);
							// only show empty squares here
							nvgStrokeColor(args.vg, nvgRGBA(0xff,0x66,0x00,0x30));
							nvgStroke(args.vg);
						}
                    }
				}
			}

			std::shared_ptr<rack::Font> font = APP->window->loadFont(rack::asset::plugin(pluginInstance, fontPath));
			if (font) {
				nvgFontSize(args.vg, 9);
				nvgFontFaceId(args.vg, font->handle);
				nvgFillColor(args.vg, nvgRGB(0xf0,0xf0,0xf0));
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
				float t = (displaySize.x-2.f) / 4.0f;
				for (int i=0;i<4;i++) {
					nvgText(args.vg, rack::mm2px(5.25f + (t*i)),rack::mm2px(displaySize.y-3.5f), stepLabels[i], NULL);
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

};
