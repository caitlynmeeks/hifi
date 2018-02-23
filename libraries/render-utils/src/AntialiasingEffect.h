//
//  AntialiasingEffect.h
//  libraries/render-utils/src/
//
//  Created by Raffi Bedikian on 8/30/15
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AntialiasingEffect_h
#define hifi_AntialiasingEffect_h

#include <DependencyManager.h>

#include "render/DrawTask.h"
#include "DeferredFrameTransform.h"
#include "VelocityBufferPass.h"


class JitterSampleConfig : public render::Job::Config {
    Q_OBJECT
        Q_PROPERTY(float scale MEMBER scale NOTIFY dirty)
        Q_PROPERTY(bool freeze MEMBER freeze NOTIFY dirty)
        Q_PROPERTY(bool stop MEMBER stop NOTIFY dirty)
        Q_PROPERTY(int index READ getIndex NOTIFY dirty)
public:
    JitterSampleConfig() : render::Job::Config(true) {}

    float scale{ 0.5f };
    bool stop{ false };
    bool freeze{ false };

    void setIndex(int current);

public slots:
    int cycleStopPauseRun();
    int prev();
    int next();
    int none();
    int pause();
    int play();

    int getIndex() const { return _index; }
    int getState() const { return _state; }
signals:
    void dirty();

private:
    int _state{ 0 };
    int _index{ 0 };

};


class JitterSample {
public:

    enum {
        SEQUENCE_LENGTH = 128 
    };

    using Config = JitterSampleConfig;
    using JobModel = render::Job::Model<JitterSample, Config>;

    void configure(const Config& config);
    void run(const render::RenderContextPointer& renderContext);

private:

    struct SampleSequence {
        SampleSequence();

        glm::vec2 offsets[SEQUENCE_LENGTH + 1];
        int sequenceLength{ SEQUENCE_LENGTH };
        int currentIndex{ 0 };
    };

    SampleSequence _sampleSequence;
    float _scale{ 1.0 };
    bool _freeze{ false };
};


class AntialiasingConfig : public render::Job::Config {
    Q_OBJECT
    Q_PROPERTY(float blend MEMBER blend NOTIFY dirty)
    Q_PROPERTY(float sharpen MEMBER sharpen NOTIFY dirty)
    Q_PROPERTY(float covarianceGamma MEMBER covarianceGamma NOTIFY dirty)
 
    Q_PROPERTY(bool constrainColor MEMBER constrainColor NOTIFY dirty)
    Q_PROPERTY(bool covarianceClipColor MEMBER covarianceClipColor NOTIFY dirty)
    Q_PROPERTY(bool clipExactColor MEMBER clipExactColor NOTIFY dirty)
    Q_PROPERTY(bool feedbackColor MEMBER feedbackColor NOTIFY dirty)

    Q_PROPERTY(bool debug MEMBER debug NOTIFY dirty)
    Q_PROPERTY(float debugX MEMBER debugX NOTIFY dirty)
    Q_PROPERTY(float debugFXAAX MEMBER debugFXAAX NOTIFY dirty)
    Q_PROPERTY(float debugShowVelocityThreshold MEMBER debugShowVelocityThreshold NOTIFY dirty)
    Q_PROPERTY(bool showCursorPixel MEMBER showCursorPixel NOTIFY dirty)
    Q_PROPERTY(glm::vec2 debugCursorTexcoord MEMBER debugCursorTexcoord NOTIFY dirty)
    Q_PROPERTY(float debugOrbZoom MEMBER debugOrbZoom NOTIFY dirty)

    Q_PROPERTY(bool showClosestFragment MEMBER showClosestFragment NOTIFY dirty)

public:
    AntialiasingConfig() : render::Job::Config(true) {}

    float blend{ 0.05f };
    float sharpen{ 0.15f };

    bool constrainColor{ true };
    bool covarianceClipColor{ true };
    float covarianceGamma{ 0.9f };
    bool clipExactColor{ false };
    bool feedbackColor{ false };

    float debugX{ 0.0f };
    float debugFXAAX{ 1.0f };
    float debugShowVelocityThreshold{ 1.0f };
    glm::vec2 debugCursorTexcoord{ 0.5f, 0.5f };
    float debugOrbZoom{ 2.0f };

    bool debug { false };
    bool showCursorPixel { false };
    bool showClosestFragment{ false };

signals:
    void dirty();
};

#define SET_BIT(bitfield, bitIndex, value) bitfield = ((bitfield) & ~(1 << (bitIndex))) | ((value) << (bitIndex))
#define GET_BIT(bitfield, bitIndex) ((bitfield) & (1 << (bitIndex)))

