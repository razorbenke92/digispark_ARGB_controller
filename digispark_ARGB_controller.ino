// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN   4    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

#define DEBUG_PIN    1 //DebugPIN - used only on Digispark - else set to P5 which is RESET pin anyway
#define PIXEL_PIN    0    // Digital IO pin connected to the NeoPixels.
#define MODE_COUNT   16
#define EFFECT_COUNT 15 //Update when adding new effects
#define COLOR_COUNT 20 //Update when adding new colors

byte pixelCount = 18;
byte selectedMode = 0;
byte state = 0; // 0 - off | 1 - on | 2 - setEffect | 3 - setColor | 4 - setBackColor | 5 - setSpeed | 6 - setPixelCount |

int stepcounter = 0;

struct ModePresets 
{
  byte effect;
  byte color;
  byte backcolor;
  byte Speed;
};



ModePresets mode[MODE_COUNT];

/*
byte modeEffect[MODE_COUNT];
byte modeColor[MODE_COUNT];
byte modeSpeed[MODE_COUNT];
*/

Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixelCount, PIXEL_PIN, NEO_GRB + NEO_KHZ800);;

bool oldState = HIGH;
unsigned long presscount = 0;
unsigned long timer = millis();

const uint32_t presetColor[COLOR_COUNT] = {0,0x0000ff,0x7fff,0x00ffff,0xff8c00,0xffd700,0x008000,0x4b0082,0x00ff00, 0xff00ff,0x00fa9a,0xffa500,0xff4500,0x800080,0xff0000,0x00ff7f,0x008080,0xffffff,0xffff00,0x9acd32};


