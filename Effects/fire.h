#ifndef _FIRE_H
#define _FIRE_H

#include <Arduino.h>
#include <Time.h>

#include "../EffectController.h"

extern EffectConfiguration fire;

namespace fireNamespace
{
	extern void reset();
	extern void tick();
}

#endif