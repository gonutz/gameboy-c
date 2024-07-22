This folder contains screenshots of the different states of the game:

![0](0_screenshot_opening_credits.png) opening credits

![1](1_screenshot_player_count_select.png) player count selection menu

![2](2_screenshot_game_type_select.png) game type selection menu

![3](3_screenshot_level_select.png) level selection menu

![4](4_screenshot_in_game.png) in game

![5](5_screenshot_in_game_2.png) pause

![6](6_screenshot_in_game_3.png) game ending, screen fills with bricks

![7](7_screenshot_in_game_4.png) game over

![8](8_screenshot_level_select_2.png) enter name for highscore

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
