#ifndef TEMPO_FADER_H
#define TEMPO_FADER_H

class TempoFader{
public:
    void init_value(int default_value){
        value = (float)default_value;
        start = end = default_value;
        fading = 0;
    }

    void startFading(int end, int num_steps){
        this->end = end;
        this->start = (int)value;
        fading = 1;
        step = ((float)end - (float)start) / (float)num_steps;
    }

    void stopFading(){
        fading = 0;
        value = end;
    }

    int update(){
        if(fading){
            value += (float)step;
            if((end > start && value >= end) || (end < start && value <= end)){
                stopFading();
            }
        }

        return (int)round(value);
    }

    int getValue(){
        return (int)round(value);
    }

    float getProgression(){
        if(start != end){
            return (float)(value - (float)start) / float(end - start);
        }else{
            return 1.0f;
        }
    }
    
private:
    bool fading = 0;
    float value = 0.0f;
    int start = 0;
    int end = 0;
    float step = 0.0f;
};

#endif