# acModules

A collection of useful modules for VCV Rack, with both Dark and Light panels.

![Screenshot of acModules in VCV Rack](/images/acModules.png)

## Ov3rCross
A 3-band CV and Trigger "crossover".

[![Ov3rCross Video](/images/vidOv3r.png)](http://www.youtube.com/watch?v=PI2qv28sgvA)

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

[![Pick6 with Chord4Roy Video](/images/vidChordPick.png)](http://www.youtube.com/watch?v=Ij7Q0Ud_MjQ)

At its core, Pick6 is a simple trigger switch; with every trigger in (via **PICK**), a trigger out will be sent to one of six(6) output channels, on the polyphonic **POLY OUT** output. Which output channel is based on the current step in an 8-step sequence. Additionally, a single-channel trigger output occurs with every non-empty step at **MONO OUT**. These are both in the **TRIG** section, bottom-right.

The current pattern can be set via the **PATTERN** knob or it's connected CV input (0-10V). The pattern can be reset to the beginning using the **RESET** trigger input.

The **EOP** switch determines what happens at the **E**nd **o**f **P**attern. This will either: loop back to the beginning of the current pattern; move to the next pattern then return to the selected pattern after that one ends; or move to a blank pattern then return.

By default the **MONO OUT** output in the **CV** section (bottom-left) will output a voltage for an open-string guitar "E" chord based on the pattern step. To be more exciting, you can send a polyphonic input to the **POLY IN** input, also in the **CV** section. This input will accept six channels of CV for note PLUS an additional six channels of mutes (0V not-muted, 10V muted). On a guitar, some chords are made with certain strings not played, and this allows note generator modules to send in both note values and "do not play" (mute) information. *[**Chord4Roy** below is one such module]* You can also send in *less than* six channels of CV, and the highest channel (numerically) will be used for unpatched inputs. For example, if you send in a single channel of CV, all six strings will output that CV voltage; if you send in three channels, strings 1-3 will reflect your CV inputs while 4-6 will be the same as channel 3. In the case of less than six CV channels, no string mutes will occur and all strings will play (pattern skip steps will still happen).

To make this module even more interesting, there is the **SMART RIFF** functionality, which does the following: when sending in CV and mute data, if a chord has muted strings, Pick6 will pick non-muted notes to play on these steps; algorithmically (neither random nor all the time) add in notes on non-picked steps without repeating previous or next steps; remove picked notes in the middle or end of a pattern after a number of measures. This feature adds character to otherwise repetative sequences.

At the end of the pattern selection are two custom pattern slots. These can be edited using the **Pick6p** extender (see below).

There are two settings in the context menu:
- **"Muted Note CV to 0"** which will output 0V when a note is muted or not-picked instead of leaving it at the last step's voltage
- **"Smart Riff in Blank Pattern"** which will allow the Smart Riff functionality to do its thing when on a blank pattern, which you may actually want to be blank.
- **"Offset End of Pattern"** will reverse the sequence of the **EOP** setting, meaning that if you have that set to go to a blank pattern after the current pattern, Pick6 will start with the blank pattern and then go to the current pattern, and repeat from there. This allows a "dueling banjos" scenario, where two Pick6 modules take turns playing their pattern.

## Pick6p

Custom pattern programmer for **Pick6**. This should be attached directly to the right-side of **Pick6**; when attached, a LED will light on both modules near their names.

This is simply a 2x8 knob panel that edits the last two patterns in **Pick6**.

There are several tools found in the context menu:
- **"Copy..."** submenu lets you copy patterns from one side to the other, or from the current preset pattern in Pick6 to either side for customization.
- **"Clear..."** submenu lets you clear either left or right patterns, or both patterns.

## Chord4Roy

Polyphonic guitar-style chord (with mutes) CV generator.

[![Pick6 with Chord4Roy Video](/images/vidChordPick.png)](http://www.youtube.com/watch?v=Ij7Q0Ud_MjQ)

**Chord4Roy** will generate six CV and six mutes on one polyphonic output (**POLY OUT CV**):
- channels 1-6 are note CV
- channels 7-12 are 0V/10V normal/muted voltages
- channel 13 is the root note CV, regardless if it might be muted in the chord

You can set which note using the **ROOT NOTE** knob or CV input (0-10V), and select the chord using the **CHORD** knob or CV input (0-10V). Available chords are Root, Minor, 7th, Major 7th, Minor 7th, 6th, Minor 6th, and Sus.

You can choose between open-neck chords or bar chords (also called "barre") using the **STYLE** switch (or its CV input). You can also choose between muted strings in chords being marked as muted or left as open strings using the **MUTES** switch. When combined with a module like **Pick6** (see above), guitar chords can be played with the appropriate non-played strings ignored if desired; sometimes you might want to pick open strings in chords that might normally be muted.

*NOTE: In this implementation, there are no bar chords for m6 or sus chords; they are substituted with open-neck chords.*

Two context-menu options:
- **"Use V/Oct Root Note Selection"** will let you pick the root note using standard V/Oct (0-10V). Note: regardless of octave voltage passed in, there is only the one *output* octave.
- **"PianoMan Mode"** will output piano-style chords! This feature follows the guitar-oriented 6 note format and is geared towards arpeggiation. Therefore note 1 is the root note one octave down, notes 2-4 are chord notes, note 5 is either a chord note or root note one octave up, and note 6 is the second note of the chord one octave up. Whew! Mutes indicate whether the note is part of the chord or one of the octave shift notes. You might want to set the **MUTES** switch to "open" to get all six notes.

## Rhythm1101

16-step drum-oriented trigger sequencer, with preprogrammed grooves and algorithmic variation feature.

[![Rhythm1101 Video](/images/vidR1101.png)](http://www.youtube.com/watch?v=TKvci508R3Y)

Rhythm1101 (aka Rhythm13) is a basic sequencer that lets you create 16 4-channel patterns, and comes full of preset patterns based on classic drum tracks. Think of this as a quick way to add drum patterns to a larger song - just hook up the four **TRIG** outputs to drum modules, select a pattern, and send in a trigger to the **STEP** input. You can select which pattern plays using the **PATTERN** knob or connected CV input (0-10V) and set the length of the pattern between 1 and 16 steps using the **STEPS** knob.

Every step of any pattern can be edited using the 16 knobs below the display area. These knobs' values range from 0 to 15; 0 means no track played on that step, 15 is all tracks play on that step. Values inbetween follow a binary pattern (hence "1101" of the module's name), representing the four tracks; 1 is the first track, 2 is the second track, 3 is the first _and_ second track, 4 is the third track, and so on. This is easier in-practice than reading about it 😄 

As a bonus, there is the **MUTATE** feature that can be toggled on or off. This feature adds additional triggers to a track on rhythmically-appropriate steps using hand-coded weights and algorithms. The added triggers only persist for the playing of the current measure and do not alter the underlying pattern. You can set which tracks will be mutated using the knob in the **MUTATE** area, and the LEDs will light up to represent which tracks will be effected (these LEDs follow the same binary pattern as the step editing).

There are five(5) options in the context menu:
- **"Shift Pattern"** which lets you shift the current pattern to the right or left, one step at a time.
- **"Mutation Frequency"** allows you to set how often a mutation occurs, every measure (1:1), or every second (1:2), third (1:3), or fourth measure (1:4).
- **"Clear Current Pattern"** will clear the current pattern.
- **"Clear All Patterns"** will clear all 16 patterns.
- **"Reset To Factory Patterns"** will replace all the patterns with the preset grooves that come with the module.

## Merc8or

Quickly remap, scale, limit, and/or invert one range of polyphonic CV values to another.

[![Merc8or Video](/images/vidMerc8or.png)](https://youtu.be/wwNMGmk1ijA)

At it's core, this module is just a CV attenuverter/scaler/offset/limiter. However, unlike other modules that do the same thing, this one doesn't require you to do math to scale one range into another range or chain multiple modules. For instance, if I want to use a LFO that goes from -5V to 5V to control volume of my mixer that takes 0-10V, but only have it go between 2.5V and 4V, that requires me to think much too hard about offsets and scale factors. With **Merc8or**, I can select the CV input (**IN**) range of 10V on the **HIGH** knob and 0V on the **LOW** knob; on the output side (**OUT**) I select 4V on the **HIGH** knob, and 2.5V on the **LOW** knob. Done! If the incoming values go outside of the in range, they automatically get limited.

As mentioned, this is a polyphonic module, so you can convert up to 16 channels of CV from one range to another range, with all channels being converted through the same settings.

It can also invert the output range if you want, either by selecting **"Invert Output"** in the context menu, or put the **HIGH** output value lower than the **LOW** output value. In the above example, if I put the **HIGH** output knob at 2.5V and the **LOW** output knob at 4V, I'd have the same output range, but inverted!

All of these features let you use a single modulation source for multiple purposes easily and quickly. And without thinking about math. 😃

There are two more context menu options:
- **"Quick Set Input Range..."** provides several common options for input ranges.
- **"Quick Set Output Range..."** provides several commong output ranges as well as some targetted at notes/octave ranges.
- **"Invert Output"** will invert the output range (when not using MOREc8or)
- **"Output Snapping..."** allows the output range knobs *and* the voltage output to snap to **Octaves** or **Semitones** (or **None**). An additional option when any snap setting is to add a **Half-Semitone Offset** to the output, which is useful for some 1V/Oct modules that will "flicker" between notes depending upon rounding methedology. This is niche, but might be useful to you.

## Pul5es

Outputs one-shot or looping triggers upon receiving a specified number of incoming triggers. Because sometimes you just need to send a trigger every 23rd beat.

[![Pul5es Video](/images/vidPul5es.png)](https://youtu.be/peWUNlWQi_c)

This is a very simple module - send in triggers to the **STEP** input, and on the number of triggers the **OUT ON** knob is set to, Pul5es will send a trigger to **OUT**. As an example, if I want to send a trigger on every 3rd input trigger, I set the **OUT ON** knob to 3 and turn on **LOOP**. Another example: I might want to mute a mixer channel *after* 16 beats, so I set the **OUT ON** knob to 17, and send in beat triggers from my clock source; after 16 beats, on the 17th trigger in, Pul5es sends a trigger out and my mixer channel gets muted.

The input next to the **OUT ON** knob will let you use CV (0-10V) to control the timing of the output trigger instead of the knob.

There is a button, **LOOP**, which will start the counting over after sending a trigger. This enables you to repeatedly send a trigger every N-number of trigger **STEPS** in. <!--Next to the button there is an input which lets you use a trigger to toggle the **LOOP** state. (You can choose to use a gate instead using an option in the context menu; see below). Note that the CV control, trigger or gate, is only changing the way **LOOP** works, not the initial trigger output. If you don't want the initial trigger output, you can use a gate input *and* set "Only Count With Gate On" context menu option (again, see below).-->

There is also a **RESET** input which can be used to reset the count at any time. Typically I connect this to the "reset" AND "run" outputs on my clock source.

There are several context menu options:
- **"Invert Pulse Logic"** will send a trigger out on every **STEP** in *EXCEPT* for the **OUT ON** count number.
- **"Use Gate For Loop On/Off"** will change the way **LOOP** state works so that Pul5es only loops while a gate is connected to the loop CV input and is open/on/active.
- **"Only Count With Gate On"** will cause Pul5es to not send any triggers *unless you are using gate input **and** the gate is open/on/active*.

## CFor2N2ForC

Output a configurable CV and/or trigger across a 1V/Oct semitone CV input.

[![CFor2N2ForC Video](/images/vidCFor2.png)](https://youtu.be/CI5YjzepSeg)

Have you ever wanted to send a specific CV voltage to a module based on an incoming musical note? While now you can!

CFor2N2ForC will take a V/Oct CV input (**CV IN**), see what note it corresponds to, and output that note's CV voltage set by the note's knob. For instance, you might want to set a filter cutoff frequency higher for C notes than for F notes. Set the knobs next to C and F to the appropriate values, and when you send in CV for a C or F note, the values you set those note's knobs to will be output. 

CFor2N2ForC will also take an incoming trigger (**TRIG IN**) and route it to a 12-channel polyphonic output corresponding to the note the current CV refers to. Think of this as a 1:12 switch based on musical note.

Context menu options:
- **"Output Trigger On Note Change"** will send a trigger out when the incoming CV note changes. It will use the same 12-channel polyphonic output as incoming trigger outputs. This feature is useful for when you don't need incoming triggers but still want to trigger in a note-based way. Be aware that this only option only outputs a trigger when the note changes.
  
## MOREc8or

A right-side expander module for Merc8or, which adds CV control of Merc8or's output range parameters, including inverting, and linking. 

[![MOREc8or Video](/images/vidMorec8or.png)](https://youtu.be/CW1P8HefElA)

This module must be connected on the right-side of a Merc8or module; when attached, the small light in the names of both Merc8or and Morec8or will illuminate. There are three sections of controls **Invert**, **Link**, and CV inputs for the **High** and **Low** output range.

**Invert** provides manual, trigger, and gate control of inverting the output range in Merc8or. Pressing the manual button will simply invert the output range. CV input, coupled with the CV input type switch, allows remote triggering of the invert state. With the switch set to **Trig**, a CV trigger will act the same as the manual button and toggle the inverted state. When the switch is set to **Gate**, the output range will be temporarily inverted only while the CV is open/on.

**Link** will, well, *link* the High and Low output voltages together. For instance, if the Low output is set at 1V, and the High output is set at 2V, after enabling **Link**, the High output voltage will stay 1V away from the Low output voltage. When **Link** is enabled, only the Low output range knob in Merc8or will adjust the values; the LEDs next to each knob show either green or red depending upon their active or inactive state, respectively. Also, when enabled, the **Invert** controls become disabled.

Finally, the inputs at the bottom allow control of the output range via CV. Both inputs will set their respective output to any voltage you provide, from -10V to 10V. When cables are connected, **Invert** will be disabled. When **Link** is *enabled*, only the Low CV input will have any effect.

## C|RB Vi

C|RB Vi is a two-dimensional (XY) pad calibrated for note input across multiple octaves and with multiple Y-axis input curves, turning a simple XY pad into a performance instrument. It can also function as a playable VCA.

[![C|RB Vi Video](/images/vidCRBVi.png)](https://youtu.be/v6m56E9tSqk)

More details soon.

____

# Changelog

[Now in its own document!](./CHANGELOG.md)
