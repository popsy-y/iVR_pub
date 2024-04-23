// include guard
#pragma once

#include "sim.h"
#include "calc.h"
#include "derivedTypeDef.h"

extern SimDataT simdata;

static ObjDataT *pd = &simdata.player.base;
static ObjDataT *od = &simdata.objective.base;

//  EnemyGeneric behavior logics
// ---------------------------------------
EnemyGeneric::EnemyGeneric()
	:state(0), stateEnteredAt(-INFINITY), noticeDist(0), attackDist(0), moveSpeed(0), target(nullptr), attackedAt(-INFINITY), clrs({})
{
	isKilled = false;
	isVanishable = false;

	spawn();
}

void EnemyGeneric::spawn() {
	std::cout << "EnemyGeneric Spawn" << std::endl;

	vector_t initPos = getSpawnLocation(simdata.config.mapData.stageRadius, simdata.config.mapData.minSpawnDistance);

	this->base.pos.x = initPos.x;
	this->base.pos.y += .5;
	this->base.pos.z = initPos.z;

	euler_t rotAngle;
	EulerAtoB(&this->base, od, &rotAngle);
	this->base.rot.yaw = rotAngle.yaw;

	setAryObjClr(this->clrs[0]);

	setObjLocal(&this->attackField, &this->base);

	std::for_each(this->parAngs.begin(), this->parAngs.end(), [](float& e) mutable {
		e = uniformRandom(0, M_PI * 2);
	});

	this->stateEnteredAt = simdata.frameCount;
}

void EnemyGeneric::update() {
	float dist2p = DistanceAtoB(&this->base, pd);
	float dist2o = DistanceAtoB(&this->base, od);
	int fr = simdata.frameCount;

	switch (this->state)
	{
	case 0: // NEUTRAL
		// set target
		if (dist2p < this->noticeDist)
		{
			target = pd;
		}
		else {
			target = od;
		}

		// attack if nearer than threshold
		if (dist2p < this->attackDist || dist2o < this->attackDist)
		{
			this->stateEnteredAt = fr;
			setAryObjClr(this->clrs[1]);

			// transition to state 1: CAST
			this->state = 1;
		}

		if (dist2p > this->minimumDist && dist2o > this->minimumDist)
		{
			moveGeneral();
		}

		break;
	case 1: // CAST
		//cast(); // DEPRECATED: cast(); is now modified to function for draw particles
		this->applyRotation();

		if (fr - this->stateEnteredAt > this->castLength)
		{
			this->stateEnteredAt = fr;
			setAryObjClr(this->clrs[2]);

			// transition to state 2: ATTACK
			onAttackEnter();

			this->state = 2;
		}
		break;
	case 2: // ATTACK
		//attack(); // DEPRECATED: attack(); is now modified to function for draw particles

		if (fr - this->stateEnteredAt > this->attackLength)
		{
			this->stateEnteredAt = fr;
			setAryObjClr(this->clrs[0]);

			// transition to state 0: NEUTRAL
			this->state = 0;
		}
		break;
	case 3: // HIT
		hitReactGeneral();

		if (fr - this->stateEnteredAt > this->hitLength)
		{
			this->stateEnteredAt = fr;

			if (this->health <= 0) // killed or not
			{
				this->isKilled = true;
				setAryObjClr(this->clrs[4]);

				// transition to state 4: KILLED
				this->state = 4;
			}
			else
			{
				setAryObjClr(this->clrs[0]);

				// transition to state 0: NEUTRAL
				this->state = 0;
			}
		}
		break;
	case 4: // KILLED
		killedReactGeneral();

		if (fr - this->stateEnteredAt > this->killedLength)
		{
			simdata.enemyMgr.registerVanished();
			isVanishable = true;
		}
		break;
	default:
		std::cout << "[DEV][WARN] EnemyGeneric: INVALID STATE INTEGER." << std::endl;
		std::cout << "[DEV][WARN] EnemyGeneric: ENTERED TO default STATE. RESET STATE TO 0: NEUTRAL." << std::endl;
		this->state = 0;
		break;
	}
}

void EnemyGeneric::moveGeneral() {
	// set rotation
	euler_t rotAngle;
	EulerAtoB(&this->base, this->target, &rotAngle);

	this->base.rot.yaw = rotAngle.yaw;

	// set move speed
	this->base.move = this->moveSpeed;

	// apply
	MoveObject(&this->base);
}

void EnemyGeneric::applyRotation() {
	// set rotation
	euler_t rotAngle;
	EulerAtoB(&this->base, this->target, &rotAngle);

	this->base.rot.yaw = rotAngle.yaw;
}

void EnemyGeneric::hitReactGeneral() {
	setAryObjClr(this->clrs[3]);
	this->target = pd;
	//std::cout << "instance hit" << std::endl;
}

void EnemyGeneric::killedReactGeneral() {
	setAryObjClr(this->clrs[4]);
	//std::cout << "instance killed" << std::endl;
}

std::vector<float> EnemyGeneric::getPosV3(){
	return std::vector<float>{this->base.pos.x, this->base.pos.y, this->base.pos.z};
}

void EnemyGeneric::hitInstance(float damageAmount) {
	this->health -= damageAmount;
	this->stateEnteredAt = simdata.frameCount;
	this->state = 3;
}

void EnemyGeneric::setAryObjClr(std::array<float, 4> clrAry) {
	// std::cout << "setAryObjClr" << std::endl;

	this->base.color.red = clrAry[0];
	this->base.color.green = clrAry[1];
	this->base.color.blue = clrAry[2];
	this->base.color.alpha = clrAry[3];
}