-- Q: 40077
UPDATE `quest_objectives` SET `ObjectID` = '244898' WHERE `quest_objectives`.`ID` = 280292 AND `QuestID` = 40077;
REPLACE INTO `gameobject_quest_visual` (`goID`, `questID`, `incomplete_state_spell_visual`, `incomplete_state_world_effect`, `complete_state_spell_visual`, `complete_state_world_effect`, `Comment`)
 VALUES ('244898', '40077', '37794', '0', '8743', '0', 'Legion. Q: 40077');
UPDATE `gameobject_template` SET `ScriptName` = 'go_q40077' WHERE `gameobject_template`.`entry` = 244898;
UPDATE `creature_template` SET `ScriptName` = 'npc_q40077' WHERE `creature_template`.`entry` = 93011;