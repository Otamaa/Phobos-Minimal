#pragma once

#include <FileFormats/SHP.h>

#include <vector>

class Blitter;
class RLEBlitter;
class SHPExtData
{
public:

	SHPReference* AttachedToObject;
	SHPReference* AlphaSHP;
	std::vector<Blitter*> Blitters; //??
	std::vector<RLEBlitter*> RLeBlitters; //??

	void EnsureResident(bool verbose);
	bool LoadAlphaImage();

};