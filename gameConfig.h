#pragma once
#include <array>

struct EnemyData
{
	float noticeDist, attackDist, minimumDist, moveSpeed, health, spellHitRad;
	int castLength, attackLength, attackInterval, hitLength, killedLength;
	float attackRad;
	std::array<float, 4> clr0, clr1, clr2, clr3, clr4;
};

struct AttackFieldData {
	float meleeRad, areaRad, spellRad;

	float pSpellSpeed, pSpellRad, pMeshRad, eSpellSpeed, eSpellRad, eMeshRad;
};

struct ItemsData {
	float flyHeight, flyLength;

	float healHpAmount, buffAtkAmount, buffDefAmount;

	int amountLimit;

	float modelScale;
};

struct MapData {
	float stageRadius;
	float minSpawnDistance;
};

struct GameConfig
{
	EnemyData Melee;
	EnemyData Caster;
	EnemyData Range;
	AttackFieldData fieldData;
	ItemsData itemsData;
	MapData mapData;
};