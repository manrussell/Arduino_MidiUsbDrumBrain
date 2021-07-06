/*
 * DrumBrain_01.ino
 *
 * Created: 22/6/2021 20:44:00
 * Author: Mark Russell
 *
 * This make a usb midi device where the adc's are used for triggers for midi notes.
 * Original intention was for a drum brain for my PS3 Guitar Hero drum kit.
 * 
 * Built/tested on Arduino Leonardo board AT32u4 chip.
 *
 *
 * ToDo:
 *        *) fix the UART_PRINT_ON/OFF
 *        *) Might be nice to have cli so you could update the midi notes, 
 *       but, it wouldn't be survive reset and is probably over kill
 *        *) Not implemented software gain
 */

/***********************************************************************************
 * Headers
 ***********************************************************************************/
#include "MIDIUSB.h"

/***********************************************************************************
 * Macros
 ***********************************************************************************/
/* 
 * Start of Config 
 */
/* what midiVelocity/ADCValue qualifies as a drum pad strike triggering a midi note. */
#define HIT_THRESHOLD               (32u)

/* Currently restricted by the number of ADC's on the board. */
#define MAX_NUM_OF_PADS             (6u)

/* Enable debug from this peripheral */
#define LED_DEBUG_ENABLE            (false)
#define UART_DEBUG_ENABLE           (false)

/* 
 * End of Config 
 */
#if LED_DEBUG_ENABLE 
#   define LED_HIGH( )              (digitalWrite(ledPinNum, HIGH))
#   define LED_LOW( )               (digitalWrite(ledPinNum, LOW))
#else /* ! LED_DEBUG_ENABLE */
#   define LED_HIGH( )
#   define LED_LOW( )
#endif /* LED_DEBUG_ENABLE */      

#if UART_DEBUG_ENABLE 
/* fix these now macro... */
#   define UART_PRINT_ON( )         do{ Serial.println("on  %s", drumPad[i].padName); }while(0)
#   define UART_PRINT_OFF( )        do{ Serial.println("off %s",drumPad[i].padName); }while(0)
#else /* ! UART_DEBUG_ENABLE */
#   define UART_PRINT_ON( )
#   define UART_PRINT_OFF( )
#endif /* UART_DEBUG_ENABLE */  

#define MIDI_NOTE_ON_MESSAGE        (0x09)
#define MIDI_NOTE_OFF_MESSAGE       (0x08)
#define MIDI_CONTROL_CHANGE_MESSAGE (0xB)

/* Midi range is 7 bit (0-127) */
#define MAX_127( val_ )             (val_ & 0x7F)

/***********************************************************************************
 * typeDefs
 ***********************************************************************************/
typedef struct
{
    uint8_t midiNoteChannel;
    uint8_t midiNoteOnVal;
    uint16_t adcNum;
    uint8_t midiVelocity;
    const char *padName;
    bool padHit;
} triggerPad_t;

/*********************************************************************************** 
 * Globals 
 ***********************************************************************************/
const int ledPinNum                 = 13u;
int softwareGainMultiplyer          = 0u; /* not implemented */

