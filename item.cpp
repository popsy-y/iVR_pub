// include guard
#include "sim.h"
#include "derivedTypeDef.h"
#include "drawUtility.h"

extern SimDataT simdata;
extern PlayerDataT *pd;

DifficultyMgrT *dm = &simdata.difficultyMgr;

ItemsData *im = &simdata.config.itemsData;

void Item::spawn()
{
	std::cout << "Item spawn" << std::endl;

	vector_t initPos = getSpawnLocation(simdata.config.mapData.stageRadius - 5., simdata.config.mapData.minSpawnDistance - 5.);

	this->base.pos.x = initPos.x;
	this->base.pos.y = 1;
	this->base.pos.z = initPos.z;

	this->state = 0;

	this->base.radius = 1;

	this->isVanishable = false;
}

void Item::update() {
	switch (this->state) {
	case 0:// not hold, not touched
		if (isHit(&this->base, &simdata.handR)) {
			setObjColor(&this->base, 0.0, 1.0, 0.5);

			pd->isHoldingItem = true;
			this->holded = true;

			moveWorldToLocal(&this->base, &simdata.handR);

			this->state = 1;
		}
		else {
			setObjColor(&simdata.sphere, 0.7, 0.7, 0.7);
		}
		break;
	case 1: // holding, touching
		if (isHit(&this->base, &simdata.handL)) {
			setObjColor(&this->base, 1.0, 0.5, 0.0);

			pd->isHoldingItem = false;
			this->holded = false;

			moveLocalToWorld(&this->base);

			this->state = 2;
		}
		break;
	case 2:// not hold, touching
		if (!isHit(&this->base, &simdata.handR)) {
			setObjColor(&simdata.sphere, 0.7, 0.7, 0.7);
			this->state = 0;
		}
		break;
	}
}

void Item::postActivate(){
	pd->isHoldingItem = false;

	this->isVanishable = true;
}

void Item::drawGeneral() {
	glPushMatrix();

	applyObjTransform(this->getBasePtr());

	//setObjColor(this->getBasePtr(), 1., 0., 0.);
	applyObjColor(this->getBasePtr());

	// float animation
	glTranslatef(0, sinf(simdata.frameCount/10.)/4., 0.);

	//if (this->state == 0)
	{
		glRotatef(90, 0., 1., 0.);
		glRotatef(simdata.frameCount % 360, 0., 1., 0.);
	}

	float scale = simdata.config.itemsData.modelScale;
	glScalef(scale, scale, scale);

	//glutSolidTorus(1, 1.2, 4, 4);
}

ItemBufAtk::ItemBufAtk() {
	spawn();
}
void ItemBufAtk::activate() {
	GameStateMgrT *gs = &simdata.gameState;
	gs->pAtkAdj = im->buffAtkAmount;
}
void ItemBufAtk::drawUnique() {
	glPopMatrix();
}

ItemHeal::ItemHeal() {
	spawn();
}
void ItemHeal::activate() {
	if (pd->maxHp - pd->hp < im->healHpAmount)
	{
		pd->hp = pd->maxHp;
	}
	else {
		pd->hp += im->healHpAmount;
	}
}
void ItemHeal::drawUnique() {
	{
		GLfloat amb[] = { 0.35f,0.75f,0.44f,1.0f };
		GLfloat diff[] = { 0.59f,0.84f,0.61f,1.0f };
		GLfloat spec[] = { 0.59f,0.55f,0.59f,1.0f };
		GLfloat shine = 42.;

		applyMaterial(amb, diff, spec, shine);
	}

	// body
	glPushMatrix();
	{
		glScalef(1., 3., 1.);
		glutSolidCube(1);
	}
	glPopMatrix();
	glRotatef(90, 0., 0., 1.);
	glPushMatrix();
	{
		glScalef(1., 3., 1.);
		glutSolidCube(1);
	}
	glPopMatrix();

	// shell
	letThereBeLight(.3, .75, .5);

	glPushMatrix();
	{
		glScalef(3., 3., 3.);
		glutWireIcosahedron();
	}
	glPopMatrix();

	disableEmission();

	glPopMatrix();
}

ItemFly::ItemFly() {
	spawn();
}
void ItemFly::activate() {
	pd->stateEnteredAt = simdata.frameCount;

	pd->isCasting = false;
	pd->state = 1;
}
void ItemFly::drawUnique() {
	{
		GLfloat amb[] = { 0.35f,0.6f,0.76f,1.0f };
		GLfloat diff[] = { 0.52f,0.73f,0.84f,1.0f };
		GLfloat spec[] = { 0.59f,0.55f,0.59f,1.0f };
		GLfloat shine = 42.;

		applyMaterial(amb, diff, spec, shine);
	}

	// body
	glPushMatrix();
	{
		glTranslatef(0., -.3, .0);
		glScalef(1., 2., 1.);
		glutSolidCube(1);
	}
	glPopMatrix();
	glPushMatrix();
	{
		glTranslatef(0., .3, 0.);
		glRotatef(-90, 1., 0., 0.);
		glutSolidCone(1.5, 2., 4, 5);
	}
	glPopMatrix();

	// shell
	letThereBeLight(.3, .5, .75);

	glPushMatrix();
	{
		glScalef(3., 3., 3.);
		glutWireIcosahedron();
	}
	glPopMatrix();

	disableEmission();

	glPopMatrix();
}

void ItemsMgrT::instantiate(std::unique_ptr<Item> instance) {
	instance->spawn();

	this->items.push_back(std::move(instance));
}

bool ItemsMgrT::canPopNewItem() {
	return this->items.size() < simdata.config.itemsData.amountLimit;
}

void ItemsMgrT::spawn() {
	std::unique_ptr<Item> instance;

	float itemType = uniformRandom(0, 1);
	if (itemType > .5)
	{
		instance = std::make_unique<ItemFly>();
		this->instantiate(std::move(instance));
	}
	else {
		instance = std::make_unique<ItemHeal>();
		this->instantiate(std::move(instance));
	}
}

void ItemsMgrT::update() {
	std::vector<std::unique_ptr<Item>> *sVec = &this->items;

	// send back killed instances
	auto vanished = std::remove_if(
		sVec->begin(), sVec->end(),
		[](std::unique_ptr<Item>& item) { return item->isVanishable; }
	);

	// erase killed instances
	sVec->erase(vanished, sVec->end());

	for (auto& item : this->items) {
		item->update();
	}
}

void ItemsMgrT::draw() {
	for (auto& item : this->items) {
		item->drawGeneral();
		item->drawUnique();
	}
}

void ItemsMgrT::activate() {
	for (auto& item : this->items) {
		if (item->getHolded())
		{
			item->activate();
			item->postActivate();
			return;
		}
	}
}