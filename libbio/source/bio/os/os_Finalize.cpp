#include <bio/os/os_Finalize.hpp>
#include <vector>

namespace bio::os
{
    static std::vector<ResourceFinalizeFunction> _inner_FinalizeFunctions;

    void AddFinalizeFunction(ResourceFinalizeFunction func)
    {
        _inner_FinalizeFunctions.push_back(func);
    }

    void CallFinalizeFunctions()
    {
        for(u32 i = 0; i < _inner_FinalizeFunctions.size(); i++) (_inner_FinalizeFunctions[i])();
    }
}