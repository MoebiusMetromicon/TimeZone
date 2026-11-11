#ifndef GUI_H
#define GUI_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <initializer_list>

#define BLACK SSD1306_BLACK
#define WHITE SSD1306_WHITE

String noteTable[] = {
    "C 0",
    "C#0",
    "D 0",
    "D#0",
    "E 0",
    "F 0",
    "F#0",
    "G 0",
    "G#0",
    "A 0",
    "A#0",
    "B 0",
    "C 1",
    "C#1",
    "D 1",
    "D#1",
    "E 1",
    "F 1",
    "F#1",
    "G 1",
    "G#1",
    "A 1",
    "A#1",
    "B 1",
};

class GUI{
public:
    GUI(Adafruit_SSD1306& display) :
        display(display)
    {
        bpm_field_w = display.width() - 2 * bpm_field_x;
        bpm_field_h = display.height() / 2;
    };

    void drawBPMPreview(std::initializer_list<uint8_t> contents, int selected_slot, bool drawAsNote){
        display.fillRect(0, 0, display.width(), metronome_y, BLACK);
        display.setTextSize(1);
        int i = 0;
        for(auto content : contents){
            String text = drawAsNote ? noteTable[content] : String(content);
            display.setCursor(6 + i * (display.width() / 4), 2);
            int16_t x, y;
            uint16_t w, h;
            display.getTextBounds(text, display.getCursorX(), display.getCursorY(), &x, &y, &w, &h);
            if(i == selected_slot){
                display.fillRect(4 + i * (display.width() / 4), 0, (display.width() / 4) - 8, 12, WHITE);
                display.setTextColor(BLACK);
            }else{
                display.setTextColor(WHITE);
            }
            display.println(text);
            ++i;
        }
        display.display();
    }

    void drawBPMText(uint8_t content, bool drawAsNote){
        String text = drawAsNote ? noteTable[content] : String(content);
        display.fillRect(bpm_field_x, bpm_field_y, bpm_field_w, bpm_field_h, BLACK);
        display.setTextSize(4);
        display.setTextColor(WHITE);
        display.setCursor(bpm_field_x, bpm_field_y);
        int16_t x, y;
        display.getTextBounds(text, bpm_field_x, bpm_field_y, &x, &y, &bpm_field_w, &bpm_field_h);
        display.println(text);
        display.display();
    }

    void drawMetronome(bool on, float progression){
        display.fillRect(metronome_x, metronome_y, metronome_w, metronome_h, WHITE);
        display.fillRect(metronome_x + 1 + (int)floorf(progression * (float)(metronome_w)), metronome_y + 1, (int)ceilf((1.0f - progression) * (float)(metronome_w)) - 2, metronome_h - 2, BLACK);
        if(!on){
            display.fillRect(metronome_x + 1, metronome_y + 1, metronome_w - 2, metronome_h - 2, BLACK);
        }
        display.display();
    }

    void drawCatchUpTime(int time){
        display.fillRect(0, metronome_y + metronome_h, bpm_field_x, display.height() - metronome_y - metronome_h, BLACK);
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(6, 46);
        display.println(time);
        display.display();
    }
private:
    Adafruit_SSD1306& display;
    uint16_t bpm_field_x = 54;
    uint16_t bpm_field_y = 34;
    uint16_t bpm_field_w;
    uint16_t bpm_field_h;
    uint16_t metronome_x = 4;
    uint16_t metronome_y = 16;
    uint16_t metronome_w = 120;
    uint16_t metronome_h = 10;
};

#endif