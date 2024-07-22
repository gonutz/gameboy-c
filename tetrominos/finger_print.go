//go:build exclude

package main

import (
	"bytes"
	"fmt"
	"image"
	"image/png"
	"os"
)

var markers = []image.Point{
	{X: 2, Y: 1},
	{X: 2, Y: 2},
	{X: 6, Y: 2},
	{X: 3, Y: 3},
}

func main() {
	if err := run(); err != nil {
		panic(err)
	}
}

func run() error {
	args := os.Args[1:]

	images := make([]image.Image, len(args))
	for i := range images {
		var err error
		images[i], err = loadPNG(args[i])
		if err != nil {
			return err
		}
	}

	fingerPrints := make([][]byte, len(images))
	for i := range images {
		fingerPrints[i] = fingerPrint(images[i])
	}

	for i := range fingerPrints {
		for j := i + 1; j < len(fingerPrints); j++ {
			if bytes.Equal(fingerPrints[i], fingerPrints[j]) {
				return fmt.Errorf(
					"%s and %s have the same finger print", args[i], args[j],
				)
			}
		}
	}

	fmt.Printf("finger prints for pixels %v:\n", markers)
	for i := range fingerPrints {
		fmt.Printf("%s: %v\n", args[i], fingerPrints[i])
	}

	return nil
}

func loadPNG(path string) (image.Image, error) {
	f, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer f.Close()

	return png.Decode(f)
}

func fingerPrint(img image.Image) []byte {
	print := make([]byte, len(markers))
	for i, m := range markers {
		r, _, _, _ := img.At(m.X, m.Y).RGBA()
		print[i] = byte(r & 3)
	}
	return print
}
