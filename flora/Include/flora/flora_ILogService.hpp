 
#pragma once
#include <switch.h>
#include <stratosphere.hpp>
#include <flora/flora_Log.hpp>

namespace flora
{
    class ILogService final : public IServiceObject
    {
        private:

            enum class CommandId
            {
                Initialize = 0,
                WriteOut = 1,
                WriteErr = 2,
            };

            Result Initialize(PidDescriptor pid);

            Result Write(u32 type, InBuffer<char> log);
            Result WriteOut(InBuffer<char> log);
            Result WriteErr(InBuffer<char> log);

        public:
        
            DEFINE_SERVICE_DISPATCH_TABLE
            {
                MAKE_SERVICE_COMMAND_META(ILogService, Initialize),
                MAKE_SERVICE_COMMAND_META(ILogService, WriteOut),
                MAKE_SERVICE_COMMAND_META(ILogService, WriteErr),
            };

        private:

            u64 app_id;
            bool initialized = false;
    };
}