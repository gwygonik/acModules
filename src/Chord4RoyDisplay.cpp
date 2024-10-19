#include <rack.hpp>

template <class TModule>
struct Chord4RoyDisplay : rack::LedDisplay {
	TModule* module;
	rack::Vec displaySize;
	const char* noteNameLabels[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	const char* chordTypeLabels[8] = { "", "min", "7", "Maj7", "min7", "6", "min6", "Sus"};

	int curnote;
	int curchord;
	std::string fontPath = rack::asset::system("res/fonts/ShareTechMono-Regular.ttf");

	void drawLayer(const DrawArgs& args, int layer) override {

		if (layer == 1 && module) {
			curnote = module->curNote - 1;
			curchord = module->curChord-1;

			rack::Vec p;
			// Draw steps
			nvgScissor(args.vg, RECT_ARGS(args.clipBox));
			nvgBeginPath(args.vg);

			std::shared_ptr<rack::Font> font = APP->window->loadFont(fontPath);
			if (font) {
				nvgFontSize(args.vg, 30);
				nvgFontFaceId(args.vg, font->handle);
				nvgFillColor(args.vg, nvgRGB(0xf0,0xf0,0xf0));
				nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
				nvgText(args.vg, rack::mm2px(1.75f),rack::mm2px(displaySize.y-3.5f), noteNameLabels[curnote], NULL);

				nvgFontSize(args.vg, 12);
				nvgFontFaceId(args.vg, font->handle);
				nvgFillColor(args.vg, nvgRGB(0xf0,0xf0,0xf0));
				nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
				nvgText(args.vg, rack::mm2px(13.f),rack::mm2px(displaySize.y-7.5f), chordTypeLabels[curchord], NULL);

				nvgFontSize(args.vg, 10);
				nvgFontFaceId(args.vg, font->handle);
				nvgFillColor(args.vg, nvgRGB(0xc0,0xc0,0xc0));
				nvgTextAlign(args.vg, NVG_ALIGN_LEFT);
				nvgText(args.vg, rack::mm2px(13.f),rack::mm2px(displaySize.y-2.5f), module->isUsingBar ? "BAR" : "OPEN", NULL);
			}

		}


		nvgResetScissor(args.vg);
		Widget::drawLayer(args, layer);
	}


};
