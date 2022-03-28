#include "FastLEDHub.h"

#include "Animation.h"
#include "ColorUtils.h"
#include "Config.h"
#include "Fade.h"
#include "SerialOut.h"
#include "Webserver.h"
#include "WebSocket.h"

#include "Animations/Color.h"

#include <ESPEssentials.h>

FastLEDHubClass::FastLEDHubClass() : status(STOPPED),
                                     speed(255),
                                     cycleButtonPushed(false),
                                     toggleButtonPushed(false),
                                     autostartHandled(false),
                                     filteredBrightness(128),
                                     potentiometerPin(-1),
                                     cycleButtonPin(-1),
                                     toggleButtonPin(-1)
{
}

void FastLEDHubClass::initialize(const String &projectName, uint16_t numberOfLeds)
{
  initESPEssentials(projectName);
  Config.initialize();
  if (WiFi.status() == WL_CONNECTED)
  {
    WebSocket::initialize();
    Webserver::initialize();
    Fade::initialize();
  }

  if (Config.sliderValues.size() >= 2)
  {
    setBrightness(Config.sliderValues.get(0));
    speed = Config.sliderValues.get(1);
  }

  registerAnimation(new Color("Color"));

  registerSlider(new Slider("Brightness", 0, 255, 255, 1));
  registerSlider(new Slider("Speed", 0, 255, 127, 1));
}

void FastLEDHubClass::enableCycleButton(uint8_t pin)
{
  inputTicker.attach_ms(FastLEDHub_INPUT_TICKER_INTERVAL, std::bind(&FastLEDHubClass::handleInput, this));
  pinMode(pin, INPUT);
  cycleButtonPin = pin;
}

void FastLEDHubClass::enableToggleButton(uint8_t pin)
{
  inputTicker.attach_ms(FastLEDHub_INPUT_TICKER_INTERVAL, std::bind(&FastLEDHubClass::handleInput, this));
  pinMode(pin, INPUT);
  cycleButtonPin = pin;
}

void FastLEDHubClass::enablePotentiometer(uint8_t pin)
{
  inputTicker.attach_ms(FastLEDHub_INPUT_TICKER_INTERVAL, std::bind(&FastLEDHubClass::handleInput, this));
  pinMode(pin, INPUT);
  potentiometerPin = pin;
}

void FastLEDHubClass::handle()
{
  handleESPEssentials();

  if (!autostartHandled)
    autostart();

  if (status == RUNNING && currentAnimation)
    currentAnimation->loop();

  Fade::handle();
  WebSocket::handle();
}

bool FastLEDHubClass::isDim()
{
  if (getBrightness() == 0)
    return true;

  for (uint16_t i = 0; i < FastLEDHub.count(); ++i)
  {
    for (uint16_t j = 0; j < FastLEDHub.size(); ++j)
    {
      if (FastLEDHub[i].leds()[j] != CRGB(0, 0, 0))
      {
        return false;
      }
    }
  }

  return true;
}

void FastLEDHubClass::delay(uint16_t ms)
{
  unsigned long start = micros();
  while (micros() - start < 1000.0 * ms * pow((FastLEDHub.speed - 255) / 128.0, 2))
  {
    handleESPEssentials();
    Fade::handle();
    WebSocket::handle();
  }
}

void FastLEDHubClass::registerSlider(Slider *slider)
{
  if (Config.sliderValues.size() > sliders.size())
  {
    slider->value = Config.sliderValues.get(sliders.size());
  }
  else
  {
    Config.sliderValues.add(slider->value);
  }

  sliders.add(slider);
}

void FastLEDHubClass::registerAnimation(Animation *animation)
{
  animations.add(animation);
}

Animation *FastLEDHubClass::getAnimation(String name)
{
  for (uint8_t i = 0; i < animations.size(); i++)
  {
    Animation *a = animations.get(i);
    if (a->getName() == name)
      return a;
  }

  return NULL;
}

Animation *FastLEDHubClass::getAnimation(uint8_t i)
{
  return (i < animations.size()) ? animations.get(i) : NULL;
}

Slider *FastLEDHubClass::getSlider(String name)
{
  for (uint8_t i = 0; i < sliders.size(); i++)
  {
    Slider *a = sliders.get(i);
    if (a->name == name)
      return a;
  }

  return NULL;
}

