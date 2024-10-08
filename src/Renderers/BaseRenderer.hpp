﻿//
// Created by carlo on 2024-10-07.
//

#ifndef BASERENDERER_HPP
#define BASERENDERER_HPP

namespace RENDERERS
{
    class BaseRenderer
    {
    public:
        virtual ~BaseRenderer() = default;
        virtual void RecreateSwapChainResources() = 0;
        virtual void RenderFrame(ENGINE::InFlightQueue* inflightQueue) = 0;
        virtual void ReloadShaders() = 0;
    };
}
#endif //BASERENDERER_HPP