#include "sim.h"
#include "derivedTypeDef.h"
#include "drawUtility.h"

extern SimDataT simdata;

// prototype
void drawSpell(int affiliation);

Spell::Spell() {
	this->spawnedAt = simdata.frameCount;
	this->isVanishable = false;
}

void Spell::moveGeneral(float speed) {
	// set rotation
	euler_t rotAngle;
	EulerAtoB(&this->base, this->target, &rotAngle);

	this->base.rot = rotAngle;

	this->base.move = speed;

	// apply
	MoveObject(&this->base);

	this->base.pos.y += uniformRandom(-.05, .05);
}

PSpell::PSpell(ObjDataT *target, ObjDataT *source, EnemyGeneric *targetInstance)
	:targetInstance(targetInstance)
{
	setAffiliation(0);

	this->target = target;

	// argument source is not for member variable, it only used to initialize spell position
	this->base.pos = source->pos;
	this->base.pos.y += 2.;
}

void PSpell::update() {
	float dist = DistanceAtoB(&this->base, target);

	if (dist < simdata.config.fieldData.pSpellRad)
	{
		simdata.enemyMgr.registerSpellAttack(this->targetInstance, simdata.difficultyMgr.pBaseAtk * 3);
		this->isVanishable = true;
	}

	moveGeneral(simdata.config.fieldData.pSpellSpeed);

	if (simdata.frameCount - getSpawnTime() > 120)
	{
		this->isVanishable = true;
	}
}

void PSpell::draw() {
	glPushMatrix();
	{
		applyObjTransform(&this->base);
		drawSpell(0);
		//glutSolidSphere(0.4, 4, 4);
	}
	glPopMatrix();
}

ESpell::ESpell(ObjDataT *target, ObjDataT *source){
	setAffiliation(1);

	this->target = target;

	// argument source is not for member variable, it only used to initialize spell position
	this->base.pos = source->pos;
	this->base.pos.y += 1;
}

void ESpell::update() {
	PlayerDataT *pd = &simdata.player;
	ObjectiveDataT *od = &simdata.objective;
	ObjDataT *source = &this->base;

	DifficultyMgrT *dm = &simdata.difficultyMgr;
	GameStateMgrT *gs = &simdata.gameState;

	float dist2o = DistanceAtoB(source, &od->base);
	float dist2p = DistanceAtoB(source, &pd->base);

	if (dist2o < simdata.config.Melee.attackRad)
	{
		od->hp -= dm->ecBaseAtk + gs->ecAtkAdj;
		this->isVanishable = true;
	}

	if (dist2p < simdata.config.Melee.attackRad)
	{
		pd->hp -= dm->ecBaseAtk + gs->ecAtkAdj;
		pd->lastDamaged = simdata.frameCount;
		this->isVanishable = true;
	}

	//float dist = DistanceAtoB(&this->base, target);
	//
	//if (dist < simdata.config.fieldData.eSpellRad)
	//{
	//	PlayerDataT *pd = &simdata.player;
	//
	//	pd->hp -= simdata.difficultyMgr.ecBaseAtk;
	//	this->isVanishable = true;
	//}

	moveGeneral(simdata.config.fieldData.eSpellSpeed);

	if (simdata.frameCount - getSpawnTime() > 1000)
	{
		this->isVanishable = true;
	}
}

void ESpell::draw() {
	glPushMatrix();
	{
		applyObjTransform(&this->base);
		drawSpell(1);
		//glutWireSphere(0.4, 4, 4);
	}
	glPopMatrix();
}

FSpell::FSpell(ObjDataT *source) {
	this->base.pos = source->pos;
	this->base.rot = source->rot;
	this->base.move = simdata.config.fieldData.pSpellSpeed;
}

