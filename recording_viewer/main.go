package main

import (
	"fmt"
	"os"

	"github.com/gonutz/prototype/draw"
)

const (
	gameboyWidth  = 160
	gameboyHeight = 144
	pixelSize     = 4
	windowWidth   = 2 * gameboyWidth * pixelSize
	windowHeight  = gameboyHeight * pixelSize
	imageSize     = gameboyWidth * gameboyHeight
	frameSize     = imageSize + NumberOfKeys
)

const (
	KeyRight = iota
	KeyLeft
	KeyUp
	KeyDown
	KeyA
	KeyB
	KeySelect
	KeyStart
	NumberOfKeys
)

func main() {
	if err := run(); err != nil {
		panic(err)
	}
}

func run() error {
	path := "../recording"
	if len(os.Args) == 2 {
		path = os.Args[1]
	}

	f, err := os.Open(path)
	if err != nil {
		return err
	}
	defer f.Close()

	n, err := f.Seek(0, os.SEEK_END)
	if err != nil {
		return err
	}
	frameCount := int(n / frameSize)

	colors := []draw.Color{
		rgb(155, 188, 15),
		rgb(139, 172, 15),
		rgb(48, 98, 48),
		rgb(15, 56, 15),
	}

	var image [gameboyWidth * gameboyHeight]byte
	var keyState [NumberOfKeys]byte

	loadImage := func(frame int) error {
		_, err := f.Seek(int64(frame*frameSize), os.SEEK_SET)
		if err != nil {
			return err
		}

		_, err = f.Read(image[:])
		if err != nil {
			return err
		}

		_, err = f.Read(keyState[:])
		if err != nil {
			return err
		}

		return nil
	}

	currentFrame := 0
	loadImage(currentFrame)

	playing := false

	return draw.RunWindow("Tetris Viewer", windowWidth, windowHeight, func(window draw.Window) {
		if window.WasKeyPressed(draw.KeyEscape) {
			window.Close()
		}

		if window.WasKeyPressed(draw.KeySpace) {
			playing = !playing
		}

		oldFrameIndex := currentFrame

		if playing {
			currentFrame++
		}

		if window.WasKeyPressed(draw.KeyRight) {
			currentFrame++
		}

		if window.WasKeyPressed(draw.KeyLeft) {
			currentFrame--
		}

		if window.WasKeyPressed(draw.KeyDown) {
			currentFrame += 10
		}

		if window.WasKeyPressed(draw.KeyUp) {
			currentFrame -= 10
		}

		if window.WasKeyPressed(draw.KeyPageDown) {
			currentFrame += 1000
		}

		if window.WasKeyPressed(draw.KeyPageUp) {
			currentFrame -= 1000
		}

		if window.WasKeyPressed(draw.KeyHome) {
			currentFrame = 0
		}

		if window.WasKeyPressed(draw.KeyEnd) {
			currentFrame = frameCount - 1
		}

		if window.WasKeyPressed(draw.KeyEnter) {
			for currentFrame+1 < frameCount {
				lastImage := image
				currentFrame++
				loadImage(currentFrame)
				if image != lastImage {
					break
				}
			}
		}

		if window.WasKeyPressed(draw.KeyBackspace) {
			for currentFrame-1 >= 0 {
				lastImage := image
				currentFrame--
				loadImage(currentFrame)
				if image != lastImage {
					break
				}
			}
		}

		if currentFrame < 0 {
			currentFrame = 0
		}
		if currentFrame >= frameCount {
			currentFrame = frameCount - 1
		}

		if currentFrame != oldFrameIndex {
			loadImage(currentFrame)
		}

		// Draw the game boy screen on the left side.
		for y := 0; y < gameboyHeight; y++ {
			for x := 0; x < gameboyWidth; x++ {
				p := colors[image[x+y*gameboyWidth]&3]
				window.FillRect(x*pixelSize, y*pixelSize, pixelSize, pixelSize, p)
			}
		}

		// Draw the state information on the right side.
		offsetX := windowWidth / 2
		buttonSize := windowWidth / 2 / 13
		textScale := float32(pixelSize) / 2.0
		_, textHeight := window.GetScaledTextSize("|", textScale)

		window.FillRect(offsetX, 0, windowWidth, windowHeight, rgb(210, 216, 182))

		text := fmt.Sprintf("Frame %d / %d", currentFrame+1, frameCount)
		window.DrawScaledText(text, offsetX+textHeight, textHeight, textScale, draw.Black)
		if playing {
			window.DrawScaledText("Replaying (SPACE to stop)", offsetX+textHeight, 2*textHeight, textScale, draw.Black)
		} else {
			window.DrawScaledText("SPACE to start replaying", offsetX+textHeight, 2*textHeight, textScale, draw.Black)
		}

		offsetY := windowHeight - 6*buttonSize
		window.FillRect(offsetX+2*buttonSize, offsetY+1*buttonSize, buttonSize, buttonSize, dpadColor(keyState[KeyLeft]))
		window.FillRect(offsetX+3*buttonSize, offsetY+1*buttonSize, buttonSize, buttonSize, dpadColor(0))
		window.FillRect(offsetX+4*buttonSize, offsetY+1*buttonSize, buttonSize, buttonSize, dpadColor(keyState[KeyRight]))
		window.FillRect(offsetX+3*buttonSize, offsetY+0*buttonSize, buttonSize, buttonSize, dpadColor(keyState[KeyUp]))
		window.FillRect(offsetX+3*buttonSize, offsetY+2*buttonSize, buttonSize, buttonSize, dpadColor(keyState[KeyDown]))
		window.FillEllipse(offsetX+8*buttonSize, offsetY+1*buttonSize, buttonSize, buttonSize, abButtonColor(keyState[KeyB]))
		window.FillEllipse(offsetX+10*buttonSize, offsetY+1*buttonSize, buttonSize, buttonSize, abButtonColor(keyState[KeyA]))
		window.FillRect(offsetX+4*buttonSize, offsetY+4*buttonSize, 2*buttonSize, buttonSize, menuButtonColor(keyState[KeySelect]))
		window.FillRect(offsetX+7*buttonSize, offsetY+4*buttonSize, 2*buttonSize, buttonSize, menuButtonColor(keyState[KeyStart]))
	})
}

func rgb(r, g, b byte) draw.Color {
	return draw.RGB(float32(r)/255, float32(g)/255, float32(b)/255)
}

func dpadColor(state byte) draw.Color {
	if state == 0 {
		return draw.Black
	}
	return draw.Gray
}

func abButtonColor(state byte) draw.Color {
	if state == 0 {
		return draw.Red
	}
	return draw.DarkRed
}

func menuButtonColor(state byte) draw.Color {
	if state == 0 {
		return draw.Gray
	}
	return draw.DarkGray
}
