#pragma once
#include "NPC.h"

// Commander is no longer used in this version.
// Kept as a stub for project file compatibility.
class CommanderNPC : public NPC {
public:
    CommanderNPC(double positionX, double positionY, char character, int team, int type, NPC** teamNPC)
        : NPC(positionX, positionY, character, team, type) {}
    void DoSomeWork() override {}
};
