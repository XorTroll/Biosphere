
#pragma once
#include <bio/ipc/ipc_Session.hpp>
#include <bio/os/os_Event.hpp>

namespace bio::ipc
{
    template<typename T>
    class InRaw : public RequestArgument
    {
        public:
            InRaw(T type) : value(type), offset(0)
            {
            }

            virtual void Process(RequestData &data, u8 part) override
            {
                switch(part)
                {
                    case 0:
                        BIO_IPC_PROCESS_TYPE_RAW_OUT(T, data.in_raw_size, offset)
                        break;
                    case 2:
                        *((T*)(((u8*)data.in_raw) + offset)) = value;
                        break;
                }
            }

        private:
            T value;
            u64 offset;
    };

    class InProcessId : public RequestArgument
    {
        public:
            virtual void Process(RequestData &data, u8 part) override;
    };

    template<HandleMode HMode>
    class InHandle : public RequestArgument
    {
        public:
            InHandle(u32 in_handle) : handle(in_handle), mode(HMode)
            {
            }

            virtual void Process(RequestData &data, u8 part) override
            {
                switch(part)
                {
                    case 0:
                        switch(mode)
                        {
                            case HandleMode::Copy:
                                data.in_copy_hs[data.in_copy_hs_size] = handle;
                                data.in_copy_hs_size++;
                                break;
                            case HandleMode::Move:
                                data.in_move_hs[data.in_move_hs_size] = handle;
                                data.in_move_hs_size++;
                                break;
                        }
                        break;
                }
            }

        private:
            u32 handle;
            HandleMode mode;
    };

    template<typename S>
    class InSession : public RequestArgument
    {
        static_assert(std::is_base_of<Session, S>::value, "InSession object must inherit from bio::ipc::Session!");

        public:
            InSession(std::shared_ptr<S> &session) : s(session)
            {
            }

            virtual void Process(RequestData &data, u8 part) override
            {
                switch(part)
                {
                    case 0:
                        if(s->IsDomain() || s->IsDomainSubService())
                        {
                            data.in_object_ids[data.in_object_ids_size] = s->GetObjectId();
                            data.in_object_ids_size++;
                        }
                        break;
                }
            }

        private:
            std::shared_ptr<S> s;
    };

    class InObjectId : public RequestArgument
    {
        public:
            InObjectId(u32 object_id);
            virtual void Process(RequestData &data, u8 part) override;

        private:
            u32 obj_id;
    };

    class InBuffer : public RequestArgument
    {
        public:
            InBuffer(const void *data, size_t size, u32 type);
            virtual void Process(RequestData &data, u8 part) override;

        private:
            Buffer buf;
    };

    class InStaticBuffer : public RequestArgument
    {
        public:
            InStaticBuffer(const void *data, size_t Size, u32 Index);
            virtual void Process(RequestData &data, u8 part) override;

        private:
            Buffer buf;
    };

    class InSmartBuffer : public RequestArgument
    {
        public:
            InSmartBuffer(const void *data, size_t size, u32 index, u64 expected_size);
            virtual void Process(RequestData &data, u8 part) override;

        private:
            Buffer buf_normal;
            Buffer buf_static;
    };

    template<typename T>
    class OutRaw : public RequestArgument
    {
        public:
            OutRaw(T &ref) : value(ref), offset(0)
            {
            }

            virtual void Process(RequestData &data, u8 part) override
            {
                switch(part)
                {
                    case 3:
                        BIO_IPC_PROCESS_TYPE_RAW_OUT(T, data.out_raw_size, offset)
                        break;
                    case 5:
                        value = *((T*)(((u8*)data.out_raw) + offset));
                        break;
                }
            }

        private:
            T &value;
            u64 offset;
    };

    class OutProcessId : public RequestArgument
    {
        public:
            OutProcessId(u64 &out_pid);
            virtual void Process(RequestData &data, u8 part) override;

        private:
            u64 &pid;
    };

    template<u32 OIndex>
    class OutHandle : public RequestArgument
    {
        public:
            OutHandle(u32 &out_h) : handle(out_h), idx(OIndex)
            {
            }

            virtual void Process(RequestData &data, u8 part) override
            {
                switch(part)
                {
                    case 4:
                        if(idx < data.out_hs_size) handle = data.out_hs[idx];
                        break;
                }
            }

        private:
            u32 idx;
            u32 &handle;
    };

    template<u32 OIndex, bool AutoClear>
    class OutEvent : public RequestArgument
    {
        public:
            OutEvent(std::shared_ptr<os::Event> &out_ev) : event(out_ev), idx(OIndex), auto_cl(AutoClear)
            {
            }

            virtual void Process(RequestData &data, u8 part) override
            {
                switch(part)
                {
                    case 4:
                        if(idx < data.out_hs_size)
                        {
                            u32 handle = data.out_hs[idx];
                            event = std::move(os::Event::Open(handle, auto_cl));
                        }
                        break;
                }
            }

        private:
            u32 idx;
            bool auto_cl;
            std::shared_ptr<os::Event> &event;
    };

    template<u32 OIndex>
    class OutObjectId : public RequestArgument
    {
        public:
            OutObjectId(u32 &out_obj_id) : obj_id(out_obj_id), idx(0)
            {
            }

            virtual void Process(RequestData &data, u8 part) override
            {
                switch(part)
                {
                    case 5:
                        if(idx < data.out_object_ids_size) obj_id = data.out_object_ids[idx];
                        break;
                }
            }

        private:
            u32 idx;
            u32 &obj_id;
    };

    template<u32 OIndex, typename S>
    class OutSession : public RequestArgument
    {
        static_assert(std::is_base_of<Session, S>::value, "OutSession object must inherit from bio::ipc::Session!");

        public:
            OutSession(std::shared_ptr<S> &session) : s(session), idx(OIndex)
            {
            }

            virtual void Process(RequestData &data, u8 part) override
            {
                switch(part)
                {
                    case 5:
                        if(data.requester_session_is_domain)
                        {
                            if(idx < data.out_object_ids_size) s = std::make_shared<S>(data.requester_session_handle, data.out_object_ids[idx]);
                        }
                        else
                        {
                            if(idx < data.out_hs_size) s = std::make_shared<S>(data.out_hs[idx]);
                        }
                        break;
                }
            }

        private:
            u32 idx;
            std::shared_ptr<S> &s;
    };

    class OutBuffer : public RequestArgument
    {
        public:
            OutBuffer(const void *data, size_t size, u32 type);
            virtual void Process(RequestData &data, u8 part) override;

        private:
            Buffer buf;
    };

    class OutStaticBuffer : public RequestArgument
    {
        public:
            OutStaticBuffer(const void *data, size_t size, u32 index);
            virtual void Process(RequestData &data, u8 part) override;

        private:
            Buffer buf;
    };

    class OutSmartBuffer : public RequestArgument
    {
        public:
            OutSmartBuffer(const void *data, size_t size, u32 index, u64 expected_size);
            virtual void Process(RequestData &data, u8 part) override;

        private:
            Buffer buf_normal;
            Buffer buf_static;
    };
}