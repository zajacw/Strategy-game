# Strategy-game

A strategy game inspired by fantastic Heroes of Might and Magic 3, implemented in C++ with use of Allegro 4 gaming library.
This project is playable, however unfinished due to lack of further support of Allegro 4 (Allegro 5 is a lot different).

The goal of this game is to build your own army, by raising buildings in various towns; levelling your hero and equiping better weapons to fight and clear all the camps (which difficulty depends on your hero's level) scattered throughout the map. Clearing the campsite provide exp and resoruces which can be spent on building or recuiting. Beside gold for recruitment hero needs also leadership points (which are gained by level-up) so it is impossible to have too strong army for its level.

Most of the graphics were taken from HoMM3 and https://wall.alphacoders.com

The program has built-in music player for playing mp3 (play next, previous or shuffle). It is easy to include your own music playlist into the game, as it loads .mp3. For my gameplay testing I used breathtaking works of Two Steps From Hell.

Maps are build in txt files in smiliar way to my platform game project. All necessary stats for units, weapons, towns and music are also stored in txt files making it easier to develop the game further.

The fighting system was inspired by d20 and Dungeons&Dragons mechanics. Unfortunatelly I haven't provide graphical battle system, so it is dice-rolling and battling automatically.

The game is saved by going back to main menu (pressing ESC).

The mouse controls also haven't been programmed, so the only way (right now) to build, shop and recruit is only by typing commands.
Yes, the commandline (and console) is included, cheats can be provided.
