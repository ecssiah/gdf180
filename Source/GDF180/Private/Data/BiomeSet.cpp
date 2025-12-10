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
	},
	RingDefinitionArray {
		{
			TEXT("Ring 0"),
			0.0f,
			5000.0f,
			{
				{ 0, 1.0f },
			}
		},
		{
			TEXT("Ring 1"),
			5000.0f,
			10000.0f,
			{
				{ 0, 0.5f },
				{ 1, 0.5f },
			}
		},
		{
			TEXT("Ring 2"),
			10000.0f,
			60000.0f,
			{
				{ 2, 0.25f },
				{ 3, 0.25f },
				{ 4, 0.25f },
				{ 5, 0.25f },
			}
		}
	}
{}

float UBiomeSet::GetFrequency() const
{
	return 1.0f / BiomePeriod;
}

const FRingDefinition& UBiomeSet::GetRingDefinition(const float Radius) const
{
	const FRingDefinition* RingDefinition { 
		RingDefinitionArray.FindByPredicate(
			[&](const FRingDefinition& Candidate)
			{
				return Radius >= Candidate.InnerRadius &&
					Radius < Candidate.OuterRadius;
			}
		) 
	};

	if (RingDefinition)
	{
		return *RingDefinition;
	}
	
	return RingDefinitionArray.Last();
}

