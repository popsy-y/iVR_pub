// include guard
#pragma once
#include "sim.h"
#include "derivedTypeDef.h"
#include "drawUtility.h"

extern SimDataT simdata;

EMelee::EMelee() {
	EnemyData *cnf = &simdata.config.Melee;

	this->noticeDist = cnf->noticeDist;
	this->attackDist = cnf->attackDist;
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

	this->type = 0;

	std::for_each(this->parDists.begin(), this->parDists.end(), [this](float& e) mutable {
		e = uniformRandom(0., this->attackRad);
	});

	{
		ObjDataT *parent = &this->base;
		ObjDataT *fld = &this->attackField;
		vector_t *parentPos = &parent->pos;
		euler_t *parentRot = &parent->rot;

		fld->pos = { parentPos->x, parentPos->y, parentPos->z + simdata.config.fieldData.meleeRad/2 };
		fld->rot.roll = parentRot->roll;
		fld->rot.pitch = parentRot->pitch;
		fld->rot.yaw = parentRot->yaw;

		//moveWorldToLocal(fld, parent);
	}
}

void EMelee::cast() {
	{
		GLfloat amb[] = { 0.72f,0.38f,0.46f,1.0f };
		GLfloat diff[] = { 0.76f,0.4f,0.34f,1.0f };
		GLfloat spec[] = { 0.7f,0.38f,0.15f,1.0f };
		GLfloat shine = 74.;

		applyMaterial(amb, diff, spec, shine);
	}

	glRotatef(90., 1., 0., 0.);

	for (size_t i = 0; i < this->parAngs.size(); i++)
	{
		glPushMatrix();

		float xOfst = 1. * sinf(this->parAngs[i]);
		float zOfst = 1. * cosf(this->parAngs[i]);

		this->parDists[i] -= .02;
		if (abs(this->parDists[i]) > this->attackRad)
		{
			this->parDists[i] = 0;
		}

		glTranslatef(xOfst, this->parDists[i], zOfst);
		glRotatef(90, 1., 0., 0.);

		glutWireCone(.1, .2, 4, 4);

		glPopMatrix();
	}
}

void EMelee::attack() {
}

void EMelee::draw() {
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

		// body
		// core
		letThereBeLight(1., .52, .52);
		glutSolidSphere(.2, 6, 6);
		disableEmission();

		// outer
		glPushMatrix();
		{
			glRotatef(fmod(simdata.frameCount, 360), 1., 1., 1.);
			glScalef(.6, .9, .6);
			glutWireCube(1);
		}
		glPopMatrix();

		// hands
		glPushMatrix();
		{
			int hands = 6;

			// rotate if casting or attacking
			if (this->getState() == 1 || this->getState() == 2)
			{
				glRotatef(90., 1., 0., 0.);

				// general rotation
				glRotatef(fmod(simdata.frameCount * 20, 360), 0., 1., 0.);
			}
			else
			{
				// general rotation
				glRotatef(fmod(simdata.frameCount, 360), 0., 1., 0.);
			}


			for (size_t i = 0; i < hands; i++)
			{
				glPushMatrix();
				{
					// distribute
					glTranslatef(
						sinf((M_PI * 2) * (i / float(hands))) * .8,
						0.,
						cosf((M_PI * 2) * (i / float(hands))) * .8
					);

					// punch animtion
					//if (this->getState() == 2)
					//{
					//	glTranslatef(0., float(this->attackRad - ((this->attackRad * ((this->getStateEnteredAt() - simdata.frameCount)) / this->attackLength))), 0.);
					//}

					glutSolidSphere(.15, 4, 4);
				}
				glPopMatrix();
			}
		}
		glPopMatrix();

		// effect
		if (this->getState() == 1)
		{
			this->cast();
		}

		if (showBounding)
		{
			//glutWireSphere(this->spellHitRad, 4, 4);
		}
	}
	glPopMatrix();
}

void EMelee::onAttackEnter() {
	this->attackTarget = this->target;

	simdata.enemyMgr.registerEMeleeAttack(&this->base, this->attackRad);
}