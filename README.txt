CopyPastris
-----------

by Christiaan Janssen

A game made during the Berlin 5h Microjam on July 2012.
The theme of the jam was "Copy and Paste", and we were
encouraged to reuse assets/code taken from the internet.

I took the idea quite literally, so I decided to make
a tetris clone where the game is played automatically
by the computer, but so badly that the player has to
help by copying/pasting the pieces.

Click on pieces to select them, and use the commands
to cut/copy/paste them to make lines.

But instead of writing the clone myself, I searched
for an implementation of tetris with SDL and hacked
the code.  I found one MIT-licensed implementation
on sourceforge ( http://sourceforge.net/projects/sdltetris/ )
by Dominik Zaczkowski (dmz@pro.wp.pl).  Although
the comments inside the code were written in Polish,
I could still figure out how it was working and
put my hacks in there (although it must be said that
my additions are pure spagghetti code, a hack on
top of a hack on top of a hack).

This version keeps the same license, MIT, except
for the font, which is GPL licensed.

Dependencies
------------

The game requires SDL and SDL_ttf, which you need
to install in your system if you're running the
linux version.  For Windows and Mac, the necessary
libraries are bundled together and it should work
out of the box.


Christiaan Janssen
August 2012
