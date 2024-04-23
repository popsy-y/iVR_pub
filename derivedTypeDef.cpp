// include guard
#pragma once

#include "sim.h"
#include "calc.h"
#include "derivedTypeDef.h"

#include <memory>
#include <algorithm>

extern SimDataT simdata;

static ObjDataT *pd = &simdata.player.base;
static ObjDataT *od = &simdata.objective.base;

vector_t getSpawnLocation(float floorRadius, float minSpawnDistance) {
	// decide distance from center and angle
	float spawnDistance = uniformRandom(minSpawnDistance, floorRadius);
	float angle = uniformRandom(0, M_PI*2);

	printf("dist: %f, ang: %f\n", minSpawnDistance, angle);

	vector_t result = { spawnDistance * sinf(angle), 0., spawnDistance * cosf(angle) };

	return result;
}

// --------------------------
//  MANAGER OBJECT
// --------------------------

void EnemyMgrT::instantiate(std::unique_ptr<EnemyGeneric> instance) {
	instance->spawn();

	this->enemies.push_back(std::move(instance));
}

void EnemyMgrT::spawn() {
	std::unique_ptr<EnemyGeneric> instance;

	if (onlyMelee)
	{
		instance = std::make_unique<EMelee>();
		this->instantiate(std::move(instance));
	}
	else {
		if (this->enemies.size() <= 0)
		{
			// spawn only melee class in initial spawn
			instance = std::make_unique<EMelee>();
			this->instantiate(std::move(instance));
		}

		float enemyClass = uniformRandom(0., 1.);
		if (enemyClass < .2)
		{
			// Spawn melee
			instance = std::make_unique<EMelee>();
			this->instantiate(std::move(instance));
		}
		else if (enemyClass < .8) {
			// Spawn range
			instance = std::make_unique<ERange>();
			this->instantiate(std::move(instance));
		}
		else {
			// Spawn caster
			instance = std::make_unique<ECaster>();
			this->instantiate(std::move(instance));
		}
	}
}

void EnemyMgrT::update() {
	// update
	for (auto& instance : enemies) {
		instance->update();
	}

	std::vector<std::unique_ptr<EnemyGeneric>> *eVec = &this->enemies;

	// send back killed instances
	auto killed = std::remove_if(
		eVec->begin(), eVec->end(),
		[](std::unique_ptr<EnemyGeneric>& enemy) { return enemy->isVanishable; }
	);

	// erase killed instances
	eVec->erase(killed, eVec->end());
}

void EnemyMgrT::draw() {
	for (auto& instance : enemies) {
		ObjDataT *base = instance->getBasePtr();
		applyObjColor(base);
		instance->draw();
	}
}

void EnemyMgrT::registerMeleeAttack(ObjDataT *source, float range, float damage) {
	for (auto& enemy : enemies) {
		float dist = DistanceAtoB(source, enemy->getBasePtr());

		if (dist < range)
		{
			enemy->hitInstance(damage);
		}
	}
}

void EnemyMgrT::registerAreaAttack(ObjDataT *source, float range, float damage) {
	for (auto& enemy : enemies) {
		float dist = DistanceAtoB(source, enemy->getBasePtr());

		if (dist < range)
		{
			if (enemy->getType() == 1)
			{
				enemy->hitInstance(damage * 5);
			}
			else {
				enemy->hitInstance(damage);
			}
		}
	}
}

void EnemyMgrT::registerSpellAttack(EnemyGeneric *target, float damage) {
	target->hitInstance(damage);
}

void EnemyMgrT::registerEMeleeAttack(ObjDataT *source, float range) {
	ObjectiveDataT *od = &simdata.objective;
	PlayerDataT *pd = &simdata.player;
	DifficultyMgrT *dm = &simdata.difficultyMgr;
	GameStateMgrT *gs = &simdata.gameState;

	float dist2o = DistanceAtoB(source, &od->base);
	float dist2p = DistanceAtoB(source, &pd->base);

	if (dist2o < simdata.config.Melee.attackRad)
	{
		od->hp -= dm->emBaseAtk + gs->emAtkAdj;
	}

	if (dist2p < simdata.config.Melee.attackRad)
	{
		pd->hp -= dm->emBaseAtk = gs->emAtkAdj;
		pd->lastDamaged = simdata.frameCount;
	}
}

