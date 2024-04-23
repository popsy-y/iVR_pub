#pragma once
bool isGestureTriggered(void);

int skillGestureChecker(ObjDataT *hL, ObjDataT *hR);

bool spellCastChecker(ObjDataT *hR);
bool spellShootChecker(ObjDataT *hR);

int movementGestureChecker(ObjDataT *hL, ObjDataT *hR);

bool itemActivationChecker(ObjDataT *hR);

bool restartGestureChecker(ObjDataT *hL, ObjDataT *hR);