#ifndef StatusLightsOutput_h
#define StatusLightsOutput_h

#include "../Effector/Color.h"

namespace StatusLightsOutput
{
    void init();
    bool write(const Color *colors, int colorCount);
}

#endif // StatusLightsOutput_h
