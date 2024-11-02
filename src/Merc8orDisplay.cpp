#include <rack.hpp>
#include "acModules.hpp"

template <class TModule>
struct Merc8orDisplay : rack::LedDisplay {
	TModule* module;
	rack::Vec displaySize;
	float lowVal = 0.f;
	float highVal = 0.f;
	float lowOut = 0.f;
	float highOut = 0.f;
	float cvVal = 0.f;
	float cvOut = 0.f;
	int voltageScale = 0;
	float rangeLow = -10.f;
	float rangeHigh = 10.f;
	float rangeLowOut = -10.f;
	float rangeHighOut = 10.f;
	std::string fontPath = rack::asset::system("res/fonts/ShareTechMono-Regular.ttf");


	void drawLayer(const DrawArgs& args, int layer) override {

		if (layer == 1 && module) {

			rack::Vec p;

			rangeLow = module->inCVlow;
			rangeHigh = module->inCVhigh;
			rangeLowOut = module->outCVlow;
			rangeHighOut = module->outCVhigh;

			nvgScissor(args.vg, RECT_ARGS(args.clipBox));
			nvgLineCap(args.vg, NVG_ROUND);
			nvgMiterLimit(args.vg, 2.f);
			nvgStrokeWidth(args.vg, 1.f);
			nvgFillColor(args.vg, nvgRGB(0x90,0x90,0x90));

			
			// cv val
			float y1 = rack::math::clamp(rack::math::rescale(rangeLow, -10.f, 10.f, 53.f, 1.f),1.f,53.f);
			float y2 = rack::math::clamp(rack::math::rescale(rangeHigh, -10.f, 10.f, 53.f, 1.f),1.f,53.f);
			float y3 = rack::math::clamp(rack::math::rescale(rangeLowOut, -10.f, 10.f, 53.f, 1.f),1.f,53.f);
			float y4 = rack::math::clamp(rack::math::rescale(rangeHighOut, -10.f, 10.f, 53.f, 1.f),1.f,53.f);

			// lines
			nvgStrokeColor(args.vg, nvgRGBA(0xd0,0xd0,0xd0, 0x40));

			// 0V middle
			p.x = rack::mm2px(1.f);
			p.y = rack::mm2px(27.f);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(p));
			p.x = rack::mm2px(41.f);
			nvgLineTo(args.vg, VEC_ARGS(p));
			nvgStroke(args.vg);
			// 5V 
			p.x = rack::mm2px(1.f);
			p.y = rack::mm2px(16.f);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(p));
			p.x = rack::mm2px(41.f);
			nvgLineTo(args.vg, VEC_ARGS(p));
			nvgStroke(args.vg);
			// -5V 
			p.x = rack::mm2px(1.f);
			p.y = rack::mm2px(42.f);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(p));
			p.x = rack::mm2px(41.f);
			nvgLineTo(args.vg, VEC_ARGS(p));
			nvgStroke(args.vg);
			// 10V 
			p.x = rack::mm2px(1.f);
			p.y = rack::mm2px(3.f);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(p));
			p.x = rack::mm2px(41.f);
			nvgLineTo(args.vg, VEC_ARGS(p));
			nvgStroke(args.vg);
			// -10V 
			p.x = rack::mm2px(1.f);
			p.y = rack::mm2px(55.f);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(p));
			p.x = rack::mm2px(41.f);
			nvgLineTo(args.vg, VEC_ARGS(p));
			nvgStroke(args.vg);

			nvgStrokeColor(args.vg, nvgRGBA(0xd0,0xd0,0xd0, 0x80));
			// low to low
			p.x = rack::mm2px(11.f);
			p.y = rack::mm2px(1.75f + y1);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(p));
			p.x = rack::mm2px(16.5f);
			p.y = rack::mm2px(1.75f + y3);
			nvgLineTo(args.vg, VEC_ARGS(p));
			nvgStroke(args.vg);
			// high to high
			p.x = rack::mm2px(11.f);
			p.y = rack::mm2px(2.25f + y2);
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(p));
			p.x = rack::mm2px(16.5f);
			p.y = rack::mm2px(2.25f + y4);
			nvgLineTo(args.vg, VEC_ARGS(p));
			nvgStroke(args.vg);


			// range bars
			nvgStrokeWidth(args.vg, 1.f);
			nvgFillColor(args.vg, nvgRGB(0xd0,0xd0,0xd0));
			p.x = rack::mm2px(3.f);
			p.y = rack::mm2px(2.f+y2);
			nvgBeginPath(args.vg);
			nvgRect(args.vg, p.x, p.y, rack::mm2px(8.f), rack::mm2px(y1-y2));
			nvgFill(args.vg);

			if (y4 < y3) {
				nvgFillColor(args.vg, nvgRGB(0xd0,0xd0,0xd0));
            } else {
				nvgFillColor(args.vg, nvgRGB(0xff,0x7f,0x2a));
            }
			p.x = rack::mm2px(16.5f);
			p.y = rack::mm2px(2.f+y4);
			nvgBeginPath(args.vg);
			nvgRect(args.vg, p.x, p.y, rack::mm2px(8.f), rack::mm2px(y3-y4));
			nvgFill(args.vg);

			std::shared_ptr<rack::Font> font = APP->window->loadFont(rack::asset::plugin(pluginInstance, fontPath));
			if (font) {
				nvgFontSize(args.vg, 9);
				nvgFontFaceId(args.vg, font->handle);
				// low
				std::string stepsStr = std::to_string(rangeLow);
				stepsStr = stepsStr.substr(0, stepsStr.find(".")+3);
				if (stepsStr == "-10.00") stepsStr = "-10.0";
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
				p.y = rack::mm2px(y1);
				if (rangeLow < -8.5f) {
					nvgFillColor(args.vg, nvgRGB(0x00,0x00,0x00));
					p.y += rack::mm2px(1.f);
                } else {
					nvgFillColor(args.vg, nvgRGB(0xf0,0xf0,0xf0));
					p.y += rack::mm2px(5.f);
                }
				nvgText(args.vg, rack::mm2px(10.75f),p.y, stepsStr.c_str(), NULL);
				// high
				stepsStr = std::to_string(rangeHigh);
				stepsStr = stepsStr.substr(0, stepsStr.find(".")+3);
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
				p.y = rack::mm2px(y2);
				if (rangeHigh > 8.5f) {
					nvgFillColor(args.vg, nvgRGB(0x00,0x00,0x00));
					p.y += rack::mm2px(5.f);
                } else {
					nvgFillColor(args.vg, nvgRGB(0xf0,0xf0,0xf0));
					p.y += rack::mm2px(1.f);
                }
				nvgText(args.vg, rack::mm2px(10.75f),p.y, stepsStr.c_str(), NULL);
				// low out
				stepsStr = std::to_string(rangeLowOut);
				stepsStr = stepsStr.substr(0, stepsStr.find(".")+3);
				if (stepsStr == "-10.00") stepsStr = "-10.0";
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
				p.y = rack::mm2px(y3);
				if (rangeLowOut < -8.5f) {
					nvgFillColor(args.vg, nvgRGB(0x00,0x00,0x00));
					p.y += rack::mm2px(1.f);
                } else {
					nvgFillColor(args.vg, nvgRGB(0xf0,0xf0,0xf0));
					p.y += rack::mm2px(5.f);
                }
				nvgText(args.vg, rack::mm2px(24.25f),p.y, stepsStr.c_str(), NULL);

				// high out
				stepsStr = std::to_string(rangeHighOut);
				stepsStr = stepsStr.substr(0, stepsStr.find(".")+3);
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
				p.y = rack::mm2px(y4);
				if (rangeHighOut > 8.5f) {
					nvgFillColor(args.vg, nvgRGB(0x00,0x00,0x00));
					p.y += rack::mm2px(5.f);
                } else {
					nvgFillColor(args.vg, nvgRGB(0xf0,0xf0,0xf0));
					p.y += rack::mm2px(1.f);
                }
				nvgText(args.vg, rack::mm2px(24.25f),p.y, stepsStr.c_str(), NULL);
			}


			// CV vals
			if (module->hasCVin) {
				nvgLineCap(args.vg, NVG_ROUND);
				nvgMiterLimit(args.vg, 2.f);
				nvgStrokeWidth(args.vg, 1.5f);
				nvgStrokeColor(args.vg, nvgRGB(0xff,0x7f,0x2a));
				for (int i=0;i<module->numChannels;i++) {
					nvgStrokeColor(args.vg, nvgRGB(0xff,0x7f,0x2a));
					y1 = rack::math::clamp(rack::math::rescale(module->cvIN[i], -10.f, 10.f, 53.f, 1.f),1.f,53.f);
					y2 = rack::math::clamp(rack::math::rescale(module->cvOUT[i], -10.f, 10.f, 53.f, 1.f),1.f,53.f);
					p.x = rack::mm2px(3.f);
					p.y = rack::mm2px(2.f + y1);
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, VEC_ARGS(p));
					p.x = rack::mm2px(11.f);
					nvgLineTo(args.vg, VEC_ARGS(p));
					nvgStroke(args.vg);

					if (y4 < y3) {
						nvgStrokeColor(args.vg, nvgRGB(0xff,0x7f,0x2a));
					} else {
						nvgStrokeColor(args.vg, nvgRGB(0xff,0xff,0xff));
					}
					p.x = rack::mm2px(16.5f);
					p.y = rack::mm2px(2.f + y2);
					nvgBeginPath(args.vg);
					nvgMoveTo(args.vg, VEC_ARGS(p));
					p.x = rack::mm2px(24.5f);
					nvgLineTo(args.vg, VEC_ARGS(p));
					nvgStroke(args.vg);
                }

            }



		}


		nvgResetScissor(args.vg);
		Widget::drawLayer(args, layer);
	}


};
