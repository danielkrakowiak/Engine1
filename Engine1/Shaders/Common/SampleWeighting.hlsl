
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

// Returns higher weight for samples which are close to the center of ellipse. 
// Weight decreases similar to Gaussian function and reaches zero at ellipse circumference.
// Note: All arguments are squared values.
float getSampleWeightGaussianEllipse( 
    const float samplePosXSqr, 
    const float samplePosYSqr,
    const float ellipseVertRadiusSqr,
    const float ellipseHorzRadiusSqr)
{
    const float ellipseCircumferenceYSqr = ellipseVertRadiusSqr - ( samplePosXSqr * ellipseVertRadiusSqr / ellipseHorzRadiusSqr ); 
    const float ellipseRadiusSqr         = samplePosXSqr + ellipseCircumferenceYSqr;
    const float distanceInPixelsSqr      = samplePosXSqr + samplePosYSqr;

    // Note: Gaussian function approximated with cosine.
    const float weight = ( 1.0 + cos( Pi * min(1.0, distanceInPixelsSqr / ellipseRadiusSqr))) / 2.0;

    return weight;
}
