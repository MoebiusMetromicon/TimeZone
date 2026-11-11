# TimeZone
TimeZone is a simple hardware MIDI master clock and transposition tool based on a teensy 3.2 microcontroller. 

## Concept
TimeZone is a companion to oldschool hardware sequencers like the Doepfer Darktime. Many of these hands-on sequencers don't feature numerical BPM-based tempo selection and don't have global note transposition build-in. During live performances I was missing those features, so I build TimeZone to allow for precise and fast tempo and key changes between songs during performances. Currently the device only outputs MIDI messages via USB- However physical DIN MIDI or CV/Gate options for modular synths are straight-forward extensions.

## User Interface & Features
TimeZone features 4 slots for saving tempos and transposition values. 
- _Changing tempo & key:_ The 4 endoders correspond to the 4 tempo/transposition slots. Turning the encoders modifies the tempo or the transpotion note of the slot (depending on the position of a toggle switch).
- _Changing Slots:_ Pushing the encoder of a slot which isn't active activates it. This changes the MIDI realtime clock to the new tempo, sends a note off message for the old transposition note and a note on message for the new note.
- _Saving a slot:_ Pressing the encoder of a slot which is already active, saves the tempo and note setting of this slot to the non-volatile EEPROM memory.
- _Transport control:_ The clock is activated by a start/stop toggle switch, which also sends the corresponding realtime MIDI start/stop messages.
- _Transition time:_ A potentiometer sets the number of 16th notes it takes to smoothly transition from one tempo to another. This parameter is global and can't be saved to EEPROM. 
- _Display:_ The current BPM, key, transition time, transport state, active slot and transition time is displayd on small i2c display.

<img width="3072" height="2048" alt="TimeZone" src="https://github.com/user-attachments/assets/f9bc853a-6a2b-40dd-9a50-ef5faf7282b8" />

## MIDI Specifics
The clock is sent as a 32 ppqn MIDI realtime message and only the the transport toggle is set to 'play'. Transposition is handled as a simple note on (or off) message in the loweset 2 octaves. You the actual transposition is achieved is up to the user and depends on your specific setup. E.g. some sequencers have dedicated transpose inputs, modular uses could use MIDI-CV converters and precision adders.

## Hardware
I used a Teensy 3.2 microcontroller for this project since it features a convenient USB MIDI mode (and I had one left over from a prvious project). Nothing is stopping you from adapting this project for a different microcontroller though. The other physical components aren't anything specific or fancy; I hand-wired and soldered them to a piece of stripboard. However, I'll add schematics to this repo (which are very simple...) if I'll find the time.

## Building the Project
The code for TimeZone is written in C++ and uses the PlatformIO VS Code extension for dependency management, building and programming the teensy.
