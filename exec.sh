#!/usr/bin/env bash
magick convert -size 2000x1666 -depth 8 R:R.char G:G.char B:B.char -combine -gamma 1.5 assemb.png
sxiv assemb.png
