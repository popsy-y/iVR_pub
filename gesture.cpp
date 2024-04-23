#pragma once

#include "common.h"
#include "config.h"
#include "sim.h"
#include "keyboard.h"
#include "gesture.h"

using namespace std;

extern int debugSkillStatus;

extern float handFore;
extern float handBack;
extern float handUp;
extern float handDown;

extern bool debugItemActivated;

bool isGestureTriggered() {
	return true;
}

int skillGestureChecker(ObjDataT *hL, ObjDataT *hR) {
	float LY = hL->pos.y;
	float RY = hR->pos.y;

	float RZ = hR->pos.z;

	if (kbdControl)
	{
		cout << debugSkillStatus << endl;
		// defined and controlled in keyboard.cpp
		return debugSkillStatus;
	} else {
		if (LY < handDown && RY < handDown)
		{
			// return area attack
			return 1;
		}

		if (RY > handUp)
		{
			// return magical attack
			return 2;
		}

		if (RZ < handFore)
		{
			// return melee attack
			return 0;
		}
	}

	// return 999 as "NO GESTURE DETECTED"
	return 999;
}

bool spellCastChecker(ObjDataT *hR) {

	if (hR->pos.y > handUp)
	{
		return true;
	}

	return false;
}

bool spellShootChecker(ObjDataT *hR) {
	if (kbdControl && hR->pos.z < 1.8)
	{
		return true;
	}

	if (hR->pos.z < handFore)
	{
		return true;
	}

	return false;
}

int movementGestureChecker(ObjDataT *hL, ObjDataT *hR) {
	bool hLFore = hL->pos.z < handFore;
	bool hLBack = hL->pos.z > handBack;

	bool hRFore = hR->pos.z < handFore;
	bool hRBack = hR->pos.z > handBack;

	// forward
	if (hLFore && hRFore)
	{
		return 0;
	}
	// back
	if (hLBack && hRBack)
	{
		return 1;
	}

	// right turn
	if (hLFore && hRBack)
	{
		return 2;
	}
	// left turn
	if (hLBack && hRFore)
	{
		return 3;
	}

	return 4;
}

bool itemActivationChecker(ObjDataT *hR) {
	if (kbdControl && debugItemActivated)
	{
		return true;
	}
	else
	{
		if (hR->pos.y < handDown)
		{
			return true;
		}
	}
	

	return false;
}

bool restartGestureChecker(ObjDataT *hL, ObjDataT *hR) {
	bool hLUp = hL->pos.y > handUp;
	bool hRUp = hR->pos.y > handUp;

	if (kbdControl && hR->pos.y > 1.7)
	{
		return true;
	}

	if (hLUp && hRUp)
	{
		return true;
	}

	return false;
}