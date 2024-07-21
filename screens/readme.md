This folder contains screenshots of the different states of the game:

![0](screenshot_000.png) opening credits

![1](screenshot_001.png) player count selection menu

![2](screenshot_002.png) game type selection menu

![3](screenshot_003.png) level selection menu

![4](screenshot_004.png) in game

![5](screenshot_005.png) pause

![6](screenshot_006.png) game ending, screen fills with bricks

![7](screenshot_007.png) game over

![8](screenshot_008.png) enter name for highscore

From these we can see that there are 5 different screens:

1. opening credits (screenshot 0)
2. player count selection menu (screenshot 1)
3. game type selection menu (screenshot 2)
4. level selection menu (screenshots 3 and 8)
5. in game (screenshots 4 to 7)

To find out which of these 5 screens we are in, we want to look at a small set
of pixels whose combination of colors uniquely identifies the current screen.

The program `finger_print.go` helps us find a good set of pixels for this
finger print. It takes as inputs 1) the set of images to differentiate between
and 2) a set of 2D points, the pixel positions to use as a finger print. It
then checks that the finger print for all given images is unique. If unique,
the finger prints are output so we can copy them to our bot code.

`finger_print.go` does not look for a good combination of pixels. That process
happens manually. This tool then helps verify the pixel set and outputs the
finger prints so we do not have to do that manually in an image editor.
