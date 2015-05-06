/**
	Project: Arduino+MP3 Musical Bench
	Bozeman Makerspace / Childrens Museum of Bozeman
	Contributors: Daniel Simser
			 	  Rob Lund
			 	  Joe Schwem
	Original Idea: http://www.instructables.com/id/Make-a-Musical-Bench-1/

	Required Libraries:
		SdFat, SFEMP3Shield: https://github.com/madsci1016/Sparkfun-MP3-Player-Shield-Arduino-Library
*/

#include <SPI.h>           // SPI library
#include <SdFat.h>         // SDFat Library
#include <SdFatUtil.h>     // SDFat Util Library
#include <SFEMP3Shield.h>  // Mp3 Shield Library

/* debug enabled/disabled
   (0) disabled
   (1) enabled
*/
int debugMode = 1;

/** Playback Modes
  (1) Play a random track each time the bench is activated
  (2) Play a track based on various analog values (Not complete: 5/1/2015)
*/     
int playbackMode = 1; 

/** Number of mp3 files available
    track001.mp3
    track002.mp3 ... etc */
int availableMp3s = 4;
/** Starting track number. 
	If you only have track005.mp3 and beyond you need to tell the script to only look for tracks on and above that track???
	eg. track001.mp3, track002.mp3, track003.mp3, track004.mp3, track005.mp3 ... */
int mp3StartingTrack = 5;

/** Calibration min/max of analog sensor
    This is only used if playback mode (2)
	TO-DO: once the bench is completed we need to calibrate these settings */
int sensorMin = 20;
int sensorMax = 999;
int sensorStopPlayback = 1000;

/* Set default MAX volumn here
   MP3 Player volume 0=max, 255=lowest (off)
   Defaulting to a bit lower, because the speakers will go to 11 ;) */
const uint8_t volume = 10; 
const uint16_t monoMode = 1;  // Mono setting 0=off, 3=max

/* Pin setups */
int pinTrigger = A0; // Trigger pin on the MP3 Shield which when triggered will start the tracks
int pinTriggerValue; // used to store the analog sensor value (touching hands)

/* Used to smooth analog input */
const int numReadings = 10;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;               // the running total
int average = 0;                // the average

int isPlaying = 0;

/* Setup the required libraries */
SdFat sd;
SFEMP3Shield MP3player;

void setup()
{
  // initialize serial communication with computer:
  if(debugMode = 1) { Serial.begin(9600);}

  // initialize all the readings to 0 (used to smooth the analog sensor)
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  
  initSD();  // Initialize the SD card
  initMP3Player(); // Initialize the MP3 Shield
}

void loop() {
	/* smoothing analog sensor values */
  total = total - readings[readIndex];
  readings[readIndex] = analogRead(pinTrigger);
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  // Our AVERAGE sensor value (hands touching)
  average = total / numReadings;

  if(debugMode = 1) { Serial.print("Average sensor reading:"); Serial.println(average); }
  
  switch (playbackMode) {
    case 1:
       playbackMode_randomTrack(mp3StartingTrack,mp3StartingTrack+availableMp3s);
       break; 
    case 2:
       playbackMode_variable(average);
       break;
    default:
       playbackMode_randomTrack(mp3StartingTrack,mp3StartingTrack+availableMp3s);
  }
  
  delay(10);
}

// initSD() initializes the SD card and checks for an error.
void initSD()
{
  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) 
    sd.initErrorHalt();
  if(!sd.chdir("/")) 
    sd.errorHalt("sd.chdir");
}

// initMP3Player() sets up all of the initialization for the
// MP3 Player Shield. It runs the begin() function, checks
// for errors, applies a patch if found, and sets the volume/
// stero mode.
void initMP3Player()
{
  uint8_t result = MP3player.begin(); // init the mp3 player shield
  if(result != 0) // check result, see readme for error codes.
  {
    // Error checking can go here!
  }
  MP3player.setVolume(volume, volume);
  MP3player.setMonoMode(monoMode);
}

/** 
	Playback Modes
*/
void playbackMode_randomTrack(int minTrack, int maxTrack) {
    Serial.println(MP3player.isPlaying());
  if(average > sensorMin && average < sensorMax) {
      // Play track a random track #minTrack-maxTrack
	  // We need to add +1 to the maxTrack for the random function to do what we want
      // notes: http://www.arduino.cc/en/Reference/random
      if(MP3player.isPlaying() == 0) {
        int myRandomtrack = random(minTrack,maxTrack+1);
        uint8_t result = MP3player.playTrack(myRandomtrack);
        isPlaying=1;
        if(debugMode) { Serial.print("Playback: Track("); Serial.print(myRandomtrack); Serial.println(")"); delay(1); }
      }
   } else {
        MP3player.stopTrack();
        isPlaying=0;
   }

}

void playbackMode_variable(int analogValue) {

  int sensorAnalogSteps = sensorMin+(sensorMax/availableMp3s);
  
  // stop if value above or below thresholds
  if(analogValue < sensorMin || analogValue > sensorStopPlayback) {
    if(debugMode) { Serial.println("Playback: Stopped"); } 
    MP3player.stopTrack(); 
  }
 

  // TO-DO
  /** Finish this ;)
  int i;
  // depending on number of mp3s available, alter min/max per track based on analog input
  for(i=mp3StartingTrack;i<mp3StartingTrack+availableMp3s;i++) {
   
     if(analogValue > sensorMin+(sensorMax/availableMp3s)) {
       if (MP3player.isPlaying() != 1) {
         if(debugMode) { Serial.print("Playback: Track("); Serial.print(i); Serial.println(")"); delay(1); }
       }  
     }
  }
  */ 
}
