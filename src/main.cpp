#include <Arduino.h>
#include <Bounce2.h>
#include <RotaryEncoder.h>
#include <Potentiometer.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <DisplayTestRoutines.h>
#include <GUI.h>
#include <EEPROM.h>
#include <RangedValue.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <TempoFader.h>

#define MIDI_NAME {'T', 'i', 'm', 'e', ' ', 'Z', 'o', 'n', 'e'}
#define MIDI_NAME_LEN 9
#define MIDI_CHANNEL 1
#define NUM_MIDI_CHANNELS 4

#define POTI_PIN 14

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     17 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define CURRENT_PARAM(enc) (state.encoder_target ? parameters.transpose[enc] : parameters.tempo[enc])

//----------------------------------------------------------------------------------

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
GUI gui(display);

Bounce buttons[4];
Bounce play_toggle = Bounce();
Bounce transpose_toggle = Bounce();
RotaryEncoder *encoders[4];
Potentiometer poti = Potentiometer();
int previous_note = 0;
TempoFader clock_duration_24ppqn = TempoFader();
volatile int clock_pulse_counter = 0;
IntervalTimer clockTimer;

//------------------------------------------------------------------------------

enum Encoder{
   I = 0,
   II = 1,
   III = 2,
   IV = 3
};

enum EncoderTarget{
   TEMPO = 0,
   TRANSPOSE = 1
};

struct Parameters{
   RangedValue<uint8_t> tempo[4] = {
      RangedValue<uint8_t>(40, 240),
      RangedValue<uint8_t>(40, 240),
      RangedValue<uint8_t>(40, 240),
      RangedValue<uint8_t>(40, 240)
   };
   RangedValue<uint8_t> transpose[4] = {
      RangedValue<uint8_t>(0, 23),
      RangedValue<uint8_t>(0, 23), 
      RangedValue<uint8_t>(0, 23), 
      RangedValue<uint8_t>(0, 23)
   };
   int catch_up_time = 0;
}parameters;

struct State{
   bool play = 0;
   EncoderTarget encoder_target = TEMPO;
   Encoder selected_slot = I;
}state;

void saveParametersToEEPROM(Parameters& parameters){
   for(int i = 0; i < 4; ++i){
      EEPROM.write(i * 2, parameters.tempo[i]);
      EEPROM.write(i * 2 + 1, parameters.transpose[i]);
   }
}

void readParametersFromEEPROM(Parameters& parameters){
   for(int i = 0; i < 4; ++i){
      parameters.tempo[i] = EEPROM.read(i * 2);
      parameters.transpose[i] = EEPROM.read(i * 2 + 1);
   }
}

int bpmToFreq(uint8_t bpm){
   return (int)round(60000000.0 / ((double)bpm * 24.0));
}


void clockHandler(){
   #ifdef USB_MIDI
   usbMIDI.sendRealTime(usbMIDI.Clock);
   #endif
   if(clock_pulse_counter < 23){
      clock_pulse_counter++;
   }else{
      clock_pulse_counter = 0;
      clockTimer.update(clock_duration_24ppqn.update());
   }
}

void pollPlayToggle(){
   state.play = !play_toggle.read();
   if(state.play){
      // !TODO: Possibly not necessary, remove later
      // clock_pulse_counter = 23;
      #ifdef USB_MIDI
      usbMIDI.sendRealTime(usbMIDI.Start);
      #endif
      clockTimer.begin(clockHandler, clock_duration_24ppqn.update());
      clockTimer.priority(100);
      #ifndef USB_MIDI
      Serial.printf("Starting with Tempo: %d\n", clock_duration_24ppqn.getValue());
      #endif
   }else{
      clockTimer.end();
      #ifdef USB_MIDI
      usbMIDI.sendRealTime(usbMIDI.Stop);
      #endif
      #ifndef USB_MIDI
      Serial.printf("Stopping.\n");
      #endif
      clock_duration_24ppqn.stopFading();
      gui.drawMetronome(0, 0.0f);
      clock_pulse_counter = 23;
   }
}

void pollTransposeToggle(){
   state.encoder_target = (EncoderTarget)!transpose_toggle.read();
   #ifndef USB_MIDI
   Serial.printf("Encoder Target: %s\n", state.encoder_target ? "Transpose" : "Tempo");
   #endif
   gui.drawBPMText(CURRENT_PARAM(state.selected_slot), state.encoder_target);
   gui.drawBPMPreview({
      CURRENT_PARAM(0), 
      CURRENT_PARAM(1), 
      CURRENT_PARAM(2), 
      CURRENT_PARAM(3)}, 
      state.selected_slot,
      state.encoder_target
   );
}

//-------------------------------------------------------------
//-------------------------------------------------------------

