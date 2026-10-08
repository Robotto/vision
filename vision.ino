//code snippets from URL: 

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


int xMin = 55;
int xMax = 170;
int yMin = 91;
int yMax =149;

int pupilX = 64;
int pupilY = 32;

/*
          pupilX = map(data.x, xMin, xMax, 120, 8);
          pupilY = map(data.y, yMin, yMax, 8, 56);
*/
const int pupilMaxX = 8;
int pupilMinX = 120;
const int pupilMaxY = 8;
int pupilMinY = 56;



GroveAI ai(Wire);
uint8_t state = 0;
unsigned long faceDetectTimestamp = 0;
void setup() {
  Wire.begin();
  Serial.begin(115200);

  Serial.print("Running Setup...");


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

  if (ai.begin(ALGO_OBJECT_DETECTION, (MODEL_INDEX_T)0x11))  // Object detection and pre-trained model 1
  {

    Serial.print("Version: ");
    Serial.println(ai.version());
    Serial.print("ID: ");
    Serial.println( ai.id());
    Serial.print("Algo: ");
    Serial.println( ai.algo());
    Serial.print("Model: ");
    Serial.println(ai.model());
    Serial.print("Confidence: ");
    Serial.println(ai.confidence());

    state = 1;
  } else {
    Serial.println("Algo begin failed.");
    left.println("ALGO FAILED");
    
        
    right.println("Have you tried turning it off and on again?");
    display();

  }

  // Display static text
  left.println("Hello");
  left.display();
  right.println("World!");
  right.display();

  Serial.println(" Done!");
  delay(2000);
  clear();
  outlines();
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
          //clear();
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
          if (data.confidence < 80) continue;                             //Skip any detected faces with lower confidence
          unsigned long detectionTime = millis()-faceDetectTimestamp;
          faceDetectTimestamp = millis();
          
          Serial.print("confidence:");
          Serial.print(data.confidence);
          Serial.print("\tdT:");
          Serial.print(detectionTime);
          //CENTER COORDINATES
          Serial.print("\tX:");
          Serial.print(data.x);

          Serial.print("\tY:");
          Serial.print(data.y);
          //Serial.print("\tW:");
          //Serial.print(data.w);
          //Serial.print("\tH:");
          //Serial.print(data.h);
          
          //Refresh min/max values:
          if (data.x < xMin) xMin = data.x;
          if (data.y < yMin) yMin = data.y;
          if (data.x > xMax) xMax = data.x;
          if (data.y > yMax) yMax = data.y;

          pupilX = map(data.x, xMin, xMax, pupilMinX, pupilMaxX);
          pupilY = map(data.y, yMin, yMax, pupilMinY, pupilMaxY);
          //clear();
          eyes(pupilX, pupilY);
          display();
/*
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
*/
          Serial.print("\tOLEDx:");
          Serial.print(pupilX);
          Serial.print("\tOLEDy:");
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
  static int oldX,oldY;
  left.fillCircle(oldX, oldY, 6, 0);
  right.fillCircle(oldX, oldY, 6, 0);
  
  left.fillCircle(x, y, 6, 1);
  right.fillCircle(x, y, 6, 1);

  oldX=x;
  oldY=y;

}

void outlines(){
    const int centerX=64;
    const int fatness=3;

    for(int i=centerX;i>centerX-fatness;i--){ //use a for loop to draw fatter ellipses...
    //left.drawEllipse(64, 32, 64, 32,1);
    //right.drawEllipse(64, 32, 64, 32,1);
    left.drawEllipse(64, 32, i, i/2,1);
    right.drawEllipse(64, 32, i, i/2,1);
    }
    
    
  
}

void display() {
  left.display();
  right.display();
}