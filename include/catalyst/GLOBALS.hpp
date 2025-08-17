#include <string>
#define STRINGIFY_IMPL(x) #x
#define TOSTRING(x) STRINGIFY_IMPL(x)

namespace catalyst {
#ifdef CATALYST_BUILD_SYS
    constexpr std::string CATALYST_VERSION = TOSTRING(CATALYST_PROJ_VER);
#else
    constexpr std::string CATALYST_VERSION = "0.0.2-dev";
#endif
};
