#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#include <Arduino.h>
class Potentiometer{
public:
    ~Potentiometer(){
        if(window != nullptr){
            free(window);
        }
    }
    void attach(int pin, int window_size, int min_value, int max_value){
        this->window_size = window_size;
        this->min_value = min_value;
        this->max_value = max_value - min_value;
        if(pin >= 0){
            this->pin = pin;
        }
        window = (float*)malloc(window_size * sizeof(float));
        for(int i = 0; i < window_size; ++i){
            window[i] = 0.0f;
        }
    }

    int update(){
        for(int i = window_size - 1; i > 0; --i){
            window[i] = window[i - 1];
        }

        window[0] = ((float)analogRead(pin) / 1023.0f) * (float)max_value;

        window_sum = 0.0f;

        for(int i = 0; i < window_size; i++){
            window_sum += window[i];
        }
        value = (int)round(window_sum / (float)window_size);
        value_changed = value != old_value;
        old_value = value;
        return value + min_value;
    }

    int read(){
        return value + min_value;
    }

    bool changed(){
        return value_changed;
    }

private:
    int min_value = 0;
    int max_value = 1023;
    int pin = 0;
    int window_size = 1;
    float* window;
    int value = 0;
    int old_value = 0;
    float window_sum = 0.0f;
    bool value_changed = 0;

};

#endif