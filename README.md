Mode
====

GBA re-make of AdamAtomic's MODE

MODE 0
Zachary Tarvit

Story: You're a spaceman, to your left you see that you've got a gun. 
The surrounding area is strange, and you don't remember how you got here,
nor do you really see a way out. You notice on some of the walls there
are huge panels of some sort. They open and close on what seem to be a set
rythm. Your thoughts are interrupted by the sound of an alarm, and this
time from the panels emerge semi-sentient robotic crafts, and they're
aimed at you.

In-Game Controls:
The D-Pad moves your character as you'd expect, the A button
jumps, and the B button shoots.
You can shoot downward for a small boost to your jump height(per shot).
Use this to get to ledges that seem otherwise impossible to jump to.

Features:
Lots of sprites!
Lots of sprites colliding with a tilemap!
Space man!
Gun!

Seriously though:
-Intro screen using palette shifting for some fading effects, both for
	the fade-in effect, and the flashing text effect.
-MODE startup screen with a neat-o 120 particle explosion and screen
	flash effect using the palette shifting.
-In-game map is randomly generated, and has not been impossible in any
	of my own tests.
-Things that are killed spew lots of particles that interact with the
	base tilemap, and bounce around.
-Animated character based on several conditions (i.e. walking while
	you are and aren't holding up, and tapping repeatedly does
	not interrupt the animation's progress, only changes it)
-Animated unit spawners. These create little bots that try to fly to you
-Bots with the worst AI you've never heard of. They're affine sprites though,
	but with no way to get the angle between the bot and the player...
	it's pretty ugly.
-Made in about two days... More on that below.

Hurdles:
-No atan2 usable on the Gameboy. This made the bot AI from the original
	game not viable to re-build at all.
-TIME. FUCK.

Geez, I wish I had more time for this. I've been sick ever since we
got back from thanksgiving break. I'm talking every day I've had a 
fever, and Thursday night it spiked up to 103.2, very unfun. So I've
sort of been out of it for a while. I had assumed that what I had would
last like 3-4 days at worst, so I rested Mon-Wed, and then on Thursday
I went all out and worked on this for 7 or 8 hours, same with today
though I think I've been working today for 8-10. Still, I did what
I could with what life decided to punch me with, and I like what I 
finally ended up with.

Oh, there's also no way to lose currently. Normally it would be by
having your score drain to 0 by either time, or getting hit. just..
no time.

(I would have liked to have had sound though, but there's not enough
time to learn how to do audio mixing and implement it in a couple hours.)
