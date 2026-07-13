-- replace Russian text for quest 42421 (The Nightfallen)
UPDATE quest_request_items SET CompletionText = 'Assist us, and we will reward you.' WHERE ID = 42421;
-- replace Russian text for quest 41989 (Blood of My Blood)
UPDATE page_text SET Text = 'Ran\'thos Lunastre$B$BHead of House Lunastre. Father of Ly\'leth and Anarys.$B$BDied in honorable service to Grand Magistrix Elisande.' WHERE ID = 5279;
-- replace Russian text for random statue 243559 (Statue of Liftbrul)
UPDATE page_text SET Text = 'Liftbrul, greatest of the weightlifters ("No, scratch that part out!") among all drogbar, champion of the Stonedark.\n\nImmortalized in stone by chief Rynox, second-strongest drogbar of his time ("What are you writing there, Stonecarver?").\n\nThis is not a statue, it is Liftbrul, Rynox is a Stoneshaper ("Does it say something nice about me?").' WHERE ID = 5158;

-- reduce Bristlefur Bear (96146) gold drop was 171504, which is correct according
-- to wowhead but cannot find evidence that wowhead is correct
UPDATE creature_template SET mingold = 0, maxgold = 0 WHERE entry = 96146;
