/*
 * Copyright (C) 2011-2012 DarkCore <http://www.darkpeninsula.eu/>
 * Copyright (C) 2011-2012 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2012 MaNGOS <http://getmangos.com/> *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "Creature.h"
#include "MapManager.h"
#include "RandomMovementGenerator.h"
#include "ObjectAccessor.h"
#include "Map.h"
#include "Util.h"
#include "CreatureGroups.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"

#define RUNNING_CHANCE_RANDOMMV 20                                  //will be "1 / RUNNING_CHANCE_RANDOMMV"

#ifdef MAP_BASED_RAND_GEN
#define rand_norm() creature.rand_norm()
#endif

template<>
void RandomMovementGenerator<Creature>::SetRandomLocation(Creature &creature)
{
    float respX, respY, respZ, respO, destX, destY, destZ, travelDistZ;
    creature.GetHomePosition(respX, respY, respZ, respO);
    Map const* map = creature.GetBaseMap();

    // For 2D/3D system selection
    bool isAirOk = creature.canFly();

    const float angle = float(rand_norm()) * static_cast<float>(M_PI*2.0f);
    const float range = float(rand_norm()) * _wanderDistance;
    const float distanceX = range * cos(angle);
    const float distanceY = range * sin(angle);

    destX = respX + distanceX;
    destY = respY + distanceY;

    // prevent invalid coordinates generation
    Darkcore::NormalizeMapCoord(destX);
    Darkcore::NormalizeMapCoord(destY);

    travelDistZ = distanceX*distanceX + distanceY*distanceY;

    if (isAirOk)                                          // 3D system above ground and above water (flying mode)
    {
        // Limit height change
        const float distanceZ = float(rand_norm()) * sqrtf(travelDistZ)/2.0f;
        destZ = respZ + distanceZ;
        float levelZ = map->GetWaterOrGroundLevel(destX, destY, destZ-2.0f);

        // Problem here, we must fly above the ground and water, not under. Let's try on next tick
        if (levelZ >= destZ)
            return;
    }
    else                                                    // 2D only
    {
        // 10.0 is the max that vmap high can check (MAX_CAN_FALL_DISTANCE)
        travelDistZ = travelDistZ >= 100.0f ? 10.0f : sqrtf(travelDistZ);

        // The fastest way to get an accurate result 90% of the time.
        // Better result can be obtained like 99% accuracy with a ray light, but the cost is too high and the code is too long.
        destZ = map->GetHeight(destX, destY, respZ+travelDistZ-2.0f, false);

        if (fabs(destZ - respZ) > travelDistZ)              // Map check
        {
            // Vmap Horizontal or above
            destZ = map->GetHeight(destX, destY, respZ - 2.0f, true);

            if (fabs(destZ - respZ) > travelDistZ)
            {
                // Vmap Higher
                destZ = map->GetHeight(destX, destY, respZ+travelDistZ-2.0f, true);

                // let's forget this bad coords where a z cannot be find and retry at next tick
                if (fabs(destZ - respZ) > travelDistZ)
                    return;
            }
        }
    }

    if (isAirOk)
        _nextMoveTime.Reset(0);
    else
        _nextMoveTime.Reset(urand(500, 10000));

    creature.AddUnitState(UNIT_STATE_ROAMING_MOVE);

    Movement::MoveSplineInit init(creature);
    init.MoveTo(destX, destY, destZ);
    init.SetWalk(true);
    init.Launch();

    //Call for creature group update
    if (creature.GetFormation() && creature.GetFormation()->getLeader() == &creature)
        creature.GetFormation()->LeaderMoveTo(destX, destY, destZ);
}

template<>
void RandomMovementGenerator<Creature>::Initialize(Creature &creature)
{
    if (!creature.isAlive())
        return;

    if (!_wanderDistance)
        _wanderDistance = creature.GetRespawnRadius();

    creature.AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
    SetRandomLocation(creature);
}

template<>
void
RandomMovementGenerator<Creature>::Reset(Creature &creature)
{
    Initialize(creature);
}

template<>
void RandomMovementGenerator<Creature>::Interrupt(Creature &creature)
{
    creature.ClearUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
    creature.SetWalk(false);
}

template<>
void RandomMovementGenerator<Creature>::Finalize(Creature &creature)
{
    creature.ClearUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
    creature.SetWalk(false);
}

template<>
bool
RandomMovementGenerator<Creature>::Update(Creature &creature, uint32 const diff)
{
    if (creature.HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
    {
        _nextMoveTime.Reset(0);  // Expire the timer
        creature.ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }

    if (creature.movespline->Finalized())
    {
        _nextMoveTime.Update(diff);
        if (_nextMoveTime.Passed())
            SetRandomLocation(creature);
    }
    return true;
}

template<>
bool RandomMovementGenerator<Creature>::GetResetPosition(Creature &creature, float& x, float& y, float& z)
{
    float radius;
    creature.GetRespawnPosition(x, y, z, NULL, &radius);

    // use current if in range
    if (creature.IsWithinDist2d(x, y, radius))
        creature.GetPosition(x, y, z);

    return true;
}
