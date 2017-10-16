#pragma once
#include "Subsystems\IUpdateSystem.h"
#include "DataStructures\peVector.h"

#include <atomic>

namespace pe
{
    //! Default implementation of the update system
    class peUpdateSystem : public IUpdateSystem
    {
    public:
                                                   peUpdateSystem();
                                                   ~peUpdateSystem();

        void                                       Init() override;
        inline bool                                IsRunning() const override { return _isRunning; }
        void                                       Run() override;
        void                                       Shutdown() override;
        void                                       Stop() override;
        inline void                                Update(double deltaTime) override {} //Update not needed!

        void                                       DeregisterUpdateCallback(const CallbackID& callbackID) override;
        CallbackID                                 RegisterUpdateCallback(std::function<void(float)> callback) override;
    private:
        peVector<std::function<void(float)>>       _callbacks;
        bool                                       _isRunning;
    };

}