#ifndef __DERIVEDTYPEDEF_H__
#define __DERIVEDTYPEDEF_H__

#include "../lib_ObjLoader/ObjLoader_Seq.h"
#include "object.h"
#include "effects.h"
#include <array>
#include <list>

#include "sim.h"

#define VECTORMODE

using namespace objloader;

//extern SimDataT simdata;

// UTILITY FOR SPAWN
vector_t getSpawnLocation(float floorRadius, float minSpawnDistance);

// ------------------
//  PLAYER
// ------------------

//-------- player data
typedef struct {

	ObjDataT base;

	bool isCasting, isAttracting, isSpellCasting, isHoldingItem;
	bool isFlying;// DEPRECATED //
	float maxHp, hp;
	
	float skillTriggerDeadZone;

	float baseMoveRate, baseTurnRate;

	char lastUsedSkill[64];

	int lastDamaged;

	int state, stateEnteredAt;

} PlayerDataT;


// ------------------
//  ENEMIES
// ------------------

class EnemyGeneric {
	// logical state variables
	int state;
	int stateEnteredAt;

protected:
	int type;

	// physical ability
	float noticeDist, attackDist, minimumDist;
	float moveSpeed; // units / fr
	float health;
	float spellHitRad;
	int castLength, attackLength, attackInterval, hitLength, killedLength, attackedAt;
	float attackRad;
	ObjDataT base;

	// cognition
	ObjDataT *target;

	// behavior
		// specific state actions
			// general
	void moveGeneral();
	void applyRotation();
	void hitReactGeneral();
	void killedReactGeneral();

		// specific state enter actions
	virtual void onAttackEnter() = 0;

			// unique
	virtual void cast() = 0;
	virtual void attack() = 0;

	// color
	std::array<std::array<float, 4>, 5> clrs;
	void setAryObjClr(std::array<float, 4> clrAry);

	// attack field data
	ObjDataT attackField;
	ObjDataT *attackTarget;

	// effect particle data
	std::array<float, 20> parAngs;
	std::array<float, 20> parDists;

	// constructor
	EnemyGeneric();
public:
	ObjDataT* getBasePtr() { return &this->base; }
	int getState() { return this->state; }
	int getStateEnteredAt() { return this->stateEnteredAt; }
	int getType() { return this->type; }
	float getSpellHitRad() { return this->spellHitRad; }

	// spawn
	void spawn();

	// state tree
	void update();

	virtual void draw() = 0;

	bool isKilled;
	bool isVanishable;

	// interaction
	std::vector<float> getPosV3(); // for collision caliculation
	void hitInstance(float damageAmount);
};

class EMelee : public EnemyGeneric {
protected:
	void castEffectFunc();
public:
	EMelee();

	void cast() override;
	void attack() override;
	void draw() override;

	void onAttackEnter() override;
};

class ERange : public EnemyGeneric {
protected:
	void castEffectFunc();

	int unitsAmount;
	std::vector<float> unitHeights;
	void getUnitHeights();
public:
	ERange();

	void cast() override;
	void attack() override;
	void draw() override;

	void onAttackEnter() override;
};

class ECaster : public EnemyGeneric {
protected:
	void castEffectFunc();
public:
	ECaster();

	void cast() override;
	void attack() override;
	void draw() override;

	void onAttackEnter() override;
};

//-------- enemy manager
struct EnemyMgrT {
private:
	void instantiate(std::unique_ptr<EnemyGeneric> instance);

public:

	int combinationAttackState, combinationAttackType;

	std::vector<std::unique_ptr<EnemyGeneric>> enemies;

	void registerMeleeAttack(ObjDataT *source, float range, float damage);
	void registerAreaAttack(ObjDataT *source, float range, float damage);
	void registerSpellAttack(EnemyGeneric *target, float damage);
	std::unique_ptr<EnemyGeneric>* getSpellTarget(std::vector<float> source, std::vector<float> aim);// preflight

	void registerEMeleeAttack(ObjDataT *source, float range);
	void registerEAreaAttack(ObjDataT *source, float range, int units);

	std::unique_ptr<EnemyGeneric>* checkFSpellHit(ObjDataT *base);

	void spawn();

	void update();

	void draw();

	void DBGkillEnemy();

	void killAll();

	// info
	void registerVanished() { destroyedEnemies++; std::cout << "vanished: " << destroyedEnemies << std::endl; }
	int destroyedEnemies = 0;
};

// ------------------
//  SPELLS
// ------------------

//-------- spell data
typedef struct {

	float pSpellSpeed, pSpellRadius, pMeshRadius;
	CObjLoader_Seq pSpellMesh;

	float eSpellSpeed, eSpellRadius, eMeshRadius;
	CObjLoader_Seq eSpellMesh;

} SpellDataT;

