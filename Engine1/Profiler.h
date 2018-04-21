#pragma once

#include <wrl.h>
#include <vector>
#include <array>

#include "RenderingStage.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;
struct ID3D11Query;

namespace Engine1
{
    class Profiler
    {
        public:

        enum class GlobalEventType : int
        {
            Frame = 0,
            DeferredRendering,
            ASSAO,
            Bloom,
            ToneMapping,
            CalculateLuminance,
            Antialiasing,
            MAX_VALUE
        };

        enum class EventTypePerStage : int
        {
            MipmapGenerationForPositionAndNormals = 0,
            EmissiveShading,
            ReflectionTransmissionShading,
            RaytracingReflectedRefractedRays,
            ShadingNoShadows,
            Shading,
            MipmapGenerationForLayer,
            CombiningWithPreviousLayer,
            HitDistanceMipmapGeneration,
            HitDistanceSearch,
            MAX_VALUE
        };

        enum class EventTypePerStagePerLight : int
        {
            ShadowsMapping = 0,
            RaytracingShadows,
            MipmapGenerationForPreillumination,
            MipmapGenerationForShadows,
            MipmapGenerationForDistanceToOccluder,
            DistanceToOccluderSearch,
            BlurShadowPattern,
            BlurShadows,
            CombineShadowLayers,
            Shadows,
            Shading,
            MAX_VALUE
        };

        static std::string eventTypeToString( const GlobalEventType eventType );
        static std::string eventTypeToString( const EventTypePerStage eventType );
        static std::string eventTypeToString( const EventTypePerStagePerLight eventType, const int lightIdx = -1 );

        static const int s_maxLightCount = 4;

        struct Event
        {
            bool                                  measured;
            Microsoft::WRL::ComPtr< ID3D11Query > queryBegin;
            Microsoft::WRL::ComPtr< ID3D11Query > queryEnd;
            float                                 durationMilliseconds;
        };

        Profiler();
        ~Profiler();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > deviceContext );

        void beginEvent( const GlobalEventType event );
        void endEvent( const GlobalEventType event );

        void beginEvent( const RenderingStage stage, const EventTypePerStage event );
        void endEvent( const RenderingStage stage, const EventTypePerStage event );

        void beginEvent( const RenderingStage stage, const int lightIndex, const EventTypePerStagePerLight event );
        void endEvent( const RenderingStage stage, const int lightIndex, const EventTypePerStagePerLight event );

        void beginFrameProfiling();
        void endFrameProfiling();

        // Useful to pause profiling when high update rate is not needed.
        // Begin/end event calls and are ignored in that state and getters return values for the last measured frame.
        // Call beginFrameProfiling to resume profiling.
        void pauseProfiling();

        float getEventDuration( const GlobalEventType event );
        float getEventDuration( const RenderingStage stage, const EventTypePerStage event );
        float getEventDuration( const RenderingStage stage, const int lightIndex, const EventTypePerStagePerLight event );

        private:

        Event createEvent( ID3D11Device3& device );

        Microsoft::WRL::ComPtr< ID3D11Query > createTimestampQuery( ID3D11Device3& device );
        Microsoft::WRL::ComPtr< ID3D11Query > createDisjointQuery( ID3D11Device3& device );

        void resetEvents( const int frameIdx );

        Microsoft::WRL::ComPtr< ID3D11Device3 >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > m_deviceContext;

        bool m_profilingPaused;

        // Saving query results should happen to frame N, reading results from frame N - 1 
        // (so reading works even in the middle of profiling - between calls to beginFrameProfiling and endFrameProfiling)
        // and submitting a query should happen to frame N - 2 (to not overwrite the results which are being read).
        // Plus there could be some extra frames to avoid reading query results too early - when GPU hasn't finished them yet.
        int m_currentSubmitQueryFrameIndex;
        int m_currentSaveResultsFrameIndex; // If negative - nothing is read - to avoid reading query results in the first few frames.
        int m_currentReadResultsFrameIndex;

        // Number of queries per single event - more than 1 needed because of double or triple buffering on GPU.
        // We can't read query results at the same frame in which we issued them, because GPU renders frames with a delay.
        // Reading them in the same frame would synchronize CPU and GPU causing a performance drop.
        static const int queryFrameCount = 4; 

        // Disjoint query tells whether GPU clock frequency has changed during the frame. If so, timestamp queries from that frame have to be ignored.
        std::array< Microsoft::WRL::ComPtr< ID3D11Query >, queryFrameCount > m_disjointQueries;

        std::array< 
            std::array< 
                Event, 
                (int)GlobalEventType::MAX_VALUE 
            >,
            queryFrameCount 
        > m_globalEvents;

        std::array<
            std::array<
                std::array<
                    Event,
                    (int)EventTypePerStage::MAX_VALUE
                >,
                (int)RenderingStage::MAX_VALUE
            >,
            queryFrameCount
        > m_eventsPerStage;

        std::array< 
            std::array< 
                std::array< 
                    std::array< Event, (int)EventTypePerStagePerLight::MAX_VALUE >,
                    s_maxLightCount
                >,
                (int)RenderingStage::MAX_VALUE 
            >,
            queryFrameCount 
        > m_eventsPerStagePerLight;
    };
};

