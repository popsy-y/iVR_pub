#include "calc.h"
#include "effects.h"
#include "derivedTypeDef.h"

// RETURNS VECTOR OF RANDOM ANGLES
std::vector<float> initEffectAngles(size_t amount, int angleFrom, int angleTo) {
	std::vector<float> result;

	for (size_t i = 0; i < amount; i++)
	{
		result.push_back(uniformRandom(angleFrom, angleTo));
	}

	return result;
}

// RETURNS VECTOR OF RANDOM VALUES
std::vector<float> initEffectCoords(size_t amount, int from, int to) {
	std::vector<float> result;

	for (size_t i = 0; i < amount; i++)
	{
		result.push_back(uniformRandom(from, to));
	}

	return result;
}

void combinationCastEffect(int type, ObjDataT *source) {
	switch (type)
	{
	case 0:
		// (unused)
		// effect for melee and range combination
		break;
	case 1:
		// (unused)
		// effect for melee and caster combination
		break;
	case 2
		// (unused):
		//effect for range and caster combination
		break;
	default:
		std::cout << "INVALID COMBINATION CAST EFFECT TYPE" << std::endl;
		break;
	}
}

void EMelee::castEffectFunc() {
	// effect for melee normal attack
	std::cout << "Emitting EMelee cast effect" << std::endl;
}

void ERange::castEffectFunc() {
	// effect for range normal attack
	std::cout << "Emitting ERange cast effect" << std::endl;
}

void ECaster::castEffectFunc() {
	// effect for caster normal attack
	std::cout << "Emitting ECaster cast effect" << std::endl;
}


//  PLAYER EFFECTS
// -------------------------

// VARIABLES FOR Player Cast Effect
const int pceParAmount = 100;

std::vector<float> pceParAngles = initEffectAngles(pceParAmount, 0., 360.);
std::vector<float> pceParYs = initEffectCoords(pceParAmount, 0., 2);

// X-Z distance of particles from source
const float pceParDistance = 0.4;

void playerCastEffect(ObjDataT *source) {
	glPushMatrix();
		
	applyObjTransform(source);

		for (auto& elem: pceParYs)
		{
			// increace particle height
			elem += .01;

			// reset particle effect if higher than threshold
			if (elem > 2)
			{
				elem = uniformRandom(0., .15);
			}
		}

		for (size_t i = 0; i < pceParAmount; i++)
		{
			glPushMatrix();
				glTranslatef(sinf(deg2rad(pceParAngles[i])), pceParYs[i], cosf(deg2rad(pceParAngles[i])));
				glutWireSphere(.1, 3, 3);
			glPopMatrix();
		}

	glPopMatrix();
}