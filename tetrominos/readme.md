This folder contains screenshots of the different playing pieces, called
tetromios, of the game, and their starting positions.

There are 7 basic shapes, named by the letter they resemble:

```
                                       I
 J    L                                I
 J    L      SS    ZZ     OO    TTT    I
JJ    LL    SS      ZZ    OO     T     I

 J    L      S      Z     O      T     I
```

Then there are 5 special tiles:

- The empty tile.
- The flickering tile when a line is completed.
- The brick tile that fills the screen once the game is over.
- We select one tile of the pause screen to identify it.
- We select one tile of the game over screen to identify it.

To reconstruct the current set of tiles from the screen, we want to look at a
small set of pixels whose combination of colors uniquely identifies a tile.

The program `finger_print.go` helps us find a good set of pixels for this
finger print. It takes as inputs 1) the set of images to differentiate between
and 2) a set of 2D points, the pixel positions to use as a finger print. It
then checks that the finger print for all given images is unique. If unique,
the finger prints are output so we can copy them to our bot code.

`finger_print.go` does not look for a good combination of pixels. That process
happens manually. This tool then helps verify the pixel set and outputs the
finger prints so we do not have to do that manually in an image editor.