void FSpell::update() {
	vector_t p = this->base.pos;
	if (p.y < 0 || abs(p.x) > 100 || abs(p.z) > 100)
	{
		this->isVanishable = true;
	}

	if (this->base.pos.y < 5.)
	{
		std::unique_ptr<EnemyGeneric> *hitTarget = simdata.enemyMgr.checkFSpellHit(&this->base);
		if (hitTarget != nullptr)
		{
			simdata.enemyMgr.registerSpellAttack(hitTarget->get(), simdata.difficultyMgr.pBaseAtk*3);
			this->isVanishable = true;
		}
	}

	MoveObject(&this->base);
}

void FSpell::draw() {
	glPushMatrix();
	{
		applyObjTransform(&this->base);
		drawSpell(0);
		//glutSolidSphere(0.4, 4, 4);
	}
	glPopMatrix();
}

//  MANAGER
// ------------------

void SpellMgrT::pSpawn(ObjDataT *target, ObjDataT *source, EnemyGeneric *targetInstance) {
	auto instance = std::make_unique<PSpell>(target, source, targetInstance);
	this->spells.push_back(std::move(instance));
}

void SpellMgrT::eSpawn(ObjDataT *target, ObjDataT *source) {
	auto instance = std::make_unique<ESpell>(target, source);
	this->spells.push_back(std::move(instance));
}

void SpellMgrT::fSpawn(ObjDataT *source) {
	auto instance = std::make_unique<FSpell>(source);
	this->spells.push_back(std::move(instance));
}

void SpellMgrT::update() {
	std::vector<std::unique_ptr<Spell>> *sVec = &this->spells;

	// send back killed instances
	auto killed = std::remove_if(
		sVec->begin(), sVec->end(),
		[](std::unique_ptr<Spell>& spell) { return spell->isVanishable; }
	);

	// erase killed instances
	sVec->erase(killed, sVec->end());

	for (auto& spell : this->spells) {
		spell->update();
	}
}

void SpellMgrT::distractSpell(ObjDataT *source, float radius) {
	for (auto& spell : this->spells) {
		if (DistanceAtoB(spell->getBasePtr(), source) < radius)
		{
			spell->isVanishable = true;
		}
	}
}

void SpellMgrT::draw() {
	for (auto& spell : this->spells) {
		spell->draw();
	}
}

// GRAPHIC
void drawSpell(int affiliation) {
	if (affiliation == 0)// friend
	{
		GLfloat amb[] = { 0.34f,0.71f,0.64f,1.0f };
		GLfloat diff[] = { 0.32f,0.65f,0.6f,1.0f };
		GLfloat spec[] = { 0.7f,0.38f,0.15f,1.0f };
		GLfloat shine = 74.;

		applyMaterial(amb, diff, spec, shine);

		letThereBeLight(.52, 1., .82);
	}
	else {// foe
		GLfloat amb[] = { 0.72f,0.38f,0.46f,1.0f };
		GLfloat diff[] = { 0.76f,0.4f,0.34f,1.0f };
		GLfloat spec[] = { 0.7f,0.38f,0.15f,1.0f };
		GLfloat shine = 74.;

		applyMaterial(amb, diff, spec, shine);

		letThereBeLight(1., .52, .52);
	}
	
	// core
	glutSolidSphere(.2, 6, 6);
	disableEmission();
	
	// outer
	glPushMatrix();
	{
		glRotatef(fmod(simdata.frameCount, 360), 1., 1., 1.);
		glScalef(.6, .6, .6);
		glutWireIcosahedron();
	}
	glPopMatrix();

	// particles
	glPushMatrix();
	{
		glRotatef(90, 1., 0., 0.);
		glRotatef(fmod(simdata.frameCount, 360), 0., 1., 0.);

		for (size_t i = 0; i < 4; i++)
		{
			glPushMatrix();
			{
				// distribute
				glTranslatef(
					sinf((M_PI / 2) * i) * .3,
					fmod(simdata.frameCount, 20) / 10. + (i * .3),
					cosf((M_PI / 2) * i) * .3
				);

				glScalef(.25, .25, .25);
				glutWireIcosahedron();
			}
			glPopMatrix();
		}
	}
	glPopMatrix();
}