#ifndef STEP_H
#define STEP_H

struct step
{
    step();
    step(int ally_id, int enemy_id);
    int ally_id;
    int enemy_id;
};

#endif