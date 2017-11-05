
#include "Common\Constants.hlsl"

// Returns 1 if sample value is below given threshold, returns 0 otherwise.
float getSampleWeightLowerThan( const float sample, const float threshold )
{
    if ( sample < threshold )
        return 1.0f;
    else
        return 0.0f;
}

// Returns 1 if sample value is above given threshold, returns 0 otherwise.
float getSampleWeightGreaterThan( const float sample, const float threshold )
{
    if ( sample > threshold )
        return 1.0f;
    else
        return 0.0f;
}

// Returns higher weight for similar samples (smaller difference) - based on Gaussian curve.
float getSampleWeightSimilarSmooth( const float samplesDifference, const float threshold )
{
    // Note: Squaring may not be neeeded - it's just to flatten the curve near zero difference.
    //const float samplesDifferenceSquared = samplesDifference * samplesDifference;

    return pow( e, -samplesDifference / threshold );
}
