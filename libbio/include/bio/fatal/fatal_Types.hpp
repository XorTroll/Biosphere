
#pragma once
#include <bio/bio_Types.hpp>

namespace bio::fatal
{
    enum class ThrowMode
    {
        ErrorReportAndErrorScreen,
        ErrorReport,
        ErrorScreen,
    };
}