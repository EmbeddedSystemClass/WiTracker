#ifndef TONE_H
#define TONE_H

typedef struct {
    float value1K;
    float value4K;
} Tone_Reading;

extern void mc_tone_init(void);
extern Tone_Reading mc_tone_sample(void);

#endif /* TONE_H */
