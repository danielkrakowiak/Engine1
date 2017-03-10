#pragma once

#include <wrl.h>
#include <vector>
#include <array>

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Query;

namespace Engine1
{
    class Profiler
    {
        public:

        // Note: Main - main image, R - reflection, T - transmission (refraction).
        enum class StageType : int
        {
            Main = 1,
            R = 0b10,
            T = 0b11,
            // Level 2
            RR = 0b100,
            RT = 0b101,
            TR = 0b110,
            TT = 0b111,
            // Level 3
            RRR = 0b1000,
            RRT = 0b1001,
            RTR = 0b1010,
            RTT = 0b1011,
            TRR = 0b1100,
            TRT = 0b1101,
            TTR = 0b1110,
            TTT = 0b1111,
            // Level 4
            RRRR = 0b10000,
            RRRT = 0b10001,
            RRTR = 0b10010,
            RRTT = 0b10011,
            RTRR = 0b10100,
            RTRT = 0b10101,
            RTTR = 0b10110,
            RTTT = 0b10111,
            TRRR = 0b11000,
            TRRT = 0b11001,
            TRTR = 0b11010,
            TRTT = 0b11011,
            TTRR = 0b11100,
            TTRT = 0b11101,
            TTTR = 0b11110,
            TTTT = 0b11111,
            MAX_VALUE = 32
        };

        enum class GlobalEventType : int
        {
            Frame = 0,
            DeferredRendering,
            Bloom,
            CopyFrameToFinalRenderTarget,
            MAX_VALUE
        };

        enum class EventTypePerStage : int
        {
            MipmapGenerationForPositionAndNormals = 0,
            EmissiveShading,
            ReflectionTransmissionShading,
            Raytracing,
            ShadingNoShadows,
            Shading,
            MipmapGenerationForShadedImage,
            CombiningWithMainImage,
            MAX_VALUE
        };

        enum class EventTypePerStagePerLight : int
        {
            ShadowsMapping = 0,
            RaytracingShadows,
            MipmapGenerationForPreillumination,
            MipmapGenerationForIllumination,
            MipmapMinimumValueGenerationForDistanceToOccluder,
            DistanceToOccluderSearch,
            BlurShadows,
            Shadows,
            Shading,
            MAX_VALUE
        };

        static std::string stageTypeToString( const StageType stageType );
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

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device > device, Microsoft::WRL::ComPtr< ID3D11DeviceContext > deviceContext );

        void beginEvent( const GlobalEventType event );
        void endEvent( const GlobalEventType event );

        void beginEvent( const StageType stage, const EventTypePerStage event );
        void endEvent( const StageType stage, const EventTypePerStage event );

        void beginEvent( const StageType stage, const int lightIndex, const EventTypePerStagePerLight event );
        void endEvent( const StageType stage, const int lightIndex, const EventTypePerStagePerLight event );

        void beginFrameProfiling();
        void endFrameProfiling();

        // Useful to pause profiling when high update rate is not needed.
        // Begin/end event calls and are ignored in that state and getters return values for the last measured frame.
        // Call beginFrameProfiling to resume profiling.
        void pauseProfiling();

        float getEventDuration( const GlobalEventType event );
        float getEventDuration( const StageType stage, const EventTypePerStage event );
        float getEventDuration( const StageType stage, const int lightIndex, const EventTypePerStagePerLight event );

        private:

        Event createEvent( ID3D11Device& device );

        Microsoft::WRL::ComPtr< ID3D11Query > createTimestampQuery( ID3D11Device& device );
        Microsoft::WRL::ComPtr< ID3D11Query > createDisjointQuery( ID3D11Device& device );

        void resetEvents( const int frameIdx );

        Microsoft::WRL::ComPtr< ID3D11Device >        m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > m_deviceContext;

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
        static const int queryFrameCount = 3; 

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
                (int)StageType::MAX_VALUE
            >,
            queryFrameCount
        > m_eventsPerStage;

        std::array< 
            std::array< 
                std::array< 
                    std::array< Event, (int)EventTypePerStagePerLight::MAX_VALUE >,
                    s_maxLightCount
                >,
                (int)StageType::MAX_VALUE 
            >,
            queryFrameCount 
        > m_eventsPerStagePerLight;
    };
};

