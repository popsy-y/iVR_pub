// include guard
#pragma once
#include "sim.h"
#include "derivedTypeDef.h"
#include "drawUtility.h"

extern SimDataT simdata;

ECaster::ECaster() {
	EnemyData *cnf = &simdata.config.Caster;

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

	this->type = 2;

	std::for_each(this->parDists.begin(), this->parDists.end(), [this](float& e) mutable {
		e = uniformRandom(0., 15.);
	});
}

void ECaster::cast() {
	{
		GLfloat amb[] = { 0.72f,0.38f,0.46f,1.0f };
		GLfloat diff[] = { 0.76f,0.4f,0.34f,1.0f };
		GLfloat spec[] = { 0.7f,0.38f,0.15f,1.0f };
		GLfloat shine = 74.;

		applyMaterial(amb, diff, spec, shine);
	}

	for (size_t i = 0; i < this->parAngs.size(); i++)
	{
		glPushMatrix();

		float xOfst = parDists[i] * sinf(this->parAngs[i]);
		float zOfst = parDists[i] * cosf(this->parAngs[i]);

		this->parDists[i] -= .01;
		if (this->parDists[i] <= .1)
		{
			this->parDists[i] = 15.;
		}

		glTranslatef(xOfst, this->parDists[i] / 7, zOfst);
		glRotatef(simdata.frameCount, 1., 1., 1.);

		glutWireSphere(.1, 4, 4);

		glPopMatrix();
	}
}

void ECaster::attack() {
}

void ECaster::draw() {
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

		//body

		// core
		letThereBeLight(1., .52, .52);
		glutSolidSphere(.2, 6, 6);
		disableEmission();

		// outer
		glPushMatrix();
		{
			if (this->getState() == 1 || this->getState() == 2) {
				glRotatef(fmod(simdata.frameCount, 360) * 5, 0., 1., 0.);
			}
			else {
				glRotatef(fmod(simdata.frameCount, 360), 0., 1., 0.);
			}
			glScalef(.4, .4, .4);
			glutWireDodecahedron();
		}
		glPopMatrix();

		if (this->getState() == 1)
		{
			this->cast();
		}

		if (showBounding)
		{
			glutWireSphere(this->spellHitRad, 4, 4);
		}
	}
	glPopMatrix();
}

void ECaster::onAttackEnter() {
	AttackFieldData *cnf = &simdata.config.fieldData;
	this->attackTarget = this->target;

	simdata.spellMgr.eSpawn(this->attackTarget, &this->base);
}