#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

#define TFT_CS     10
#define TFT_RST    9  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to -1!
#define TFT_DC     8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);


#include <stdint.h>

#include "ship.h"
#include "disparo.h"
#include "rocks.h"
#include "coord.h"
#include "game_over.h"

#define PLAYING 0
#define GAME_OVER 1

#define SCREEN_W 160
#define SCREEN_H 118

#define JY A0
#define JX A1
#define JOYSTICK_BUTTON 2
// Cuanta menor velocidad más rapido
#define V_SHIP 16
#define V_DISPARO 2
#define V_ROCKS 200
#define V_ADV_ROCKS 3
#define V_SPAWN_ROCK 2100
#define V_SCORE 900
#define V_ESTRELLAS 100

#define RANDOM_ANALOG 3
#define MAX_ACTIVE_ROCKS 10
#define MAX_ACTIVE_DISPAROS 10
#define MAX_ACTIVE_ESTRELLAS 50
int x = SCREEN_W/2-SHIP_W/2;
int y = SCREEN_H-SHIP_H;
unsigned active_disparos = 0;
unsigned active_rocks = 0;
unsigned long last_shot_time, t_delay = 0, t_laser = 0, t_rocks=0, 
              t_spawn_rock=0, t_score=0, t_estrellas = 0;
unsigned int score = 0;
unsigned int game_state = GAME_OVER;

disparo_s disparo [MAX_ACTIVE_DISPAROS];
rock_s rock [MAX_ACTIVE_ROCKS];
coord_s estrella [MAX_ACTIVE_ESTRELLAS];

void setup() {
  Serial.begin(9600);
  pinMode(JOYSTICK_BUTTON, INPUT_PULLUP);
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);
  //tft.fillRect(0, 0, 128-SCREEN_H, SCREEN_W, ST7735_GREEN);
  tft.setRotation(1);
  tft.setTextColor(ST7735_BLACK, ST7735_GREEN);
  init_disparos();
  init_rocks();
  init_estrellas();
}

void loop() {
  start_screen();
  while(game_state == PLAYING){
  unsigned long t = millis();
  if(t_score < t){
    t_score = t + V_SCORE;
    tft.fillRect(0, SCREEN_H, 160, 128, ST7735_GREEN);
    tft.setCursor(0,SCREEN_H);    
    tft.print("Score: ");tft.println(score);
  }
  //void fillRect(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, uint16_t color);  
  int y_desp = (analogRead(A0)+20)/100-5;
  int x_desp = (analogRead(A1)/100-5)*-1; 

  if(!digitalRead(JOYSTICK_BUTTON) && (t-last_shot_time) > 200){
    last_shot_time = t;
    if(active_disparos < 10){
      spawn_disparo(x + SHIP_W/2, y);      
    }
  } 

  if(t_estrellas < t){
    t_estrellas = t+V_ESTRELLAS;
    pintar_estrellas();
  }

  if(t_delay < t){
    t_delay = t+V_SHIP;
    if(y_desp != 0 || x_desp != 0) 
      tft.fillRect(x, y, SHIP_W, SHIP_H, ST7735_BLACK);
      
    if(x >= -SHIP_W/2 && x <= SCREEN_W-SHIP_W/2) x += x_desp;
    if( x < -SHIP_W/2 ) x = -SHIP_W/2;
    if (x > SCREEN_W-SHIP_W/2) x = SCREEN_W-SHIP_W/2;
    
    if(y >= 0 && y <= SCREEN_H-SHIP_H) y += y_desp;
    if( y < 0 ) y = 0;
    if (y > SCREEN_H-SHIP_H) y = SCREEN_H-SHIP_H;
    tft.drawBitmap(x, y, ship_sprite, SHIP_W, SHIP_H, ST7735_WHITE);
  }

  
  if(t_laser < t && active_disparos){
    t_laser = t+V_DISPARO;
      move_disparos();
  }

  if(t_rocks < t){
    t_rocks = t+V_ROCKS;    
    move_rocks();
  }

  if(t_spawn_rock < t){
    t_spawn_rock = t+V_SPAWN_ROCK;
    spawn_rock_random();
    }
  
  detect_colisions();
  }
  game_over();
}

void start_screen(){
 
  }  

void game_over(){
  unsigned int t_delay = 0;
  bool invert = true;
  score = 0;
  tft.drawBitmap(0,0, game_over_bitmap, 160, 128, ST7735_WHITE);
  game_state = PLAYING;
  while(true){
    unsigned int t = millis();
    if(t_delay < t){
      t_delay = t + 500;
      invert = (invert) ? 0 : 1;
      tft.invertDisplay(invert);
    }
    if(!digitalRead(JOYSTICK_BUTTON)) break;
  
  }
  tft.invertDisplay(false);
  tft.fillRect(0,0, 160, 128, ST7735_BLACK);
}