Slider *FastLEDHubClass::getSlider(uint8_t i)
{
  return (i < sliders.size()) ? sliders.get(i) : NULL;
}

void FastLEDHubClass::handleInput()
{
  if (potentiometerPin >= 0 && Fade::getMode() == Fade::FadeMode::NONE)
  {
    // Adjust the range slightly so low and high adc values
    // span the whole 10bit brightness range
    int16_t potiBrightness = (1023 - analogRead(A0) - 29) * 1.06;
    potiBrightness = constrain(potiBrightness, 0, 1023);

    // Low pass filter the potentiometer value to smoothen it out
    filteredBrightness = filteredBrightness - 0.01 * (filteredBrightness - potiBrightness);

    // Only set brightness if it's not near the filtered brightness value.
    // This will cause the actual brightness to lock in place once the potentiometer
    // hasn't been adjusted to prevent it from changing randomly for low quality
    // potentiometers.
    if (!(filteredBrightness - 1 < potiBrightness && potiBrightness < filteredBrightness + 1))
    {
      uint16_t brightness = (float)potiBrightness * potiBrightness / 1023 / 4;
      setBrightness(brightness);
    }
  }

  if (cycleButtonPin >= 0)
  {
    if (!digitalRead(cycleButtonPin) && !cycleButtonPushed)
    {
      Fade::stop();
      cycle();
      cycleButtonPushed = true;
    }
    else if (digitalRead(cycleButtonPin) && cycleButtonPushed)
    {
      cycleButtonPushed = false;
    }
  }

  if (toggleButtonPin >= 0)
  {
    if (!digitalRead(toggleButtonPin) && !toggleButtonPushed)
    {
      Fade::stop();
      toggle();
      toggleButtonPushed = true;
    }
    else if (digitalRead(toggleButtonPin) && toggleButtonPushed)
    {
      toggleButtonPushed = false;
    }
  }
}

void FastLEDHubClass::setSpeed(uint8_t newSpeed)
{
  speed = newSpeed;
}

void FastLEDHubClass::autostart()
{
  if (Config.startupAnimation != "")
    begin(getAnimation(Config.startupAnimation));

  autostartHandled = true;
}

void FastLEDHubClass::begin(Animation *animation)
{
  if (animation == NULL || (currentAnimation && currentAnimation->getName() == animation->getName()))
    return;

  clear(true);

  status = RUNNING;
  currentAnimation = animation;
  currentAnimation->reset();

  WebSocket::broadcastStatus();
  PRINTLN("Started '" + currentAnimation->getName() + "'");
}

void FastLEDHubClass::cycle()
{
  for (uint8_t i = 0; i < animations.size(); i++)
  {
    if (currentAnimation == getAnimation(i))
    {
      begin(getAnimation((i + 1) % animations.size()));
      return;
    }
  }
}

void FastLEDHubClass::stop()
{
  if (status == STOPPED)
    return;

  status = STOPPED;
  clear(true);
  currentAnimation = NULL;

  WebSocket::broadcastStatus();
  PRINTLN("Stopped animation");
}

void FastLEDHubClass::resume()
{
  if (status != PAUSED)
    return;

  status = RUNNING;

  WebSocket::broadcastStatus();
  PRINTLN("Resumed '" + currentAnimation->getName() + "'");
}

void FastLEDHubClass::restart()
{
  stop();
  begin(currentAnimation);

  PRINTLN("Restarted '" + currentAnimation->getName() + "'");
}

void FastLEDHubClass::pause()
{
  status = PAUSED;

  WebSocket::broadcastStatus();
  PRINTLN("Paused '" + currentAnimation->getName() + "'");
}

void FastLEDHubClass::toggle()
{
  if (status == RUNNING)
    pause();
  else
    resume();
}

void FastLEDHubClass::toggle(Animation *animation)
{
  if (currentAnimation && currentAnimation == animation && status == RUNNING)
    pause();
  else if (currentAnimation && currentAnimation == animation && status == PAUSED)
    resume();
  else
    begin(animation);
}

FastLEDHubClass FastLEDHub;
