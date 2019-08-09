
#pragma once
#include <bio/bio_Types.hpp>
#include <memory>

namespace bio::os
{
    class Event
    {
        public:
            Event(bool auto_clear, u32 w_handle, u32 r_handle);
            ~Event();
            bool IsValid();
            bool AutoClears();
            void SetAutoClear(bool auto_clear);
            Result Wait(u64 timeout);
            Result Fire();
            Result Clear();
            void Close();

            static ResultWith<std::shared_ptr<Event>> Create(bool auto_clear);
            static std::shared_ptr<Event> Open(u32 handle, bool auto_clear);
        private:
            bool cauto;
            u32 whandle;
            u32 rhandle;
    };
}