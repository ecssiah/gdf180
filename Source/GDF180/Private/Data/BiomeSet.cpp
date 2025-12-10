#include "BiomeSet.h"

UBiomeSet::UBiomeSet()
	:
	bDebugBiomes { false },
	BiomePeriod { 10000.0f },
	BiomeDefinitionArray { 
		{
			{
				TEXT("b0_grass"),
				{ 0.0f, 1.0f, 0.0f, 1.0f },
			},
			{
				TEXT("b1_moss"),
				{ 0.0f, 0.5f, 0.2f, 1.0f },
			},
			{
				TEXT("b2_lichen"),
				{ 0.2f, 0.2f, 0.2f, 1.0f },
			},
			{
				TEXT("b3_redsands"),
				{ 0.8f, 0.0f, 0.2f, 1.0f },
			},
			{
				TEXT("b4_mountain"),
				{ 0.8f, 0.8f, 0.8f, 1.0f },
			},
			{
				TEXT("b5_snow"),
				{ 1.0f, 1.0f, 1.0f, 1.0f },
			},
		} 
	}
{}

float UBiomeSet::GetFrequency() const
{
	return 1.0f / BiomePeriod;
}

