#define LIN_OUT 1 // use the log output function
#define FHT_N 256 // set to 256 point fht
#define SCALE 256

#include "FHT.h"

unsigned long start, finished, elapsed;
int FHT_counter = 0, eye_down_counter = 0, eye_up_counter = -1, lock_flag = 1;
//int global_max_freq;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

int blinkings = 0;
int up=0, down=0;

// the loop routine runs over and over again forever:
void loop() {
  int FHT_flag = 1;
  int sensorValue;
  /*SAMPLING*/
  for(int i=0; i<FHT_N; i++)
  {
    // read the input on analog pin 0:
    sensorValue = analogRead(A0);
    fht_input[i] = sensorValue;
  }
  
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (5.0 / 1023.0);
  // print out the value you read:
  //Serial.println(voltage);
  
  if (voltage > 4.3) {
    up = 1;
    start = micros();
  }
  if (up) {
    if (voltage < 4.3) {
      down = 1;
      finished = micros();
      elapsed = finished - start;
      //Serial.println(elapsed);
    }
  }


  
  // SIGNAL PROCESSING - BLINKING CLASSIFICATION

  // START OF FHT PROCESSING
  
  // We activate FHT signal processing as our main method for blinking classification.

    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_lin(); // take the output of the fht
    //Serial.write(255); // send a start byte
    int max_freq = fht_lin_out[0];
    int max_freq_pos = 0;
    for (byte i = 0 ; i < FHT_N/2 ; i++) {
        if (fht_lin_out[i] > max_freq) {
          max_freq = fht_lin_out[i];
          max_freq_pos = i; // This i corresponds to a frequency (in Hz) of : i*9615/FHT_N, where at almost 9.6 kHz (9615 Hz) is the conversion sampling frequency of Arduino Uno ADC.
        }
    }

    //global_max_freq = max_freq;
    
    //Serial.println(max_freq);
    //Serial.println(max_freq_pos);

    if (max_freq <= 135) {
      lock_flag = 1;
    }
    if (max_freq >= 140 && max_freq < 460 && lock_flag) { // These values can differ, depending on the proper placement of the electrodes and the use of the conductive gel.
      FHT_counter++;                                      // We don't want false positives by simply looking down -> this produces desirable max_freq, but is not a blinking.
      if (FHT_counter > 8) {
        FHT_counter = 0;
        lock_flag = 0;
      }
    }
    if (max_freq >= 140 && max_freq <= 250 && lock_flag) {  // Extra looking down measures/restrictions.
      eye_down_counter++;
      if (eye_down_counter > 7) {
        eye_down_counter = 0;
        lock_flag = 0;
      }
    }
    if (max_freq >= 140 && max_freq <= 250 && lock_flag) {  // Extra looking down measures/restrictions.
      eye_up_counter++;
      if (eye_up_counter > 5) {
        eye_up_counter = -1;
        lock_flag = 0;
      }
    }
    
//    Serial.print("FHT_counter:");
//    Serial.println(FHT_counter);
//    Serial.print("eye_down_counter:");
//    Serial.println(eye_down_counter);
//    Serial.print("eye_up_counter:");
//    Serial.println(eye_up_counter);
  
    if ((max_freq < 130) && (FHT_counter>0) && (eye_down_counter>0) && (eye_up_counter>-1)) {  // Below 110, we consider it proper to check now for blinkings -> quiescent mode.
      blinkings++;
      up = 0;
      down = 0;
      FHT_counter = 0;
      eye_down_counter = 0;
      eye_up_counter = -1;
      FHT_flag = 0;
      //Serial.println('a');
      Serial.println(blinkings);
    }
  
  // END OF FHT PROCESSING

  

  // If the blinking is not already recognized with the FHT analysis, then, for strong blinkings, we use a secondary classification method, using the up, down and elapsed variables.
  if (FHT_flag && up && down && (elapsed>=52000 && elapsed<=54000)) {
   // if (max_freq > 450 && max_freq < 600) {
      blinkings++;
      up = 0;
      down = 0;
      FHT_counter = 0;
      eye_down_counter = 0;
      eye_up_counter = -1;
      //Serial.println('a');
      Serial.println(blinkings);
    //}
  }
  
  delay(20);
}

