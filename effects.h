#pragma once
#include "sim.h"

void combinationCastEffect(int type, ObjDataT *source);

void playerCastEffect(ObjDataT *source);
void playerMeleeAttackEffect(ObjDataT *source);

std::vector<float> initEffectAngles(size_t amount, int angleFrom, int angleTo);
std::vector<float> initEffectCoords(size_t amount, int from, int to);