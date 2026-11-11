#ifndef RANGED_VALUE_H
#define RANGED_VALUE_H

#include <Arduino.h>
#include <RotaryEncoder.h>

template<typename T>
class RangedValue{
public:
    RangedValue(T min, T max)
    {
        this->min = min;
        this->max = max;
        value = min;
    }

    T update(RotaryEncoder::Direction direction){
        if(value < max && direction == RotaryEncoder::Direction::CLOCKWISE){
            value++;
        }else if(value > min && direction == RotaryEncoder::Direction::COUNTERCLOCKWISE){
            value--;
        }
        return value;
    }

    operator T() const {
        return value;
    }

    T operator= (const T& other){
        if(other > max){
            value = max;
        }else if(other < min){
            value = min;
        }else{
            value = other;
        }
        return value;
    }
private:
    T value;
    T min, max;
};

#endif