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


#ifndef VMAPEXPORT_H
#define VMAPEXPORT_H

enum ModelFlags
{
    MOD_M2 = 1,
    MOD_WORLDSPAWN = 1<<1,
    MOD_HAS_BOUND = 1<<2
};

extern const char * szWorkDirWmo;

#endif