/* set up kit pads*/
triggerPad_t drumPad[] = \
{  
    {  
        drumPad[0].midiNoteChannel = 10u,
        drumPad[0].midiNoteOnVal   = 35u,
        drumPad[0].adcNum          = A0,
        drumPad[0].midiVelocity    = 0u,
        drumPad[0].padName         = "KICK",
        drumPad[0].padHit          = false,
    },
    {
        drumPad[1].midiNoteChannel = 10u,
        drumPad[1].midiNoteOnVal   = 38u,    /* snare */
        drumPad[1].adcNum          = A1,
        drumPad[1].midiVelocity    = 0u,
        drumPad[1].padName         = "L_TOM",  
        drumPad[1].padHit          = false,
    },
    {
        drumPad[2].midiNoteChannel = 10u,
        drumPad[2].midiNoteOnVal   = 48u,    /* high-mid tom */
        drumPad[2].adcNum          = A2,
        drumPad[2].midiVelocity    = 0u,
        drumPad[2].padName         = "M_TOM",
        drumPad[2].padHit          = false,
    },
    {
        drumPad[3].midiNoteChannel = 10u,
        drumPad[3].midiNoteOnVal   = 50u,    /*  high tom */
        drumPad[3].adcNum          = A3,
        drumPad[3].midiVelocity    = 0u,
        drumPad[3].padName         = "R_TOM",
        drumPad[3].padHit          = false,
    },
    {
        drumPad[4].midiNoteChannel = 10u,
        drumPad[4].midiNoteOnVal   = 42u,    /*  closed HH */
        drumPad[4].adcNum          = A4,
        drumPad[4].midiVelocity    = 0u,
        drumPad[4].padName         = "L_HH",
        drumPad[4].padHit          = false,
    },
    {
        drumPad[5].midiNoteChannel = 10u,
        drumPad[5].midiNoteOnVal   = 49u,    /* crash cymbal */
        drumPad[5].adcNum          = A5,
        drumPad[5].midiVelocity    = 0u,
        drumPad[5].padName         = "R_HH",
        drumPad[5].padHit          = false,
    },
};

/***********************************************************************************
 * Callbacks from USBMIDI 
 ***********************************************************************************/
// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).
void noteOn(byte channel, byte pitch, byte velocity) 
{
    midiEventPacket_t noteOn = {MIDI_NOTE_ON_MESSAGE, uint8_t(0x90 | channel), pitch, velocity};
    MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) 
{
    midiEventPacket_t noteOff = {MIDI_NOTE_OFF_MESSAGE, uint8_t(0x80 | channel), pitch, velocity};
    MidiUSB.sendMIDI(noteOff);
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).
void controlChange(byte channel, byte control, byte value) 
{
    midiEventPacket_t event = {MIDI_CONTROL_CHANGE_MESSAGE, uint8_t(0xB0 | channel), control, value};
    MidiUSB.sendMIDI(event);
}

void setup() 
{
#if LED_DEBUG_ENABLE     
    pinMode( ledPinNum, OUTPUT );
    digitalWrite(ledPinNum, LOW);
#endif /* LED_DEBUG_ENABLE */

#if UART_DEBUG_ENABLE     
    Serial.begin(115200);
#endif /* UART_DEBUG_ENABLE */
}

void loop( void ) 
{
    const uint8_t hitThreshold  = HIT_THRESHOLD;
    const uint8_t numOfDrumPads = MAX_NUM_OF_PADS;
    static uint8_t i = 0u;

    /* ADC is 10 bit, Midi range is 7 bit (0-127) do shift here, or truncate when about ot send */
    drumPad[i].midiVelocity = analogRead( drumPad[i].adcNum );

    if( (hitThreshold <= drumPad[i].midiVelocity) && (false == drumPad[i].padHit) )
    {
        noteOn( drumPad[i].midiNoteChannel, drumPad[i].midiNoteOnVal, MAX_127(drumPad[i].midiVelocity) );
        MidiUSB.flush();
        drumPad[i].padHit = true;
        LED_HIGH();
        UART_PRINT_ON();
    }
    else if( (hitThreshold > drumPad[i].midiVelocity) && (true == drumPad[i].padHit) )
    {
        noteOff( drumPad[i].midiNoteChannel, drumPad[i].midiNoteOnVal, /*Velocity*/0);  // velocity '0' -don't know if that actually matters? 
        MidiUSB.flush();
        drumPad[i].padHit = false;
        LED_LOW();
        UART_PRINT_OFF();
    }
    else
    {
        //nothing to do!
    }

    if( numOfDrumPads <= ++i ) i = 0;
    
    delay(1); // short delay for adc to recover, probably don't need now using more than 1 ADC...
}
