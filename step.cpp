#include "step.h"

step::step()
    : ally_id(-1), enemy_id(-1)
{
}

step::step(int ally_id, int enemy_id)
    : ally_id(ally_id), enemy_id(enemy_id)
{
}