
#include <bio/err/err_Assertion.hpp>
using namespace bio;

int main()
{
    err::SetAssertionFunction(err::FatalAssertionFunction);
    err::AssertResult(0xdead);

    return 0;
}