#ifndef SPENCE_EVENTMANAGER_H
#define SPENCE_EVENTMANAGER_H

#include "SFMLRenderer.h"

class SFMLEventManager {
public:
	SFMLEventManager(SFMLRenderer& renderer);
	bool update();

private:
	SFMLRenderer& renderer;
};


#endif //SPENCE_EVENTMANAGER_H
