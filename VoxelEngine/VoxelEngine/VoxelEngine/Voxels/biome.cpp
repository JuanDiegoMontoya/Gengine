#if 0
#include <block.h>
#include <biome.h>

const Biome& BiomeManager::GetBiome(float temp, float humid, TerrainType terrain)
{
	// track nearest biome to conditions at current block
	std::string currID("error");
	glm::vec2 ogPos(temp, humid);
	float closest = std::numeric_limits<float>::max();
	for (const auto& b : biomes)
	{
		if (b.second.terrain == terrain) // must be matching terrain type
		{
			float dist = glm::distance(ogPos, glm::vec2(b.second.temp_avg, b.second.humidity_avg));
			if (dist <= closest)
			{
				closest = dist;
				currID = b.second.name;
			}
		}
	}
	return biomes[currID];
}

// register hard-coded biomes first, then load custom biomes
void BiomeManager::InitializeBiomes()
{
	// error/fallback biome
	{
		Biome error;
		error.name = "error";
		error.humidity_avg = 9001;
		error.temp_avg = 9001;
		error.terrain = TerrainType::tNone;
		error.surfaceCover = BlockType::bError;
		registerBiome(error);
	}

	// ocean
	{
		Biome ocean(0, 0, TerrainType::tOcean, BlockType::bSand);
		ocean.name = "ocean";
		registerBiome(ocean);
	}

	// woodlands
	{
		Biome woodlands(0, 0, TerrainType::tPlains, BlockType::bGrass);
		woodlands.name = "woodlands";
		woodlands.surfaceFeatures.push_back({ .05f, PrefabName::OakTree });
		registerBiome(woodlands);
	}

	// flat desert
	{
		Biome desertF(-.5f, .5f, TerrainType::tPlains, BlockType::bSand);
		desertF.name = "flat desert";
		registerBiome(desertF);
	}

	// tundra
	{
		Biome tundra(0, -.5f, TerrainType::tPlains, BlockType::bSnow);
		tundra.name = "tundra";
		tundra.surfaceFeatures.push_back({ .0001f, PrefabName::BoulderA });
		tundra.surfaceFeatures.push_back({ .0001f, PrefabName::BoulderB });
		tundra.surfaceFeatures.push_back({ .0001f, PrefabName::BoulderC });
		//tundra.surfaceFeatures.push_back({ .001f, Prefab::BorealTree });
		registerBiome(tundra);
	}

	// snow hills
	{
		Biome snowH(.2f, -.5f, TerrainType::tHills, BlockType::bSnow);
		snowH.name = "snow hills";
		snowH.surfaceFeatures.push_back({ .002f, PrefabName::BorealTree });
		registerBiome(snowH);
	}

	// desert hills
	{
		Biome desertH(-.5f, .5f, TerrainType::tHills, BlockType::bSand);
		desertH.surfaceFeatures.push_back({ .005, PrefabName::Cactus });
		desertH.name = "desert hills";
		registerBiome(desertH);
	}

	// highlands
	{
		Biome highland(.3f, 0.f, TerrainType::tHills, BlockType::bDryGrass);
		highland.name = "highland";
		highland.surfaceFeatures.push_back({ .0005, PrefabName::BoulderA });
		highland.surfaceFeatures.push_back({ .0005, PrefabName::BoulderB });
		highland.surfaceFeatures.push_back({ .0001, PrefabName::BoulderC });
		registerBiome(highland);
	}

	initCustomBiomes();
}

void BiomeManager::registerBiome(const Biome & biome)
{
	for (const auto& str : biome.biomeOverride)
		biomes.erase(str);
	biomes[biome.name] = biome;
}

Biome BiomeManager::loadBiome(std::string name)
{
	return Biome();
}

void BiomeManager::initCustomBiomes()
{
	return;
	// iterate over files in a special directory and add them as biomes
}
#endif