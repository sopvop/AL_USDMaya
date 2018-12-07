//
// Copyright 2018 Animal Logic
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.//
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "AL/usdmaya/nodes/Engine.h"

#include "pxr/imaging/hdx/intersector.h"
#include "pxr/imaging/hdx/taskController.h"

namespace AL {
namespace usdmaya {
namespace nodes {

Engine::Engine(const SdfPath& rootPath,
       const SdfPathVector& excludedPaths) : UsdImagingGLEngine(rootPath, excludedPaths) {}

bool Engine::TestIntersectionBatch(
    const GfMatrix4d &viewMatrix,
    const GfMatrix4d &projectionMatrix,
    const GfMatrix4d &worldToLocalSpace,
    const SdfPathVector& paths,
    UsdImagingGLRenderParams params,
    unsigned int pickResolution,
    PathTranslatorCallback pathTranslator,
    HitBatch *outHit) {
    _UpdateHydraCollection(&_intersectCollection, paths, params, &_renderTags);

    static const HdCullStyle USD_2_HD_CULL_STYLE[] =
        {
            HdCullStyleDontCare,              // No opinion, unused
            HdCullStyleNothing,               // CULL_STYLE_NOTHING,
            HdCullStyleBack,                  // CULL_STYLE_BACK,
            HdCullStyleFront,                 // CULL_STYLE_FRONT,
            HdCullStyleBackUnlessDoubleSided, // CULL_STYLE_BACK_UNLESS_DOUBLE_SIDED
        };
    static_assert(((sizeof(USD_2_HD_CULL_STYLE) /
                    sizeof(USD_2_HD_CULL_STYLE[0]))
                   == static_cast<int>(UsdImagingGLCullStyle::CULL_STYLE_COUNT)),"enum size mismatch");

    HdxIntersector::HitVector allHits;
    HdxIntersector::Params qparams;
    qparams.viewMatrix = worldToLocalSpace * viewMatrix;
    qparams.projectionMatrix = projectionMatrix;
    qparams.alphaThreshold = params.alphaThreshold;
    qparams.cullStyle = USD_2_HD_CULL_STYLE[static_cast<int>(params.cullStyle)];
    qparams.renderTags = _renderTags;
    qparams.enableSceneMaterials = params.enableSceneMaterials;

    _taskController->SetPickResolution(pickResolution);
    if (!_taskController->TestIntersection(
        &_engine,
        _intersectCollection,
        qparams,
        HdxIntersectionModeTokens->unique,
        &allHits)) {
        return false;
    }

    if (!outHit) {
        return true;
    }

    for (const HdxIntersector::Hit& hit : allHits) {
        const SdfPath primPath = hit.objectId;
        const SdfPath instancerPath = hit.instancerId;
        const int instanceIndex = hit.instanceIndex;

        HitInfo& info = (*outHit)[pathTranslator(primPath, instancerPath,
                                                 instanceIndex)];
        info.worldSpaceHitPoint = GfVec3d(hit.worldSpaceHitPoint[0],
                                          hit.worldSpaceHitPoint[1],
                                          hit.worldSpaceHitPoint[2]);
        info.hitInstanceIndex = instanceIndex;
    }

    return true;
}

}
}
}