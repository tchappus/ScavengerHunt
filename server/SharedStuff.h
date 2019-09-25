#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <string>
#include <locale>
#include "SharedObject.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "Game.h"

class SharedStuff {
public:
    bool run;
    Sync::ThreadSem* currentgames_sem;
    std::vector<Game*> currentgames;
    Sync::SocketServer* socketServer;
        
};