void setup() 
{
  load();
  strip = Adafruit_NeoPixel(pixelCount, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(DEBUG_PIN, OUTPUT);
  
    //DEBUG
#ifdef DEBUG  
    digitalWrite(DEBUG_PIN,1);
    delay(500);
    digitalWrite(DEBUG_PIN,0);
    delay(500); 
    digitalWrite(DEBUG_PIN,1);
    delay(500);
    digitalWrite(DEBUG_PIN,0);  
    delay(500); 
    digitalWrite(DEBUG_PIN,1);
    delay(5000);
    digitalWrite(DEBUG_PIN,0);    
#endif

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  colorWipe(strip.Color(255, 255, 255), 50);
  colorSwipe(strip.Color(0, 0, 0), 25);
  state = 0;
}
void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop() 
{
  bool newState = digitalRead(BUTTON_PIN);

  if (newState == HIGH && oldState == LOW)
  {
    delay(20);
    newState = digitalRead(BUTTON_PIN);
    if (newState == HIGH) 
    {      
      oldState = newState;
      //really released
      if(millis()-presscount > 1000)
      {
        //it was a long press
        longPress();
      }
    }
  }
  else if (newState == HIGH && oldState == HIGH)
  {
    delay(20);
    newState = digitalRead(BUTTON_PIN);
    if(presscount > 0 && newState == HIGH)
    {      
      oldState = newState;
      if(millis()-presscount > 500) //there were no shortpresses for some time
      {
        presscount = 0; //making sure this stat does not come up i the next round
        singlePress();
      }
    }
    else
    {
      //the last shortpress was already handled
      //do the LED magic
      startShow();
    }
  }  
  else if (newState == LOW && oldState == HIGH) 
  {
    // Short delay to debounce button.
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if (newState == LOW) 
    {
      if(millis()-presscount > 500) //there has been no presses in the near past
      {      
        presscount = millis(); 
      }
      else //this is the second press for a while
      {
        presscount = 0; //(preventing re-occurence)
        doublePress();// doubleclick happened
      }
      oldState = newState;
    }
  } 
}

void singlePress()
{
  //DEBUG
#ifdef DEBUG  
    digitalWrite(DEBUG_PIN,1);
    delay(500);
    digitalWrite(DEBUG_PIN,0);    
#endif
  if(state == 0) //sleep
  {
    state = 1;
    stepcounter = 0;
  }
  else if(state == 1) //runmode
  {    
    selectedMode++;
    if(selectedMode == MODE_COUNT) selectedMode = 0;
    stepcounter = 0;
  }
  else if(state == 2) //setEffect
  {
    mode[selectedMode].effect++;
    if(mode[selectedMode].effect == EFFECT_COUNT) mode[selectedMode].effect=0;
    stepcounter = 0;
  }
  else if(state == 3) //setColor
  {
    mode[selectedMode].color++;
    if(mode[selectedMode].color == COLOR_COUNT) mode[selectedMode].color=0;
    stepcounter = 0;
  }
  else if(state == 4) //setBackColor
  {
    if(mode[selectedMode].backcolor == 255) mode[selectedMode].backcolor = 0;
    else mode[selectedMode].backcolor++;
    stepcounter = 0;
  }
  else if(state == 5) //setSpeed
  {
    if(mode[selectedMode].Speed == 255) mode[selectedMode].Speed = 0;
    else mode[selectedMode].Speed++;
    stepcounter = 0;
  }
  else if(state == 6)
  {
    //handle pixel renumbering
    stepcounter++;
  }  
}

void doublePress()
{
    //DEBUG
#ifdef DEBUG
    digitalWrite(DEBUG_PIN,1);
    delay(500);
    digitalWrite(DEBUG_PIN,0);
    delay(500); 
    digitalWrite(DEBUG_PIN,1);
    delay(500);
    digitalWrite(DEBUG_PIN,0);   
#endif
  if(state == 5) //setSpeed
  {
    if(mode[selectedMode].Speed == 255) mode[selectedMode].Speed = 0;
    if(mode[selectedMode].Speed > 235) mode[selectedMode].Speed = 255;
    else mode[selectedMode].Speed+=20;
    stepcounter = 0;
  }
  else if(state == 1)
  {
    mode[selectedMode].color++;
    if(mode[selectedMode].color == COLOR_COUNT) mode[selectedMode].color=0;
    stepcounter = 0;
  }
  else if(state == 3) //setColor
  {
    if(mode[selectedMode].color == 0) mode[selectedMode].color=COLOR_COUNT;
    else mode[selectedMode].color--;
    stepcounter = 0;
  }  
  else if(state == 2) //setEffect
  {    
    if(mode[selectedMode].effect == 0) mode[selectedMode].effect=EFFECT_COUNT;
    mode[selectedMode].effect--;
    stepcounter = 0;
  }
  else if(state == 4) //setbackColor
  {
    if(mode[selectedMode].backcolor == 0) mode[selectedMode].backcolor=COLOR_COUNT;
    else mode[selectedMode].backcolor--;
    stepcounter = 0;
  }  
  else if(state == 6)
  {
    //handle pixel renumbering
    stepcounter--;
  }  
}

void longPress()
{
    //DEBUG
#ifdef DEBUG
    digitalWrite(DEBUG_PIN,1);
    delay(2000);
    digitalWrite(DEBUG_PIN,0);   
#endif
  if(millis() < 10000)
  {
    state = 6;
  }
  else
  {
    if(state == 1)
    {
      state = 0;      
      colorSwipe(strip.Color(0, 0, 0), 15);
    }
    else if(state == 0)
    {
      state = 2;
    }
    else if(state == 2)
    {
      state = 3;
    }
    else if(state == 3)
    {
      state = 4;
    }
    else if(state == 4)
    {
      state = 5;
    }
    else if(state == 5)
    {
      save();
      state = 1;
    }
    else if(state == 6)
    {
      pixelCount = stepcounter;      
      save();
      resetFunc();  //call reset
    }
  }
}

void load()
{
  //EEPROM.begin(512);
  delay(10);
  if(EEPROM.read(0) == B10101010)
  {
    for(int i=0; i < MODE_COUNT; i++)
    {
      mode[i].effect = EEPROM.read(i+1);
      mode[i].color = EEPROM.read(i+MODE_COUNT+1);
      mode[i].backcolor = EEPROM.read(i+MODE_COUNT+MODE_COUNT+1);      
      mode[i].Speed = EEPROM.read(i+MODE_COUNT+MODE_COUNT+MODE_COUNT+1);
    }
    pixelCount = EEPROM.read(MODE_COUNT+MODE_COUNT+MODE_COUNT+MODE_COUNT+1);
  }
  else
  {
    for(int i=0; i < MODE_COUNT; i++)
    {
      mode[i].effect = i % EFFECT_COUNT;
      mode[i].color = i % (COLOR_COUNT-1) + 1;
      mode[i].backcolor = 0;
      mode[i].Speed = 0;
    }
    pixelCount = 18;
  }  
  EEPROM.end();
}

void save()
{
  //EEPROM.begin(512);
  EEPROM.write(0,B10101010);
  for(int i=0; i < MODE_COUNT; i++)
    {
      EEPROM.write(i+1,mode[i].effect);
      EEPROM.write(i+MODE_COUNT+1,mode[i].color);
      EEPROM.write(i+MODE_COUNT+MODE_COUNT+1,mode[i].backcolor);
      EEPROM.write(i+MODE_COUNT+MODE_COUNT+MODE_COUNT+1,mode[i].Speed);
    }
  EEPROM.write(MODE_COUNT+MODE_COUNT+MODE_COUNT+MODE_COUNT+1,pixelCount);
  //EEPROM.commit();
  EEPROM.end();
}

void startShow() 
{
  if(state == 0)
  {
    //sleeps
    delay(10);
  }
  
  else if(state !=6)
  {
    spark();
    /*switch(mode[selectedMode].effect)
    {
      case 0: solidFill();
              break;
      case 1: fadeFill();  
              break;
      case 2: patrol();  
              break;
      case 3: tetris();  
              break;
      case 4: heartbeat();  
              break;
      case 5: theatherRunning();  
              break;
      case 6: fountain();  
              break;
      case 7: spark();  
              break;
      case 8: fadeFlow(); //two colors fading across and moving
              break;
      case 9: fadeFlowRainbow(); 
              break;
      case 10: fountainRainbow();
              break;
      case 11: heartbeatRainbow(); 
              break;
      case 12: tetrisRainbow();
              break;
      case 13: patrolRainbow();
              break;
      case 14: fadeFillRainbow();
              break;
      default:break;
      
    }*/
  }
  else
  {
    colorFull(strip.Color(0,0,0));
    strip.setPixelColor(stepcounter, strip.Color(128,128,128));
    strip.show();
  }
  
}
//####################################################################################################################################
void solidFill()
{
  if(strip.getPixelColor(0) == presetColor[mode[selectedMode].color])
  {
    //not much to do
    delay(5);
  }
  else
  {
    colorFill(presetColor[mode[selectedMode].color]);
    //colorFill(0);
  }
}

//####################################################################################################################################

void fadeFill()
{
  if(millis() < timer) timer = millis();
  if(millis() - timer > (mode[selectedMode].Speed*2)+10)
  {
    timer = millis();
    uint32_t modifiedBackcolor = PresetColor(mode[selectedMode].backcolor);
    if(mode[selectedMode].backcolor == mode[selectedMode].color) modifiedBackcolor = Dim(modifiedBackcolor,60);
    //determine how many steps are required
    int fadestepsRed = abs(Red(PresetColor(mode[selectedMode].color)) - Red(modifiedBackcolor));
    int fadestepsGreen = abs(Green(PresetColor(mode[selectedMode].color)) - Green(modifiedBackcolor));
    int fadestepsBlue = abs(Blue(PresetColor(mode[selectedMode].color)) - Blue(modifiedBackcolor));
    int fadesteps = max(fadestepsRed,max(fadestepsGreen,fadestepsBlue));
  
    
    uint32_t current = strip.getPixelColor(0);
  
    if(stepcounter == 0)
    {
      colorFill(PresetColor(mode[selectedMode].color));
      stepcounter++;
    }
    else if(stepcounter == fadesteps)
    {
      colorFill(modifiedBackcolor);
      stepcounter++;
    }
    else if(stepcounter >= fadesteps*2)
    {
      stepcounter = 0; 
    }
    else if(stepcounter < fadesteps) //going from color->backcolor
    {
      if(Red(current) != Red(modifiedBackcolor)) //red still goes
      {
        if(Red(PresetColor(mode[selectedMode].color)) - Red(modifiedBackcolor) > 0)
        {
          colorFill(strip.Color(Red(current)-1,Green(current),Blue(current)));
        }
        else
        {
          colorFill(strip.Color(Red(current)+1,Green(current),Blue(current)));
        }
      }
      if(Green(current) != Green(modifiedBackcolor)) //green still goes
      {
        if(Green(PresetColor(mode[selectedMode].color)) - Green(modifiedBackcolor) > 0)
        {
          colorFill(strip.Color(Red(current),Green(current)-1,Blue(current)));
        }
        else
        {
          colorFill(strip.Color(Red(current),Green(current)+1,Blue(current)));
        }
      }
      if(Blue(current) != Blue(modifiedBackcolor)) //Blue still goes
      {
        if(Blue(PresetColor(mode[selectedMode].color)) - Blue(modifiedBackcolor) > 0)
        {
          colorFill(strip.Color(Red(current),Green(current),Blue(current)-1));
        }
        else
        {
          colorFill(strip.Color(Red(current),Green(current),Blue(current)+1));
        }
      }
      stepcounter++;
    }
    else if(stepcounter > fadesteps) //going from backcolor->color
    {
      if(Red(current) != Red(PresetColor(mode[selectedMode].color))) //red still goes
      {
        if(Red(PresetColor(mode[selectedMode].color)) - Red(modifiedBackcolor) > 0)
        {
          colorFill(strip.Color(Red(current)+1,Green(current),Blue(current)));
        }
        else
        {
          colorFill(strip.Color(Red(current)-1,Green(current),Blue(current)));
        }
      }
      if(Green(current) != Green(PresetColor(mode[selectedMode].color))) //green still goes
      {
        if(Green(PresetColor(mode[selectedMode].color)) - Green(modifiedBackcolor) > 0)
        {
          colorFill(strip.Color(Red(current),Green(current)+1,Blue(current)));
        }
        else
        {
          colorFill(strip.Color(Red(current),Green(current)-1,Blue(current)));
        }
      }
      if(Blue(current) != Blue(PresetColor(mode[selectedMode].color))) //Blue still goes
      {
        if(Blue(PresetColor(mode[selectedMode].color)) - Blue(modifiedBackcolor) > 0)
        {
          colorFill(strip.Color(Red(current),Green(current),Blue(current)+1));
        }
        else
        {
          colorFill(strip.Color(Red(current),Green(current),Blue(current)-1));
        }
      }
      stepcounter++;   
    }
    strip.show();
  }  
}
//####################################################################################################################################
void patrol()
{
  if(millis() < timer) timer = millis();
  if(millis() - timer > (mode[selectedMode].Speed*2)+10)
  {
    timer = millis();
    uint32_t modifiedBackcolor = PresetColor(mode[selectedMode].backcolor);
    if(mode[selectedMode].backcolor == mode[selectedMode].color) modifiedBackcolor = Dim(modifiedBackcolor,60);
    
    if(stepcounter < pixelCount) //going in positive direction, train to negative
    {
      colorFull(modifiedBackcolor);
      for(int i=0; i<6; i++)
      {
        if(stepcounter - i >=0)
        {
          strip.setPixelColor(stepcounter - i, Dim(PresetColor(mode[selectedMode].color),100-i*20));
        }
      }
    }
    if(stepcounter == pixelCount)
    {
      colorFull(modifiedBackcolor);
      strip.setPixelColor(pixelCount-1, PresetColor(mode[selectedMode].color));
    }
    if(stepcounter > pixelCount) //going in negative direction, train to positive
    {
      colorFull(modifiedBackcolor);
      for(int i=0; i<6; i++)
      {
        if(stepcounter - pixelCount - i > 0)
        {
          strip.setPixelColor(pixelCount + pixelCount - stepcounter + i, Dim(PresetColor(mode[selectedMode].color),100-i*20));
        }
      }
    }
    strip.show();
    stepcounter++;
    if(stepcounter > 2*pixelCount) stepcounter = 0;
  }
}
//####################################################################################################################################
void tetris()
{
  if(millis() < timer) timer = millis();
  if(millis() - timer > (mode[selectedMode].Speed*4)+100)
  {
    timer = millis();
    uint32_t modifiedBackcolor = PresetColor(mode[selectedMode].backcolor);
    if(mode[selectedMode].backcolor == mode[selectedMode].color) modifiedBackcolor = Dim(modifiedBackcolor,60);
    
    if(stepcounter == 0)
    {
      colorFill(modifiedBackcolor);
      stepcounter++;
    }
    else
    {
      byte lastpixel = pixelCount;
      for(int i=0;i<pixelCount;i++)
      {
        if(strip.getPixelColor(i) == PresetColor(mode[selectedMode].color)) lastpixel = i;
      }
      if(lastpixel < pixelCount)
      {
        if(lastpixel == pixelCount) strip.setPixelColor(pixelCount-1,PresetColor(mode[selectedMode].color));
        else if(lastpixel == 0) strip.setPixelColor(pixelCount-1,PresetColor(mode[selectedMode].color));
        else
        {
          if(strip.getPixelColor(lastpixel-1) == PresetColor(mode[selectedMode].color)) 
          {
            if(lastpixel == pixelCount-1) stepcounter=0;
            else strip.setPixelColor(pixelCount-1,PresetColor(mode[selectedMode].color));
          }
          else
          {
            strip.setPixelColor(lastpixel-1,PresetColor(mode[selectedMode].color));
            strip.setPixelColor(lastpixel,modifiedBackcolor);
          }
        }
      }
    }
  }
}
//####################################################################################################################################
void heartbeat()
{
  if(millis() < timer) timer = millis();
  if(millis() - timer > (mode[selectedMode].Speed*40)+500)
  {
    timer = millis();
    uint32_t modifiedBackcolor = PresetColor(mode[selectedMode].backcolor);
    if(mode[selectedMode].backcolor == mode[selectedMode].color) modifiedBackcolor = Dim(modifiedBackcolor,60);
    
    for(int i=0; i<=100;i++)
    {
      colorFill(Dim(PresetColor(mode[selectedMode].color),i));
      delay(2);
    }
    for(int i=100; i>=0;i--)
    {
      colorFill(Dim(PresetColor(mode[selectedMode].color),i));
      delay(3);
    }
    for(int i=0; i<=100;i++)
    {
      colorFill(Dim(PresetColor(mode[selectedMode].color),i));
      delay(2);
    }
    for(int i=100; i>=0;i--)
    {
      colorFill(Dim(PresetColor(mode[selectedMode].color),i));
      delay(3);
    }
    colorFill(modifiedBackcolor);
  }
}
//####################################################################################################################################

void theatherRunning()
{
  if(millis() < timer) timer = millis();
  if(millis() - timer > (mode[selectedMode].Speed*2)+100)
  {
    timer = millis();
    uint32_t modifiedBackcolor = PresetColor(mode[selectedMode].backcolor);
    if(mode[selectedMode].backcolor == mode[selectedMode].color) modifiedBackcolor = Dim(modifiedBackcolor,60);
  
    colorFull(modifiedBackcolor);
    stepcounter = stepcounter % 3;
    for(int i=stepcounter; i<pixelCount; i+=3)
    {
      strip.setPixelColor(i,PresetColor(mode[selectedMode].color));
    }
    stepcounter++;
    strip.show();
  }
}
//####################################################################################################################################
void fountain()
{
  if(millis() < timer) timer = millis();
  if(millis() - timer > (mode[selectedMode].Speed*4)+100)
  {
    timer = millis();
    uint32_t modifiedBackcolor = PresetColor(mode[selectedMode].backcolor);
    if(mode[selectedMode].backcolor == mode[selectedMode].color) modifiedBackcolor = Dim(modifiedBackcolor,60);
    colorFull(modifiedBackcolor);
    if(stepcounter >= 2*pixelCount) stepcounter=0;
    if(stepcounter <=pixelCount)
    {
      for(int i=0;i<stepcounter;i++)
      {
        strip.setPixelColor(i,PresetColor(mode[selectedMode].color));
      }
    }
    else
    {
      for(int i=0;i<2*pixelCount-stepcounter;i++)
      {
        strip.setPixelColor(i,PresetColor(mode[selectedMode].color));
      }
    }
    stepcounter++;
    strip.show();
  }
}
//####################################################################################################################################
void DIMnFILL(int delays, uint32_t colors, int dims)
{
      colors = Dim(colors,dims);
      colorFill(colors);
      delay(delays);
}
void spark()
{
  if(millis() < timer) timer = millis(); //if millis() has overflown
  if(millis() - timer > random(1000,(mode[selectedMode].Speed*40)+1100))
  {
    timer = millis();
    
    uint32_t localColor = presetColor[mode[selectedMode].color];
    
    for(int i=10; i<=60;i++)
    {      
      DIMnFILL(1,localColor,i);
    }
    for(int i=60; i>=20;i--)
    {
      DIMnFILL(5,localColor,i);
    }
    for(int i=20; i<=200;i++)
    {
      DIMnFILL(1,localColor,i);
    }
    for(int i=200; i>=100;i--)
    {
      DIMnFILL(3,localColor,i);
    }  
    colorFill(presetColor[mode[selectedMode].backcolor]);
  }
}
//####################################################################################################################################
void fadeFlow()
{
  if(millis() < timer) timer = millis();
  if(millis() - timer > (mode[selectedMode].Speed*4)+100)
  {
    timer = millis();
    uint32_t modifiedBackcolor = PresetColor(mode[selectedMode].backcolor);
    if(mode[selectedMode].backcolor == mode[selectedMode].color) modifiedBackcolor = Dim(modifiedBackcolor,60);
  
    if(stepcounter >= pixelCount) stepcounter = 0;
  
    int diffR = Red(modifiedBackcolor) - Red(PresetColor(mode[selectedMode].color));
    int diffG = Green(modifiedBackcolor) - Green(PresetColor(mode[selectedMode].color));
    int diffB = Blue(modifiedBackcolor) - Blue(PresetColor(mode[selectedMode].color));
  
    int gapR = (pixelCount/2) / (abs(diffR) +1);
    int gapG = (pixelCount/2) / (abs(diffG) +1);
    int gapB = (pixelCount/2) / (abs(diffB) +1);
  
    int stepR = diffR / (pixelCount/2);
    int stepG = diffG / (pixelCount/2);
    int stepB = diffB / (pixelCount/2);
  
    strip.setPixelColor(stepcounter,PresetColor(mode[selectedMode].color));
    for(int i=1; i<(pixelCount/2); i++)
    {
      int p = (i+stepcounter) % pixelCount; 
      byte R = Red(PresetColor(mode[selectedMode].color));
      byte G = Green(PresetColor(mode[selectedMode].color));
      byte B = Blue(PresetColor(mode[selectedMode].color)); 
  
      if(gapR == 0 && diffR != 0)
      {
        R = R + stepR*i; //we need to add stepX in every steps.
      }
      else if(diffR != 0)
      {
        //we need to add/decrease 1 value per gap exceeded
        R = R + (i/gapR)*diffR/abs(diffR);
      }
  
      
      if(gapG == 0 && diffG != 0)
      {
        G = G + stepG*i; //we need to add stepX in every steps.
      }
      else if(diffG != 0)
      {
        //we need to add/decrease 1 value per gap exceeded
        G = G + (i/gapG)*diffG/abs(diffG);
      }
  
      
      if(gapB == 0 && diffB != 0)
      {
        B = B + stepB*i; //we need to add stepX in every steps.
      }
      else if(diffB != 0)
      {
        //we need to add/decrease 1 value per gap exceeded
        B = B + (i/gapB)*diffB/abs(diffB);
      }
  
      strip.setPixelColor(p,strip.Color(R,G,B));
    }
    strip.setPixelColor((stepcounter+pixelCount/2)%pixelCount,PresetColor(mode[selectedMode].backcolor));
    for(int i=pixelCount/2+1; i< pixelCount; i++)
    {
      int p = (i+stepcounter) % pixelCount; 
      byte R = Red(PresetColor(mode[selectedMode].backcolor));
      byte G = Green(PresetColor(mode[selectedMode].backcolor));
      byte B = Blue(PresetColor(mode[selectedMode].backcolor)); 
  
      if(gapR == 0 && diffR != 0)
      {
        R = R - stepR*i; //we need to take stepX in every steps.
      }
      else if(diffR != 0)
      {
        //we need to add/decrease 1 value per gap exceeded
        R = R - (i/gapR)*diffR/abs(diffR);
      }
  
      
      if(gapG == 0 && diffG != 0)
      {
        G = G - stepG*i; //we need to take stepX in every steps.
      }
      else if(diffG != 0)
      {
        //we need to add/decrease 1 value per gap exceeded
        G = G - (i/gapG)*diffG/abs(diffG);
      }
  
      
      if(gapB == 0 && diffB != 0)
      {
        B = B - stepB*i; //we need to add stepX in every steps.
      }
      else if(diffB != 0)
      {
        //we need to add/decrease 1 value per gap exceeded
        B = B - (i/gapB)*diffB/abs(diffB);
      }
  
      strip.setPixelColor(p,strip.Color(R,G,B));
    }    
    strip.show();
    stepcounter++;  
  }
}
//####################################################################################################################################
void fadeFlowRainbow()
{
  if(stepcounter >= pixelCount)
  {
    if(mode[selectedMode].backcolor != 0) mode[selectedMode].backcolor = mode[selectedMode].color;
    mode[selectedMode].color++;
    if(mode[selectedMode].color == COLOR_COUNT) mode[selectedMode].color=1;
  }
  fadeFlow();
}
//####################################################################################################################################
void fountainRainbow()
{
  if(stepcounter >= 2*pixelCount)
  {
    if(mode[selectedMode].backcolor != 0) mode[selectedMode].backcolor = mode[selectedMode].color;
    mode[selectedMode].color++;
    if(mode[selectedMode].color == COLOR_COUNT) mode[selectedMode].color=1;
  }
  fountain();
}
//####################################################################################################################################
void heartbeatRainbow()
{
    if(mode[selectedMode].backcolor != 0) mode[selectedMode].backcolor = mode[selectedMode].color;
    mode[selectedMode].color++;
    if(mode[selectedMode].color == COLOR_COUNT) mode[selectedMode].color=1;
    heartbeat();
}
//####################################################################################################################################
void tetrisRainbow()
{
  if(stepcounter == 0)
  {
    if(mode[selectedMode].backcolor != 0) mode[selectedMode].backcolor = mode[selectedMode].color;
    mode[selectedMode].color++;
    if(mode[selectedMode].color == COLOR_COUNT) mode[selectedMode].color=1;
  }
  tetris();
}
//####################################################################################################################################
void patrolRainbow()
{
  if(stepcounter == 0)
  {
    if(mode[selectedMode].backcolor != 0) mode[selectedMode].backcolor = mode[selectedMode].color;
    mode[selectedMode].color++;
    if(mode[selectedMode].color == COLOR_COUNT) mode[selectedMode].color=1;
  }
  patrol();
}
//####################################################################################################################################
void fadeFillRainbow()
{
  if(stepcounter == 0)
  {
    if(mode[selectedMode].backcolor != 0) mode[selectedMode].backcolor = mode[selectedMode].color;
    mode[selectedMode].color++;
    if(mode[selectedMode].color == COLOR_COUNT) mode[selectedMode].color=1;
  }
  fadeFill();
}

void colorFull(uint32_t c)
{
    for(uint16_t i=0; i<strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, c);
    }  
}