void setup()
{
   for (int i = 0; i < 4; ++i)
   {
      encoders[i] = new RotaryEncoder(6 - i * 2 + 1, 6 - i * 2, RotaryEncoder::LatchMode::FOUR3);
      buttons[i] = Bounce();
      buttons[i].attach(11 - i, INPUT_PULLUP);
      buttons[i].interval(5);
   }

   poti.attach(POTI_PIN, 32, 1, 32);
   play_toggle.attach(15, INPUT_PULLUP);
   play_toggle.interval(5);
   transpose_toggle.attach(16, INPUT_PULLUP);
   transpose_toggle.interval(5);

   Serial.begin(115200);
   #ifndef USB_MIDI
   Serial.println("Hello World!");
   #endif

   readParametersFromEEPROM(parameters);

   parameters.catch_up_time = poti.update();
   transpose_toggle.update();
   state.encoder_target = (EncoderTarget)!transpose_toggle.read();

   // Wait for display
   delay(500);

   if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      #ifndef USB_MIDI
      Serial.println(F("SSD1306 allocation failed"));
      #endif
   }

   display.setRotation(2);

   // Clear the buffer
   display.clearDisplay();
   display.display();
   gui.drawBPMText(CURRENT_PARAM(state.selected_slot), state.encoder_target);
   gui.drawMetronome(0, 0.0f);
   gui.drawBPMPreview({
      CURRENT_PARAM(0), 
      CURRENT_PARAM(1), 
      CURRENT_PARAM(2), 
      CURRENT_PARAM(3)},
      state.selected_slot,
      state.encoder_target
   );


   #ifdef USB_MIDI
   for(int i = 1; i <= NUM_MIDI_CHANNELS; ++i){
      usbMIDI.sendNoteOn(parameters.transpose[state.selected_slot], 127, i);
   }
   #endif
   previous_note = parameters.transpose[state.selected_slot];

   clock_duration_24ppqn.init_value(bpmToFreq(parameters.tempo[state.selected_slot]));
   clock_pulse_counter = 23;

   poti.update();
   parameters.catch_up_time = poti.read();
   gui.drawCatchUpTime(parameters.catch_up_time);

   transpose_toggle.update();
   pollTransposeToggle();
   play_toggle.update();
   pollPlayToggle();
}

void loop()
{
   for (int i = 0; i < 4; ++i)
   {

      // Poll encoder rotation
      encoders[i]->tick();
      auto direction = encoders[i]->getDirection();
      if (direction != RotaryEncoder::Direction::NOROTATION)
      {
         CURRENT_PARAM(i).update(direction);
         #ifndef USB_MIDI
         Serial.printf("Encoder %d: %d\n", i, CURRENT_PARAM(i));
         #endif
         gui.drawBPMPreview({
            CURRENT_PARAM(0), 
            CURRENT_PARAM(1), 
            CURRENT_PARAM(2), 
            CURRENT_PARAM(3)}, 
            state.selected_slot,
            state.encoder_target
         );
         if(i == state.selected_slot){
            if(state.encoder_target == TEMPO){
               clock_duration_24ppqn.startFading(bpmToFreq(CURRENT_PARAM(state.selected_slot)), 1);
            }else{
               #ifdef USB_MIDI
               for(int i = 1; i <= NUM_MIDI_CHANNELS; ++i){
                  usbMIDI.sendNoteOff(previous_note, 0, i);
                  usbMIDI.sendNoteOn(parameters.transpose[state.selected_slot], 127, i);
               }
               #else
               for(int i = 1; i <= NUM_MIDI_CHANNELS; ++i){
                  Serial.println("Note Off %d on Channel %d", previous_note, i);
                  Serial.println("Note On %d on Channel %d", parameters.transpose[state.selected_slot], i);
               }
               #endif
               previous_note = parameters.transpose[state.selected_slot];
            }
            gui.drawBPMText(CURRENT_PARAM(state.selected_slot), state.encoder_target);
         }
      }

      buttons[i].update();
      if (buttons[i].changed())
      {
         bool pressed = !buttons[i].read();
         if(pressed){
            if((Encoder)i == state.selected_slot){
               EEPROM.write(i * 2 + state.encoder_target, CURRENT_PARAM(i));
            }else{
               state.selected_slot = (Encoder)i;
               clock_duration_24ppqn.startFading(
                  bpmToFreq(parameters.tempo[state.selected_slot]), 
                  state.play ? parameters.catch_up_time : 1
               );
               #ifdef USB_MIDI
               for(int i = 1; i <= NUM_MIDI_CHANNELS; ++i){
                  usbMIDI.sendNoteOff(previous_note, 0, i);
                  usbMIDI.sendNoteOn(parameters.transpose[state.selected_slot], 127, i);
               }
               #else
               for(int i = 1; i <= NUM_MIDI_CHANNELS; ++i){
                  Serial.println("Note Off %d on Channel %d", previous_note, i);
                  Serial.println("Note On %d on Channel %d", parameters.transpose[state.selected_slot], i);
               }
               #endif
               previous_note = parameters.transpose[state.selected_slot];
               gui.drawBPMPreview({
                  CURRENT_PARAM(0), 
                  CURRENT_PARAM(1), 
                  CURRENT_PARAM(2), 
                  CURRENT_PARAM(3)}, 
                  state.selected_slot,
                  state.encoder_target
               );
            }
         }
         #ifndef USB_MIDI
         Serial.printf("Button %d: %s\n", i, pressed ? "pressed" : "not pressed");
         #endif
         gui.drawBPMText(CURRENT_PARAM(i), state.encoder_target);
      }
   }

   poti.update();
   if (poti.changed())
   {
      parameters.catch_up_time = poti.read();
      gui.drawCatchUpTime(parameters.catch_up_time);
      #ifndef USB_MIDI
      Serial.printf("Poti: %d\n", parameters.catch_up_time);
      #endif
   }

   
   play_toggle.update();
   if(play_toggle.changed()){
      pollPlayToggle();
      #ifndef USB_MIDI
      Serial.printf("Play: %s\n", state.play ? "on" : "off");
      #endif
   }

   transpose_toggle.update();
   if(transpose_toggle.changed()){
      pollTransposeToggle();
   }

   if(clock_pulse_counter == 0){
      gui.drawMetronome(1, clock_duration_24ppqn.getProgression());
      #ifndef USB_MIDI
      Serial.printf("Progression: %f\n", clock_duration_24ppqn.getProgression());
      #endif
   }else if(clock_pulse_counter == 11){
      gui.drawMetronome(0, clock_duration_24ppqn.getProgression());
   }
}

//---------------------------------------------------