#include <Wire.h>       //Using I2C
#include <eeprom.h>     //In game changes like the number of hits can be stored on the EEPROM. Turning off the Arduino will keep the score saved.
#include <Adafruit_SSD1306.h>   //Controlling OLED screen
#include <Adafruit_GFX.h>       //Refer to actual documentation
#include "template.h"   //contains the template of the basic screen. Components that don't change on the screen or their default values are fed in the template bitmap
#include "frame.h"      //sources the template and makes it dynamic
#include "hexmap.h"     //stores a custom hexmap. generated by the BitConverter python module

#define spk 9   //some SFX will be good I suppose
#define trig 0  //interrupt pin 2 on uno; trigger button
#define ir_tx 5 //IR transmission LED
#define ir_rx 1 //interrupt pin 3 on uno; IR reception LED (the "tag")

#define _height 64
#define _width 128

volatile unsigned int health, ammo;
volatile bool alive;

class elapsedMillis     //creates objects whose values are automatically incremented every millisecond; millis() not required
{
private:
    unsigned long ms;
public:
    elapsedMillis(void) { ms = millis(); }
    elapsedMillis(unsigned long val) { ms = millis() - val; }
    elapsedMillis(const elapsedMillis &orig) { ms = orig.ms; }
    operator unsigned long () const { return millis() - ms; }
    elapsedMillis & operator = (const elapsedMillis &rhs) { ms = rhs.ms; return *this; }
    elapsedMillis & operator = (unsigned long val) { ms = millis() - val; return *this; }
    elapsedMillis & operator -= (unsigned long val)      { ms += val ; return *this; }
    elapsedMillis & operator += (unsigned long val)      { ms -= val ; return *this; }
    elapsedMillis operator - (int val) const           { elapsedMillis r(*this); r.ms += val; return r; }
    elapsedMillis operator - (unsigned int val) const  { elapsedMillis r(*this); r.ms += val; return r; }
    elapsedMillis operator - (long val) const          { elapsedMillis r(*this); r.ms += val; return r; }
    elapsedMillis operator - (unsigned long val) const { elapsedMillis r(*this); r.ms += val; return r; }
    elapsedMillis operator + (int val) const           { elapsedMillis r(*this); r.ms -= val; return r; }
    elapsedMillis operator + (unsigned int val) const  { elapsedMillis r(*this); r.ms -= val; return r; }
    elapsedMillis operator + (long val) const          { elapsedMillis r(*this); r.ms -= val; return r; }
    elapsedMillis operator + (unsigned long val) const { elapsedMillis r(*this); r.ms -= val; return r; }
};

elapsedMillis deathTime, shootTime, reloadTime;

class Laser
{
    uint8_t pin;
    uint16_t time, prevMillis;
    bool state;
public:
    Laser(uint8_t p, uint16_t period)
    {
        pin = p;
        time = interval;
        prevMillis = 0;
        state = true;
    }

    void update()   //updates the output on ir_tx, no other routine is needed to constantly change pin outputs; function is constantly being called in the loop function
    {
        currentMillis = millis();
        if( state && currentMillis - prevMillis > time)
        {
            state = false;
            digitalWrite(ir_tx, LOW);
        }
        else()
        {
            digitalWrite(ir_tx, HIGH);
        }
    }

    void shoot()    //pulling the trigger calls just the shoot function
        state = true;
}

Laser laser(ir_tx, 100);

void reload()
{
    reloadTime = 0;
    display.setCursor(15,7);
    display.setTextSize(1);
    display.println("Reloading...")
}

void shoot()
{
    if(alive && ammo && (reloadTime > 3000) && (shootTime > 500))   //shoot only if alive and ammo is left and previous shoot happened at least 0.5 seconds ago
    {
        shootTime = 0;
        digitalWrite(ir_tx, HIGH);
        tone(spk, 500, 50);
        ammo -= 1;
        display.display();
        delay(500);
    }
    if(ammo == 0)
        reload();
}

void hit()
{
    if(alive)
    {
        health = constrain(health-10, 0, health);
        delay(200);     //give a buffer before second hit is recorded
        if(!health)
        {
            deathTime = 0;
            alive = 0;
            dead();     //respawn() is called from dead()
        }
    }
    updateScreen();     //update after respawn
}

void dead()
{
    display.setCursor(27,7);
    display.setTextSize(6);
    display.println("DEAD");
    for(uint8_t i = 5; i > 0; i--)
    {
        display.print("Respawning in... ");
        display.println(i);
        display.display();
        delay(1000);
    }
    respawn();
}

void respawn()      //reset to default values
{
    ammo = 10;
    health = 100;
    alive = true;
}

void updateScreen()
{
    display.setCursor(27,7);    //offset for AMMO in template
    display.setTextSize(1);
    display.println(ammo);
    display.setCursor(15,42);   //offset for HP in template
    display.setTextSize(1);
    display.println(health);
}

void setup()
{
    pinMode(spk, OUTPUT);
    pinMode(trig, INPUT);
    attachInterrupt(ir_rx, hit(), RISING);
    attachInterrupt(trig, shoot(), RISING);
}

void loop()
{
    laser.update();
    display.display();
}
