static const float epsilon = 0.0001f;
static const float e       = 2.71828f;

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
    return pow( e, -samplesDifference / threshold );
}
