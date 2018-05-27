#ifndef TONE_H
#define TONE_H

typedef struct
{
    float value1K;
    float value4K;
} Tone_Reading;

// Initialises the tone ADC config
extern void mc_tone_init(void);
// Returns a tone ADC sample reading
extern Tone_Reading mc_tone_sample(void);

#endif /* TONE_H */