struct TAAParams {
    float nope{ 0.0f };
    float blend{ 0.05f };
    float covarianceGamma{ 1.0f };
    float debugShowVelocityThreshold{ 1.0f };

    glm::ivec4 flags{ 0 };
    glm::vec4 pixelInfo{ 0.5f, 0.5f, 2.0f, 0.0f };
    glm::vec4 regionInfo{ 0.0f, 0.0f, 1.0f, 0.0f };

    void setConstrainColor(bool enabled) { SET_BIT(flags.y, 1, enabled); }
    bool isConstrainColor() const { return (bool)GET_BIT(flags.y, 1); }

    void setCovarianceClipColor(bool enabled) { SET_BIT(flags.y, 2, enabled); }
    bool isCovarianceClipColor() const { return (bool)GET_BIT(flags.y, 2); }

    void setClipExactColor(bool enabled) { SET_BIT(flags.y, 3, enabled); }
    bool isClipExactColor() const { return (bool)GET_BIT(flags.y, 3); }

    void setFeedbackColor(bool enabled) { SET_BIT(flags.y, 4, enabled); }
    bool isFeedbackColor() const { return (bool)GET_BIT(flags.y, 4); }

    void setDebug(bool enabled) { SET_BIT(flags.x, 0, enabled); }
    bool isDebug() const { return (bool) GET_BIT(flags.x, 0); }

    void setShowDebugCursor(bool enabled) { SET_BIT(flags.x, 1, enabled); }
    bool showDebugCursor() const { return (bool)GET_BIT(flags.x, 1); }

    void setDebugCursor(glm::vec2 debugCursor) { pixelInfo.x = debugCursor.x; pixelInfo.y = debugCursor.y; }
    glm::vec2 getDebugCursor() const { return glm::vec2(pixelInfo.x, pixelInfo.y); }
    
    void setDebugOrbZoom(float orbZoom) { pixelInfo.z = orbZoom; }
    float getDebugOrbZoom() const { return pixelInfo.z; }

    void setShowClosestFragment(bool enabled) { SET_BIT(flags.x, 3, enabled); }

};
using TAAParamsBuffer = gpu::StructBuffer<TAAParams>;

class Antialiasing {
public:
    using Inputs = render::VaryingSet4 < DeferredFrameTransformPointer, gpu::FramebufferPointer, LinearDepthFramebufferPointer, VelocityFramebufferPointer > ;
    using Config = AntialiasingConfig;
    using JobModel = render::Job::ModelI<Antialiasing, Inputs, Config>;

    Antialiasing();
    ~Antialiasing();
    void configure(const Config& config);
    void run(const render::RenderContextPointer& renderContext, const Inputs& inputs);

    const gpu::PipelinePointer& getAntialiasingPipeline();
    const gpu::PipelinePointer& getBlendPipeline();
    const gpu::PipelinePointer& getDebugBlendPipeline();

private:

    // Uniforms for AA
    gpu::int32 _texcoordOffsetLoc;

    gpu::FramebufferSwapChainPointer _antialiasingBuffers;
    gpu::TexturePointer _antialiasingTextures[2];

    gpu::PipelinePointer _antialiasingPipeline;
    gpu::PipelinePointer _blendPipeline;
    gpu::PipelinePointer _debugBlendPipeline;

    TAAParamsBuffer _params;
    float _sharpen{ 0.15f };
    int _sharpenLoc{ -1 };
};


/*
class AntiAliasingConfig : public render::Job::Config {
    Q_OBJECT
    Q_PROPERTY(bool enabled MEMBER enabled)
public:
    AntiAliasingConfig() : render::Job::Config(true) {}
};

class Antialiasing {
public:
    using Config = AntiAliasingConfig;
    using JobModel = render::Job::ModelI<Antialiasing, gpu::FramebufferPointer, Config>;
    
    Antialiasing();
    ~Antialiasing();
    void configure(const Config& config) {}
    void run(const render::RenderContextPointer& renderContext, const gpu::FramebufferPointer& sourceBuffer);
    
    const gpu::PipelinePointer& getAntialiasingPipeline();
    const gpu::PipelinePointer& getBlendPipeline();
    
private:
    
    // Uniforms for AA
    gpu::int32 _texcoordOffsetLoc;
    
    gpu::FramebufferPointer _antialiasingBuffer;
    
    gpu::TexturePointer _antialiasingTexture;
    
    gpu::PipelinePointer _antialiasingPipeline;
    gpu::PipelinePointer _blendPipeline;
    int _geometryId { 0 };
};
*/

#endif // hifi_AntialiasingEffect_h
