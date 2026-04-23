#include "Seeed_Arduino_GroveAI.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Xmin:26	Xmax:185	Ymin:36	Ymax:181

const int SCREEN_WIDTH = 128;  // OLED display width, in pixels
const int SCREEN_HEIGHT = 64;  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 left(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SSD1306 right(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


int xMin = 512;
int xMax = 0;
int yMin = 512;
int yMax = 0;

int pupilX = 64;
int pupilY = 32;



GroveAI ai(Wire);
uint8_t state = 0;
unsigned long faceDetectTimestamp = 0;
void setup() {
  Wire.begin();
  Serial.begin(115200);

  Serial.print("Running Setup...");
  if (ai.begin(ALGO_OBJECT_DETECTION, (MODEL_INDEX_T)0x11))  // Object detection and pre-trained model 1
  {
    state = 1;
  } else {
    Serial.println("Algo begin failed.");
  }

  if (!left.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 \"left\" allocation failed"));
    while (true)
      ;
  }
  if (!right.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 \"right\" allocation failed"));
    while (true)
      ;
  }

  left.setRotation(2);  //rotates text on OLED 1=90 degrees, 2=180 degrees
  right.setRotation(2);

  left.clearDisplay();
  right.clearDisplay();

  left.setTextSize(1);
  left.setTextColor(WHITE);
  left.setCursor(0, 10);
  right.setTextSize(1);
  right.setTextColor(WHITE);
  right.setCursor(0, 10);
  // Display static text
  left.println("Hello");
  left.display();
  right.println("World!");
  right.display();

  Serial.println(" Done!");
  Serial.println("Beginning loop...");
}

void loop() {
  if (state == 1) {
    uint32_t tick = millis();
    if (ai.invoke())  // begin invoke
    {
      while (1)  // wait for invoking finished
      {
        CMD_STATE_T ret = ai.state();

        //Loop code here:
        if (millis() - faceDetectTimestamp > 10000) {  //It's been ten seconds since a face was detected.
          Serial.print('.');                           //Fall asleep
          clear();
        }

        //TODO: Blink at random



        if (ret == CMD_STATE_IDLE) {
          break;
        }
        delay(20);
      }

      uint8_t len = ai.get_result_len();  // receive how many people detect
      if (len) {
        // int time1 = millis() - tick;
        // Serial.println();
        object_detection_t data;  //get data

        for (int i = 0; i < len; i++) {
          //          Serial.println("result:detected");
          //          Serial.print("Detecting and calculating: ");
          //          Serial.println(i+1);
          ai.get_result(i, (uint8_t*)&data, sizeof(object_detection_t));  //get result
          if (data.confidence < 70) continue;                             //Skip any detected faces with lower confidence
          faceDetectTimestamp = millis();
          /*
          Serial.print("confidence:");
          Serial.print(data.confidence);
          //CENTER COORDINATES
          Serial.print("\tX:");
          Serial.print(data.x);

          Serial.print("\tY:");
          Serial.print(data.y);
          //Serial.print("\tW:");
          //Serial.print(data.w);
          //Serial.print("\tH:");
          //Serial.print(data.h);
          */
          //Refresh min/max values:
          if (data.x < xMin) xMin = data.x;
          if (data.y < yMin) yMin = data.y;
          if (data.x > xMax) xMax = data.x;
          if (data.y > yMax) yMax = data.y;

          pupilX = map(data.x, xMin, xMax, 120, 8);
          pupilY = map(data.y, yMin, yMax, 8, 56);
          clear();
          eyes(pupilX, pupilY);
          display();

          Serial.print("\tXmin:");
          Serial.print(xMin);
          Serial.print("\tXmax:");
          Serial.print(xMax);
          Serial.print("\tYmin:");
          Serial.print(yMin);
          Serial.print("\tYmax:");
          Serial.print(yMax);
          Serial.print("\tXin:");
          Serial.print(data.x);
          Serial.print("\tYin:");
          Serial.print(data.y);
          Serial.print("\tX:");
          Serial.print(pupilX);
          Serial.print("\tY:");
          Serial.print(pupilY);


          Serial.println();
        }
      } else {
        //       Serial.print(".");
      }
    } else {
      delay(500);
      Serial.println("Invoke Failed.");
    }
  } else {
    state == 0;
  }
}

void clear() {
  left.clearDisplay();
  right.clearDisplay();
}
void eyes(int x, int y) {
  left.fillCircle(x, y, 6, 1);
  right.fillCircle(x, y, 6, 1);
}

void display() {
  left.display();
  right.display();
}