void colorFill(uint32_t c)
{
    for(uint16_t i=0; i<strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, c);
    }
    //strip.fill(c);
    strip.show();
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) 
{
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void colorSwipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=strip.numPixels()-1; i>=0; i--) 
  {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) 
{
  uint16_t i, j;

  for(j=0; j<256; j++) 
  {
    for(i=0; i<strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) 
{
  uint16_t i, j;

  for(j=0; j<256*5; j++) // 5 cycles of all colors on wheel
  { 
    for(i=0; i< strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) //do 10 cycles of chasing
  {  
    for (int q=0; q < 3; q++) 
    {
      for (int i=0; i < strip.numPixels(); i=i+3) 
      {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) 
      {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) // cycle all 256 colors in the wheel
  {     
    for (int q=0; q < 3; q++) 
    {
        for (int i=0; i < strip.numPixels(); i=i+3) 
        {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) 
        {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) 
{
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) 
  {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } 
  else if(WheelPos < 170) 
  {
   WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } 
  else 
  {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

uint32_t PresetColor(byte presetpos)
{
  /*
      uint32_t result = strip.Color(0,0,0);
      switch(presetpos)
      {
        case 0: result = strip.Color(0,0,0);
                break;
        case 1: result = strip.Color(0,0,255); 
                break;
        case 2: result = strip.Color(127,255,0); 
                break;
        case 3: result = strip.Color(0,255,255); 
                break;
        case 4: result = strip.Color(255,140,0);  
                break;
        case 5: result = strip.Color(255,215,0);
                break;
        case 6: result = strip.Color(0,128,0);  //GREEN
                break;
        case 7: result = strip.Color(75,0,130); 
                break;
        case 8: result = strip.Color(0,255,0); //GREEN
                break;
        case 9: result = strip.Color(255,0,255); //MAGENTA
                break;
        case 10: result = strip.Color(0,250,154); 
                break;
        case 11: result = strip.Color(255,165,0); //Orange
                break;
        case 12: result = strip.Color(255,69,0); //OrangeRed
                break;
        case 13: result = strip.Color(128,0,128); //PURPLEMAGENTA
                break;
        case 14: result = strip.Color(255,0,0); 
                break;
        case 15: result = strip.Color(0,255,127); 
                break;
        case 16: result = strip.Color(0,128,128);  
                break;
        case 17: result = strip.Color(255,255,255); 
                break;
        case 18: result = strip.Color(255,255,0); 
                break;
        case 19: result = strip.Color(154,205,50);  
                break;
        default: result = strip.Color(0,0,0);
                break;
      }
      return result;
      */
      return presetColor[presetpos % COLOR_COUNT];
}

uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
uint32_t Dim(uint32_t c, int percent)
{
  int redcomponent = Red(c);
  int greencomponent = Green(c);
  int bluecomponent = Blue(c); 

  redcomponent = redcomponent * percent;
  greencomponent = greencomponent * percent;
  bluecomponent = bluecomponent * percent; 
  redcomponent = redcomponent / 100;
  greencomponent = greencomponent / 100;
  bluecomponent = bluecomponent / 100;

  if(redcomponent > 255) redcomponent = 255;
  if(greencomponent > 255) greencomponent = 255;
  if(bluecomponent > 255) bluecomponent = 255;

  return strip.Color((byte)redcomponent,(byte)greencomponent,(byte)bluecomponent);  
}
