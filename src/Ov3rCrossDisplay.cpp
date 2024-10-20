#include <rack.hpp>

template <class TModule>
struct Ov3rCrossDisplay : rack::LedDisplay {
	TModule* module;
	rack::Vec displaySize;
	float lowVal = 0.f;
	float highVal = 0.f;
	float cvVal = 0.f;
	int voltageScale = 0;
	float rangeLow = -5.f;
	float rangeHigh = 10.f;
	std::string fontPath = rack::asset::system("res/fonts/ShareTechMono-Regular.ttf");


	void drawLayer(const DrawArgs& args, int layer) override {

		if (layer == 1 && module) {

			rack::Vec p;

			nvgScissor(args.vg, RECT_ARGS(args.clipBox));
			nvgLineCap(args.vg, NVG_ROUND);
			nvgMiterLimit(args.vg, 2.f);
			nvgStrokeWidth(args.vg, 2.f);
			nvgStrokeColor(args.vg, nvgRGB(0xd0,0xd0,0xd0));
			nvgFillColor(args.vg, nvgRGB(0x80,0x80,0x80));

			// cv val
			p.x = rack::mm2px(1.f);
			p.y = rack::mm2px(102.f);
			nvgBeginPath(args.vg);
			nvgRoundedRect(args.vg, p.x, p.y, rack::mm2px(16.f), rack::mm2px(-1.f*rack::math::rescale(rack::math::clamp(module->rtCVIN,rangeLow,rangeHigh), rangeLow, rangeHigh, 1.f, 100.f)), 2.f);
			nvgFill(args.vg);

			// low cut
			p.x = rack::mm2px(1.f);
			p.y = rack::mm2px(2.f+(100.f-rack::math::clamp(rack::math::rescale(module->lowCut, rangeLow, rangeHigh, 1.f, 100.f),1.f,100.f)));
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(p));
			p.x = rack::mm2px(17.f);
			nvgLineTo(args.vg, VEC_ARGS(p));
			nvgStroke(args.vg);

			// high cut
			p.x = rack::mm2px(1.f);
			p.y = rack::mm2px(2.f+(100.f-rack::math::clamp(rack::math::rescale(module->highCut, rangeLow, rangeHigh, 1.f, 100.f),1.f,100.f)));
			nvgBeginPath(args.vg);
			nvgMoveTo(args.vg, VEC_ARGS(p));
			p.x = rack::mm2px(17.f);
			nvgLineTo(args.vg, VEC_ARGS(p));
			nvgStroke(args.vg);

			std::shared_ptr<rack::Font> font = APP->window->loadFont(fontPath);
			if (font) {
				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgFillColor(args.vg, nvgRGB(0xd0,0xd0,0xd0));
				std::string stepsStr = std::to_string(module->lowCut);
				stepsStr = stepsStr.substr(0, stepsStr.find(".")+3);
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
				p.y = rack::mm2px(2.f+(100.f-rack::math::clamp(rack::math::rescale(module->lowCut, rangeLow, rangeHigh, 1.f, 100.f),1.f,100.f)));
				if (module->lowCut < -4.5f) {
					p.y -= rack::mm2px(1.f);
                } else {
					p.y += rack::mm2px(3.5f);
                }
				nvgText(args.vg, rack::mm2px(16),p.y, stepsStr.c_str(), NULL);
				// high
				stepsStr = std::to_string(module->highCut);
				stepsStr = stepsStr.substr(0, stepsStr.find(".")+3);
				nvgTextAlign(args.vg, NVG_ALIGN_RIGHT);
				p.y = rack::mm2px(2.f+(100.f-rack::math::clamp(rack::math::rescale(module->highCut, rangeLow, rangeHigh, 1.f, 100.f),1.f,100.f)));
				if (module->highCut > 9.5f) {
					p.y += rack::mm2px(3.5f);
                } else {
					p.y -= rack::mm2px(1.f);
                }
				nvgText(args.vg, rack::mm2px(16),p.y, stepsStr.c_str(), NULL);
			}

		}


		nvgResetScissor(args.vg);
		Widget::drawLayer(args, layer);
	}


};
