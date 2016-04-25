#include "Auduino.h"

// constructor
Auduino::Auduino() {
    // setup()
    pinMode(PWM_PIN,OUTPUT);
    audioOn();
    pinMode(LED_PIN,OUTPUT);

    // init private members
    // Smooth logarithmic mapping
    uint16_t antilogTable[64] = {
        64830,64132,63441,62757,62081,61413,60751,60097,59449,58809,58176,57549,56929,56316,55709,55109,
        54515,53928,53347,52773,52204,51642,51085,50535,49991,49452,48920,48393,47871,47356,46846,46341,
        45842,45348,44859,44376,43898,43425,42958,42495,42037,41584,41136,40693,40255,39821,39392,38968,
        38548,38133,37722,37316,36914,36516,36123,35734,35349,34968,34591,34219,33850,33486,33125,32768
    };

    // Stepped chromatic mapping
    uint16_t midiTable[128] = {
        17,18,19,20,22,23,24,26,27,29,31,32,34,36,38,41,43,46,48,51,54,58,61,65,69,73,
        77,82,86,92,97,103,109,115,122,129,137,145,154,163,173,183,194,206,218,231,
        244,259,274,291,308,326,346,366,388,411,435,461,489,518,549,581,616,652,691,
        732,776,822,871,923,978,1036,1097,1163,1232,1305,1383,1465,1552,1644,1742,
        1845,1955,2071,2195,2325,2463,2610,2765,2930,3104,3288,3484,3691,3910,4143,
        4389,4650,4927,5220,5530,5859,6207,6577,6968,7382,7821,8286,8779,9301,9854,
        10440,11060,11718,12415,13153,13935,14764,15642,16572,17557,18601,19708,20879,
        22121,23436,24830,26306
    };

    // Stepped Pentatonic mapping
    uint16_t pentatonicTable[54] = {
        0,19,22,26,29,32,38,43,51,58,65,77,86,103,115,129,154,173,206,231,259,308,346,
        411,461,518,616,691,822,923,1036,1232,1383,1644,1845,2071,2463,2765,3288,
        3691,4143,4927,5530,6577,7382,8286,9854,11060,13153,14764,16572,19708,22121,26306
    };
}

// destructor
Auduino::~Auduino() {

}

uint16_t Auduino::mapPhaseInc(uint16_t input) {
    return (antilogTable[input & 0x3f]) >> (input >> 6);
}


uint16_t Auduino::mapMidi(uint16_t input) {
    return (midiTable[(1023-input) >> 3]);
}

uint16_t Auduino::mapPentatonic(uint16_t input) {
    uint8_t value = (1023-input) / (1024/53);
    return (pentatonicTable[value]);
}

void Auduino::audioOn() {
#if defined(__AVR_ATmega8__)
    // ATmega8 has different registers
    TCCR2 = _BV(WGM20) | _BV(COM21) | _BV(CS20);
    TIMSK = _BV(TOIE2);
#elif defined(__AVR_ATmega1280__)
    TCCR3A = _BV(COM3C1) | _BV(WGM30);
    TCCR3B = _BV(CS30);
    TIMSK3 = _BV(TOIE3);
#else
    // Set up PWM to 31.25kHz, phase accurate
    TCCR2A = _BV(COM2B1) | _BV(WGM20);
    TCCR2B = _BV(CS20);
    TIMSK2 = _BV(TOIE2);
#endif
}

// call in loop()
void Auduino::update() {
    // loop()

    // The loop is pretty simple - it just updates the parameters for the oscillators.
    //
    // Avoid using any functions that make extensive use of interrupts, or turn interrupts off.
    // They will cause clicks and poops in the audio.

    // Smooth frequency mapping
    //syncPhaseInc = mapPhaseInc(analogRead(SYNC_CONTROL)) / 4;

    // Stepped mapping to MIDI notes: C, Db, D, Eb, E, F...
    //syncPhaseInc = mapMidi(analogRead(SYNC_CONTROL));

    // Stepped pentatonic mapping: D, E, G, A, B
    syncPhaseInc = mapPentatonic(analogRead(SYNC_CONTROL));

    grainPhaseInc  = mapPhaseInc(analogRead(GRAIN_FREQ_CONTROL)) / 2;
    grainDecay     = analogRead(GRAIN_DECAY_CONTROL) / 8;
    grain2PhaseInc = mapPhaseInc(analogRead(GRAIN2_FREQ_CONTROL)) / 2;
    grain2Decay    = analogRead(GRAIN2_DECAY_CONTROL) / 4;
}