void detect_colisions(void){
  detect_laser_rock_colisions();
  detect_ship_rock_colisions();  
  }

void detect_laser_rock_colisions(){
  
  for(int i = 0; i < MAX_ACTIVE_ROCKS; i++){
    for(int j = 0; j < MAX_ACTIVE_DISPAROS; j++){
      int distancex = (disparo[j].x - rock[i].x);
      int distancey = (disparo[j].y - rock[i].y);
      if(!(rock[i].active && disparo[j].active) || rock[i].explotar) break;
      if ((distancex < ROCK1_W) && (distancex > 0) && (distancey < ROCK1_H)){        
        rock[i].explotar = true;
        score+=50;
      }
    }
  }
}

void detect_ship_rock_colisions(){
  
  }
  
void fill_rect(uint16_t x, uint16_t y, uint16_t h, uint16_t w, uint16_t color){
  for(int i = y; i < h+y; i++){
    //void drawFastHLine(uint8_t x0, uint8_t y0, uint8_t length, uint16_t color);
    tft.drawFastHLine(x, i, w, ST7735_BLACK);
  }
}

void move_disparo(int i){
  if(!disparo[i].active) return;
  tft.fillRect(disparo[i].x, disparo[i].y, DISPARO_W, DISPARO_H, ST7735_BLACK);
  disparo[i].y--;
  if(disparo[i].y > 0){
    tft.fillRect(disparo[i].x, disparo[i].y, DISPARO_W, DISPARO_H, ST7735_RED);
  }
  else{
    disparo[i].active = false;
    active_disparos--;
  }
  
}

void  move_disparos(){
  for(int i = 0; i < MAX_ACTIVE_DISPAROS; i++)
    move_disparo(i);
}

void spawn_disparo(int x, int y){
  for(int i = 0; i < MAX_ACTIVE_DISPAROS; i++)
    if(!disparo[i].active){
      disparo[i].x = x;
      disparo[i].y = y;
      disparo[i].active = true;
      active_disparos++;
      return;
      }      
}

void init_disparos(){
  for(int i = 0; i < MAX_ACTIVE_DISPAROS; i++)
    disparo[i].active = false;
}

void init_rocks(){
  for(int i = 0; i < MAX_ACTIVE_ROCKS; i++)
    rock[i].active = false;
}

void spawn_rock_random(){
  int xpos = rand()%SCREEN_W;

  if(active_rocks < MAX_ACTIVE_ROCKS){
    active_rocks++;
    for(int i = 0; i < MAX_ACTIVE_ROCKS; i++){
      if(!rock[i].active){
        rock[i].x = xpos;
        rock[i].y = 0;
        rock[i].active = true;
        return;
        }
      }
  }
}
void move_rock(int i){
  if(!rock[i].active) return;
  
  
  tft.fillRect(rock[i].x, rock[i].y, ROCK1_W, ROCK1_H, ST7735_BLACK);
  rock[i].y+=V_ADV_ROCKS;
  if(rock[i].explotar){
    rock[i].explotar = false;
    rock[i].active = false;
    active_rocks--;
    return;
  }
  if(rock[i].y < SCREEN_H-ROCK1_H){
    tft.drawBitmap(rock[i].x, rock[i].y, rock1, ROCK1_H, ROCK1_W, ST7735_WHITE);
  }
  else{
    rock[i].active = false;
    game_state = GAME_OVER;
    active_rocks--;
  }
}
void move_rocks(){
  for(int i = 0; i < MAX_ACTIVE_ROCKS; i++){
    move_rock(i);
    }
  }

void init_estrellas(void){
  for(int i = 0; i < MAX_ACTIVE_ESTRELLAS; i++){
      int posx = rand()%SCREEN_W;
      int posy = rand()%SCREEN_H;
      estrella[i].active = true;
      estrella[i].x = posx;
      estrella[i].y = posy;
  }  
}
void pintar_estrellas(void){  
  for(int i = 0; i < MAX_ACTIVE_ESTRELLAS; i++){
    tft.drawPixel(estrella[i].x, estrella[i].y, ST7735_BLACK);
    if((estrella[i].y++) == SCREEN_H-1 ) estrella[i].y = 0;
    tft.drawPixel(estrella[i].x, estrella[i].y, ST7735_WHITE);        
    }
  }
