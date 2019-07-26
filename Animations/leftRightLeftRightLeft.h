#pragma once

class LeftRightLeftRightLeft : public Animation
{
  public:
    using Animation::Animation;

    uint8_t hue = 0;
    bool left = true;
    int16_t currentLed = 0;

    void reset()
    {
      hue = 0;
      left = true;
      currentLed = 0;
    }

    void loop()
    {
      strip[currentLed] = CHSV(hue++, Config.saturation, 255);
      for(int i = 0; i < NUM_LEDS; i++)
        strip[i].nscale8(250);
      currentLed += left ? 1 : -1;
      if(currentLed == NUM_LEDS || currentLed == -1)
        left = !left;

      delay(50);
    }
};