class Spell {
	int spawnedAt;
	int affiliation;
protected:
	int getSpawnTime() { return this->spawnedAt; }
	void setAffiliation(int aff) { this->affiliation = aff; }

	ObjDataT base;
	ObjDataT *target;
	void moveGeneral(float speed);
public:
	Spell();

	ObjDataT* getBasePtr() { return &this->base; }

	virtual void update() = 0;

	virtual void draw() = 0;

	bool isVanishable;
};

class PSpell : public Spell{
protected:
	EnemyGeneric *targetInstance;
public:
	PSpell(ObjDataT *target, ObjDataT *source, EnemyGeneric *targetInstance);

	void update() override;

	void draw() override;
};

class ESpell : public Spell {
public:
	ESpell(ObjDataT *target, ObjDataT *source);

	void update() override;

	void draw() override;
};

class FSpell : public Spell {
public:
	FSpell(ObjDataT *source);

	void update() override;
	void draw() override;
};

struct SpellMgrT {
	std::vector<std::unique_ptr<Spell>> spells;

	void pSpawn(ObjDataT *target, ObjDataT *source, EnemyGeneric *targetInstance);
	void eSpawn(ObjDataT *target, ObjDataT *source);
	void fSpawn(ObjDataT *source);

	void distractSpell(ObjDataT *source, float radius);

	void update();
	void draw();
};

// ------------------
//  ITEMS
// ------------------

class Item {
	ObjDataT base;
	int state;

	int activatedAt;

	bool holded;
public:
	void spawn();

	bool getHolded() { return this->holded; }

	ObjDataT *getBasePtr() { return &this->base; }

	void update();

	virtual void activate() = 0;
	void postActivate();

	void drawGeneral();
	virtual void drawUnique() = 0;

	bool isVanishable;
};

class ItemBufAtk : public Item {
public:
	ItemBufAtk();

	void activate() override;
	void drawUnique() override;
};

class ItemHeal : public Item {
public:
	ItemHeal();

	void activate() override;
	void drawUnique() override;
};

class ItemFly : public Item {
public:
	ItemFly();

	void activate() override;
	void drawUnique() override;
};

struct ItemsMgrT{
private:
	void instantiate(std::unique_ptr<Item> instance);
public:
	const int itemTypes = 4;

	std::vector<std::unique_ptr<Item>> items;

	bool canPopNewItem();

	void update();

	void draw();

	void spawn();

	void activate();
};


// ------------------
//  OBJECTIVE
// ------------------

typedef struct {
	
	ObjDataT base;

	float maxHp;
	float hp;

} ObjectiveDataT;


// -----------------------
//  BASE DIFFICULTY
// -----------------------

typedef struct {
	float pBaseAtk;
	float emBaseAtk;// enemy melee base atk
	float ecBaseAtk;// enemy caster base atk
	float erBaseAtk;// enemy area base atk (per unit)
	int erSwarmAmount;// enemy area swarm amount

	int swarmUnitsDestroyedByAttack;// amount of enemy area swarm units that destroyed by single attack

	int effectLength;// effect length of items
	
	int enemyAmount;
	int enemyAmountAdjRate;

	float pAtkAdjRate;

	float emAtkAdjRate;// enemy melee atk adjust rate (per level)
	float erAtkAdjRate;// enemy range atk adjust rate (per level)(per unit)
	float ecAtkAdjRate;// enemy caster atk adjust rate (per level)
} DifficultyMgrT;


// ----------------------
//  GAME STATE
// ----------------------

typedef struct {
	bool isGameOver;

	int level;

	float objectiveForeHp, playerForeHp;

	int enemyAmountAdj = 0;
	
	float pAtkAdj = 0;
	
	float emAtkAdj = 0;// enemy melee atk adjust
	float erAtkAdj = 0;// enemy range atk adjust (per unit)
	float ecAtkAdj = 0;// enemy caster atk adjust
}GameStateMgrT;

#endif // !_DERIVEDTYPEDEF_H_

#ifdef VECTORMODE
#define OBJ_X(obj) ((obj)->pos.x)
#define OBJ_Y(obj) ((obj)->pos.y)
#define OBJ_Z(obj) ((obj)->pos.z)
#define OBJ_ROLL(obj)  ((obj)->rot.roll)
#define OBJ_PITCH(obj) ((obj)->rot.pitch)
#define OBJ_YAW(obj)   ((obj)->rot.yaw)
#else
#define OBJ_X(obj) ((obj)->x)
#define OBJ_Y(obj) ((obj)->y)
#define OBJ_Z(obj) ((obj)->z)
#define OBJ_ROLL(obj)  ((obj)->roll)
#define OBJ_PITCH(obj) ((obj)->pitch)
#define OBJ_YAW(obj)   ((obj)->yaw)
#endif