void EnemyMgrT::registerEAreaAttack(ObjDataT *source, float range, int units) {
	ObjectiveDataT *od = &simdata.objective;
	PlayerDataT *pd = &simdata.player;

	DifficultyMgrT *dm = &simdata.difficultyMgr;
	GameStateMgrT *gs = &simdata.gameState;

	float dist2o = DistanceAtoB(source, &od->base);
	float dist2p = DistanceAtoB(source, &pd->base);

	if (dist2o < simdata.config.Range.attackRad)
	{
		od->hp -= (dm->erBaseAtk + gs->erAtkAdj) * units;
	}

	if (dist2p < simdata.config.Range.attackRad)
	{
		pd->hp -= (dm->erBaseAtk + gs->erAtkAdj) * units;
		pd->lastDamaged = simdata.frameCount;
	}
}

std::unique_ptr<EnemyGeneric>* EnemyMgrT::getSpellTarget(std::vector<float> source, std::vector<float> aim) {
	EnemyGeneric *target = nullptr;

	// get positions
	float px1 = source[0];
	float pz1 = source[2];
	float px2 = aim[0];
	float pz2 = aim[2];

	// get angle & intercept of spell prefilight line
	float ang = (px1 - px2) / (pz1 - pz2);
	float intercept = pz1 - (ang * px1);

	float a = pz2 - pz1;
	float b = px1 - px2;
	float c = pz1 * px2 - pz2 * px1;

	// vector of enemies caught in preflight
	std::vector<std::unique_ptr<EnemyGeneric>*> potentialTargets;

	// get hit enemies
	for (auto& enemy : this->enemies) {
		//if (!enemy->isKilled)
		{
			vector<float> enemyPosition = getWorldPosition(enemy->getBasePtr());

			float epx = enemyPosition[0];
			float epz = enemyPosition[2];

			float dist = abs((a*epx) + (b * epz) + c) / sqrt(pow(a, 2) + pow(b, 2));

			std::cout << enemy->getSpellHitRad() << "dist: " << dist << std::endl;

			if (dist < enemy->getSpellHitRad())
			{
				// catch potential target
				potentialTargets.push_back(&enemy);
				//enemy->hitInstance(140.);

				std::cout << enemy->getSpellHitRad() << std::endl;
			}
		}
	}

	std::cout << "targets: " << potentialTargets.size() << std::endl;

	// find best target
	// if no one caught
	if (potentialTargets.size() <= 0)
	{
		return nullptr;
	}
	// if only one enemy caught
	else if (potentialTargets.size() <= 1)
	{
		return potentialTargets[0];
	}
	// if multiple enemies caught
	else {
		std::unique_ptr<EnemyGeneric> *bestTarget = potentialTargets[0];

		float dist2b = DistanceAtoB(bestTarget->get()->getBasePtr(), &simdata.player.base);

		bool caughtCaster = false;

		for (auto& target : potentialTargets) {
			EnemyGeneric *tmp = target->get();

			if (tmp->getType() == 2 && DistanceAtoB(bestTarget->get()->getBasePtr(), &simdata.player.base) > dist2b)
				// caster && farer than current best target
			{
				bestTarget = target;
				dist2b = DistanceAtoB(bestTarget->get()->getBasePtr(), &simdata.player.base);

				caughtCaster = true;
			}else if(!caughtCaster && DistanceAtoB(target->get()->getBasePtr(), &simdata.player.base) > dist2b)
				// farer than current best target
			{
				bestTarget = target;
				dist2b = DistanceAtoB(bestTarget->get()->getBasePtr(), &simdata.player.base);
			}
		}

		return bestTarget;
	}
}

std::unique_ptr<EnemyGeneric>* EnemyMgrT::checkFSpellHit(ObjDataT *base) {
	for (auto& enemy : this->enemies)
	{
		float dist = DistanceAtoB(base, enemy->getBasePtr());

		if (dist < enemy->getSpellHitRad() && !enemy->isKilled)
		{
			return &enemy;
		}
	}

	return nullptr;
}

void EnemyMgrT::DBGkillEnemy() {
	this->enemies[0]->hitInstance(99999.);
}

void EnemyMgrT::killAll() {
	this->enemies.clear();
}