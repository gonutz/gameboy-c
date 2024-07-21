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
	windowWidth   = gameboyWidth * pixelSize
	windowHeight  = gameboyHeight * pixelSize
	imageSize     = gameboyWidth * gameboyHeight
)

func main() {
	if err := run(); err != nil {
		panic(err)
	}
}

func run() error {
	f, err := os.Open("../recording")
	if err != nil {
		return err
	}
	defer f.Close()

	n, err := f.Seek(0, os.SEEK_END)
	if err != nil {
		return err
	}
	frameCount := int(n / imageSize)

	colors := []draw.Color{
		rgb(155, 188, 15),
		rgb(139, 172, 15),
		rgb(48, 98, 48),
		rgb(15, 56, 15),
	}

	var image [gameboyWidth * gameboyHeight]byte

	loadImage := func(frame int) error {
		_, err := f.Seek(int64(frame*imageSize), os.SEEK_SET)
		if err != nil {
			return err
		}
		_, err = f.Read(image[:])
		return err
	}

	currentFrame := 0
	loadImage(currentFrame)

	showFrameNumber := true

	return draw.RunWindow("Tetris Viewer", windowWidth, windowHeight, func(window draw.Window) {
		if window.WasKeyPressed(draw.KeyEscape) {
			window.Close()
		}

		if window.WasKeyPressed(draw.KeySpace) || window.WasKeyPressed(draw.KeyN) {
			showFrameNumber = !showFrameNumber
		}

		oldFrameIndex := currentFrame

		if window.WasKeyPressed(draw.KeyPageDown) {
			currentFrame += 100
		}

		if window.WasKeyPressed(draw.KeyPageUp) {
			currentFrame -= 100
		}

		if window.WasKeyPressed(draw.KeyRight) {
			currentFrame++
		}

		if window.WasKeyPressed(draw.KeyLeft) {
			currentFrame--
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

		for y := 0; y < gameboyHeight; y++ {
			for x := 0; x < gameboyWidth; x++ {
				p := colors[image[x+y*gameboyWidth]&3]
				window.FillRect(x*pixelSize, y*pixelSize, pixelSize, pixelSize, p)
			}
		}

		if showFrameNumber {
			text := fmt.Sprintf("%d / %d", currentFrame+1, frameCount)
			w, h := window.GetTextSize(text)
			window.FillRect(0, 0, w, h, draw.RGBA(0, 0, 0, 0.5))
			window.DrawText(text, 0, 0, draw.White)
		}
	})
}

func rgb(r, g, b byte) draw.Color {
	return draw.RGB(float32(r)/255, float32(g)/255, float32(b)/255)
}
