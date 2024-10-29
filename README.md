# acModules

A collection of modules for VCV Rack, with both Dark and Light panels.

> [!IMPORTANT]
> 2.0.2 DOES fix crashes on Windows and Linux on all modules *except* Chord4Roy. A solution has been found and a new release (2.0.3) will come later this week. Stay tuned!

![Screenshot of acModules in VCV Rack](/images/acModules.png)

## Ov3rCross
A 3-band CV and Trigger "crossover".

[Ov3rCross Video](https://youtu.be/PI2qv28sgvA)

When given a "control" CV input (**CV IN**), Ov3rCross will route this incoming CV to one of three output "zones" based on the where that CV is in relation to **LOW** and **HIGH** CUTOFF points. For example, if the incoming CV value falls above the LOW cutoff point and below the HIGH cutoff point, the CV will be routed to the MID output. The cutoff points and incoming CV range are from -5V to 10V to accomodate different use cases.

You can use this control CV input (**CV IN**) to instead route the CV input at **CV THRU** to the outputs; this can, for example, send an oscillator output to different effects based on the control CV value.

When given an incoming trigger (**TRIG IN**) along with a CV input (**CV IN**), Ov3rCross will route the trigger to the same zone as the CV output.

If the trigger is connected but the control CV is *not* connected, Ov3rCross will generate a random voltage upon trigger input, and send that voltage (or CV thru voltage) and a trigger to the appropriate output. This could be considered a three-position random trigger with probability based on the low and high cutoff points. Neat!

There are four options in the context menu: 
- **"Sample And Hold Control CV"** will only output the voltage selected at the time of trigger input, instead of the voltage that is actively in that region (unless using CV thru). This option requires a trigger input.
- **"Mute Non-Active Outputs To 0V"** will very quickly ramp-down inactive CV outputs to 0V instead of leaving the last voltage in that region
- **"Random CV Range"** sets the range that the random control CV will be generated and output (if not using CV Thru) when no control CV is hooked up to the **CV IN** input. Options are -5V to 10V; -5V to 5V; 0-5V; and 0-10V.
- **"Output Trigger on CV Zone Change"** will output a trigger to a zone's trigger output when the incoming control CV enters that zone. This only functions when **not** using the Sample and Hold feature.

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
<!--

There are several tools found in the context menu:
- **"Copy..."** submenu lets you copy patterns from one side to the other, or from the current preset pattern in Pick6 to either side for customization.
- **"Clear..."** submenu lets you clear either left or right patterns, or both patterns.
-->

## Chord4Roy

Polyphonic guitar-style chord (with mutes) CV generator.

**Chord4Roy** will generate six CV and six mutes on one polyphonic output (**POLY OUT CV**):
- channels 1-6 are note CV
- channels 7-12 are 0V/10V normal/muted voltages
- channel 13 is the root note CV, regardless if it might be muted in the chord

You can set which note using the **ROOT NOTE** knob or CV input (0-10V), and select the chord using the **CHORD** knob or CV input (0-10V). Available chords are Root, Minor, 7th, Major 7th, Minor 7th, 6th, Minor 6th, and Sus.

You can choose between open-neck chords or bar chords (also called "barre") using the **STYLE** switch (or its CV input). You can also choose between muted strings in chords being marked as muted or left as open strings using the **MUTES** switch. When combined with a module like **Pick6** (see above), guitar chords can be played with the appropriate non-played strings ignored if desired; sometimes you might want to pick open strings in chords that might normally be muted.

*NOTE: In this implementation, there are no bar chords for m6 or sus chords; they are substituted with open-neck chords.*

## Rhythm1101

16-step drum-oriented trigger sequencer, with preset grooves.

Rhythm1101 is a basic sequencer that lets you create 16 4-channel patterns, and comes full of preset patterns based on classic drum tracks. Think of this as a quick way to add drum patterns to a larger song - just hook up the four **TRIG** outputs to drum modules, select a pattern, and send in a trigger to the **STEP** input. You can select which pattern plays using the **PATTERN** knob or connected CV input (0-10V) and set the length of the pattern between 1 and 16 steps using the **STEPS** knob.

Every step of any pattern can be edited using the 16 knobs below the display area. These knobs' values range from 0 to 15; 0 means no track played on that step, 15 is all tracks play on that step. Values inbetween follow a binary pattern (hence "1101" of the module's name), representing the four tracks; 1 is the first track, 2 is the second track, 3 is the first _and_ second track, 4 is the third track, and so on. This is easier in-practice than reading about it 😄 

As a bonus, there is the **MUTATE** feature that can be toggled on or off. This feature adds additional triggers to a track on rhythmically-appropriate steps using hand-coded weights and algorithms. The added triggers only persist for the playing of the current measure and do not alter the underlying pattern. You can set which tracks will be mutated using the knob in the **MUTATE** area, and the LEDs will light up to represent which tracks will be effected (these LEDs follow the same binary pattern as the step editing).

There are five(5) options in the context menu:
- **"Shift Pattern"** which lets you shift the current pattern to the right or left, one step at a time.
- **"Mutation Frequency"** allows you to set how often a mutation occurs, every measure (1:1), or every second (1:2), third (1:3), or fourth measure (1:4).
- **"Clear Current Pattern"** will clear the current pattern.
- **"Clear All Patterns"** will clear all 16 patterns.
- **"Reset To Factory Patterns"** will replace all the patterns with the preset grooves that come with the module.

# Changelog

<!--
## 2.0.3

- Chord4Roy: Fixes last remaining crash on windows
- Pick6p: context menu options for copying/clearing patterns
-->

## 2.0.2
- Addressed crash on Windows (and occassionally on Linux)
- Ov3rCross: Added trigger out on zone change
- Chord4Roy: Aligned visuals with other modules
- Rhythm1101: Added new module!

## 2.0.0
- Initial release

