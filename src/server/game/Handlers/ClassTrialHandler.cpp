/*
 * This file is part of the DestinyCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
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

#include "BattlePayPackets.h"
#include "CharacterPackets.h"
#include "CharacterService.h"
#include "DB2Stores.h"
#include "World.h"

void WorldSession::HandleBattlePayQueryClassTrialResult(WorldPackets::BattlePay::BattlePayQueryClassTrialResult& /*packet*/)
{
    WorldPackets::BattlePay::CharacterClassTrialCreate response;
    response.Result = 0;
    SendPacket(response.Write());
}

void WorldSession::HandleBattlePayTrialBoostCharacter(WorldPackets::BattlePay::BattlePayTrialBoostCharacter& packet)
{
    if (!sWorld->getBoolConfig(CONFIG_CLASS_TRIAL_ENABLED))
        return;

    auto charInfo = sWorld->GetCharacterInfo(packet.Character);
    if (!charInfo)
        return;

    if (charInfo->AccountId != GetAccountId())
        return;

    // Boost to 100 with class trial teleport (all in one transaction)
    uint8 charRace = charInfo->Race;
    bool isAlliance = ((1 << (charRace - 1)) & RACEMASK_ALLIANCE) != 0;

    float x, y, z, o;
    uint16 mapId, zoneId;

    if (isAlliance)
    {
        mapId = 1554; zoneId = 8124; // Sword of Dawn
        x = -2556.0f; y = 2939.6f; z = 135.6f; o = 1.98f;
    }
    else
    {
        mapId = 1557; zoneId = 8422; // Tempest's Roar
        x = 0.721f; y = 1.685f; z = 35.501f; o = 6.2787f;
    }

    sCharacterService->BoostCharacter(this, packet.Character, 100,
        mapId, zoneId, x, y, z, o,
        true, static_cast<uint16>(packet.SpecializationID));

    // Respond to client — trial created OK
    WorldPackets::BattlePay::CharacterClassTrialCreate response;
    response.Result = 0;
    SendPacket(response.Write());

    // Send boost animation flow so client exits "processing" state
    WorldPackets::BattlePay::UpgradeStarted upgradeStarted;
    upgradeStarted.CharacterGUID = packet.Character;
    SendPacket(upgradeStarted.Write());

    // Send equipment items for gear fly-in animation
    uint8 loadoutType = 6; // level 100+ loadout
    WorldPackets::BattlePay::BattlePayCharacterUpgradeQueued upgradeQueued;
    upgradeQueued.Character = packet.Character;
    upgradeQueued.EquipmentItems = sDB2Manager.GetItemLoadOutItemsByClassID(charInfo->Class, loadoutType)[0];
    SendPacket(upgradeQueued.Write());

    // Delay UpgradeComplete + character list refresh to let client play the animation
    ObjectGuid charGuid = packet.Character;
    AddDelayedEvent(3000, [this, charGuid]()
        {
            WorldPackets::BattlePay::UpgradeComplete upgradeComplete;
            upgradeComplete.CharacterGUID = charGuid;
            SendPacket(upgradeComplete.Write());

            SendCharacterEnum();
        });
}