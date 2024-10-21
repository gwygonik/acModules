# acModules

A collection of modules for VCV Rack, with both Dark and Light panels.

![Screenshot of acModules in VCV Rack](/images/acModules.png)

## Ov3rCross
A 3-band CV and Trigger "crossover".

When given a control CV input (**CV IN**) an incoming trigger (**TRIG IN**) will route the CV and trigger to one of three outputs based on the **LOW** and **HIGH** CUTOFF knobs. The cutoff points and incoming CV range are from -5V to 10V to accomodate different use cases.

Alternatively, you can route a different CV input (**CV THRU**) to the outputs instead of the primary CV input; this can, for example, send an oscillator output to different effects based on the primary CV value.

If the control CV is not connected (**CV IN**), the module will select a random voltage upon trigger input, and send that voltage (or CV thru voltage) and a trigger to the appropriate output. This could be considered a three-position random trigger with probability based on the low and high points. Neat!

There are three options in the context menu: 
- **"Sample And Hold Control CV"** will only output the voltage selected at the time of trigger input, instead of the voltage that is actively in that region (unless using CV thru)
- **"Mute Non-Active Outputs To 0V"** will very quickly ramp-down inactive CV outputs to 0V instead of leaving the last voltage in that region
- **"Random CV Range"** sets the range that the random control CV will be generated and output (if not using CV Thru) when no control CV is hooked up to the **CV IN** input.

## Pick6
8-step finger-pickin' trigger sequencer (1:6 patterned switch), with optional "Smart Riff" functionality.

At its core, Pick6 is a simple trigger switch; with every trigger in (via **PICK**), a trigger out will be sent to one of six(6) output channels, on the polyphonic **POLY OUT** output. Which output channel is based on the current step in an 8-step sequence. Additionally, a single-channel trigger output occurs with every non-empty step at **MONO OUT**. These are both in the **TRIG** section, bottom-right.

The current pattern can be set via the **PATTERN** knob or it's connected CV input (0-10V). The pattern can be reset to the beginning using the **RESET** trigger input.

The **EOP** switch determines what happens at the **E**nd **o**f **P**attern. This will either: loop back to the beginning of the current pattern; move to the next pattern then return to the selected pattern after that one ends; or move to a blank pattern then return.

By default the **MONO OUT** output in the **CV** section (bottom-left) will output a voltage for an open-string guitar "E" chord based on the pattern step. To be more exciting, you can send a polyphonic input to the **POLY IN** input, also in the **CV** section. This input will accept six channels of CV for note PLUS an additional six channels of mutes (0V not-muted, 10V muted). On a guitar, some chords are made with certain strings not played, and this allows note generator modules to send in both note values and "do not play" (mute) information. *[**Chord4Roy** below is one such module]* You can also send in *less than* six channels of CV, and the highest channel (numerically) will be used for unpatched inputs. For example, if you send in a single channel of CV, all six strings will output that CV voltage; if you send in three channels, strings 1-3 will reflect your CV inputs while 4-6 will be the same as channel 3. In the case of less than six CV channels, no string mutes will occur and all strings will play (pattern skip steps will still happen).

To make this module even more interesting, there is the **SMART RIFF** functionality, which does the following: when sending in CV and mute data, if a chord has muted strings, Pick6 will pick non-muted notes to play on these steps; algorithmically (neither random nor all the time) add in notes on non-picked steps without repeating previous or next steps; remove picked notes in the middle or end of a pattern after a number of measures. This feature adds character to otherwise repetative sequences.

At the end of the pattern selection are two custom pattern slots. These can be edited using the **Pick6p** extender (see below).

There are two settings in the context menu:
- **"Muted Note CV to 0"** which will output 0V when a note is muted or not-picked instead of leaving it at the last step's voltage
- **"Smart Riff in Blank Pattern"** which will allow the Smart Riff functionality to do its thing when on a blank pattern, which you may actually want to be blank.

## Pick6p

Custom pattern programmer for **Pick6**. This should be attached directly to the right-side of **Pick6**; when attached, a LED will light on both modules near their names.

This is simply a 2x8 knob panel that edits the last two patterns in **Pick6**.

## Chord4Roy

Polyphonic guitar-style chord (with mutes) CV generator.

**Chord4Roy** will generate six CV and six mutes on one polyphonic output (**POLY OUT CV**):
- channels 1-6 are note CV
- channels 7-12 are 0V/10V normal/muted voltages
- channel 13 is the root note CV, regardless if it might be muted in the chord

You can set which note using the **ROOT NOTE** knob or CV input (0-10V), and select the chord using the **CHORD** knob or CV input (0-10V). Available chords are Root, Minor, 7th, Major 7th, Minor 7th, 6th, Minor 6th, and Sus.

You can choose between open-neck chords or bar chords (also called "barre") using the **STYLE** switch (or its CV input). You can also choose between muted strings in chords being marked as muted or left as open strings using the **MUTES** switch. When combined with a module like **Pick6** (see above), guitar chords can be played with the appropriate non-played strings ignored if desired; sometimes you might want to pick open strings in chords that might normally be muted.

*NOTE: In this implementation, there are no bar chords for m6 or sus chords; they are substituted with open-neck chords.*

# Changelog

## 2.0.0
- Initial release

