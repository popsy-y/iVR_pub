// include guard
#pragma once
#include "sim.h"
#include "derivedTypeDef.h"
#include "drawUtility.h"

extern SimDataT simdata;

ERange::ERange() {
	EnemyData *cnf = &simdata.config.Range;

	this->noticeDist = cnf->noticeDist;
	this->attackDist = cnf->attackDist;
	this->minimumDist = cnf->minimumDist;
	this->moveSpeed = cnf->moveSpeed;
	this->health = cnf->health;
	this->spellHitRad = cnf->spellHitRad;

	this->castLength = cnf->castLength;
	this->attackLength = cnf->attackLength;
	this->attackInterval = cnf->attackInterval;
	this->hitLength = cnf->hitLength;
	this->killedLength = cnf->killedLength;

	this->attackRad = cnf->attackRad;

	this->clrs = { cnf->clr0, cnf->clr1, cnf->clr2, cnf->clr3, cnf->clr4 };

	this->type = 1;

	std::for_each(this->parDists.begin(), this->parDists.end(), [this](float& e) mutable {
		e = uniformRandom(0., this->attackRad);
	});

	for (size_t i = 0; i < simdata.difficultyMgr.erSwarmAmount; i++)
	{
		this->unitHeights.push_back(uniformRandom(0., 1.));
	}
}

void ERange::getUnitHeights() {
	
}

void ERange::cast() {
	{
		GLfloat amb[] = { 0.72f,0.38f,0.46f,1.0f };
		GLfloat diff[] = { 0.76f,0.4f,0.34f,1.0f };
		GLfloat spec[] = { 0.7f,0.38f,0.15f,1.0f };
		GLfloat shine = 74.;

		applyMaterial(amb, diff, spec, shine);
	}

	glRotatef(-90., 1., 0., 0.);
	float scl = (sinf(simdata.frameCount) + .5);

	for (size_t i = 0; i < this->parAngs.size(); i++)
	{
		glPushMatrix();

		float xOfst = .4 * sinf(this->parAngs[i]);
		float zOfst = .4 * cosf(this->parAngs[i]);

		glTranslatef(xOfst, this->parDists[i], zOfst);
		glRotatef(simdata.frameCount * 10, 1., 1., 1.);

		
		glScalef(scl, scl, scl);

		glutWireSphere(.1, 4, 4);

		glPopMatrix();
	}
}

void ERange::attack() {
	
}

void ERange::draw() {
	int unitsAmount = this->unitHeights.size();// NEED TO FIX
	int drawUnits;
	if (unitsAmount * this->health / simdata.config.Range.health > 0)
	{
		drawUnits = unitsAmount * this->health / simdata.config.Range.health;
	}
	else {
		drawUnits = 0;
	}

	glPushMatrix();
	{
		applyObjTransform(this->getBasePtr());

		// color
		if (this->getState() == 3 || this->getState() == 4)
		{
			GLfloat amb[] = { 0.72f,0.38f,0.46f,1.0f };
			GLfloat diff[] = { 0.76f,0.4f,0.34f,1.0f };
			GLfloat spec[] = { 0.7f,0.38f,0.15f,1.0f };
			GLfloat shine = 74.;

			applyMaterial(amb, diff, spec, shine);
		}
		else {
			GLfloat amb[] = { 0.15f,0.08f,0.22f,1.0f };
			GLfloat diff[] = { 0.27f,0.12f,0.25f,1.0f };
			GLfloat spec[] = { 0.59f,0.55f,0.59f,1.0f };
			GLfloat shine = 74.;

			applyMaterial(amb, diff, spec, shine);
		}

		// core
		letThereBeLight(1., .52, .52);
		glutSolidSphere(.3, 4, 4);
		disableEmission();

		// children
		for (size_t i = 0; i < drawUnits; i++)
		{
			glPushMatrix();
			
			float xOffset = 0.5 * sinf(((M_PI * 2) / unitsAmount) * i + deg2rad(simdata.frameCount % 360) * 2);
			float zOffset = 0.5 * cosf(((M_PI * 2) / unitsAmount) * i + deg2rad(simdata.frameCount % 360) * 2);

			glTranslatef(xOffset, this->unitHeights[i], zOffset);

			if (this->getState() == 1 || this->getState() == 2)
			{
				glRotatef(rad2deg(((M_PI * 2) / unitsAmount)), 0., 1., 0.);
			}

			glutWireCone(.2, .2, 4, 4);
			glPopMatrix();
		}

		if (showBounding)
		{
			glutWireSphere(this->spellHitRad, 4, 4);
		}

		if (this->getState() == 1)
		{
			this->cast();
		}
	}
	glPopMatrix();
}

void ERange::onAttackEnter() {
	AttackFieldData *cnf = &simdata.config.fieldData;
	this->attackTarget = this->target;

	int unitsAmount = this->unitHeights.size();
	int remainingUnits;
	if (unitsAmount * this->health / simdata.config.Range.health > 0)
	{
		remainingUnits = unitsAmount * this->health / simdata.config.Range.health;
	}
	else {
		remainingUnits = 0;
	}

	simdata.enemyMgr.registerEAreaAttack(&this->base, this->attackRad, remainingUnits);
}