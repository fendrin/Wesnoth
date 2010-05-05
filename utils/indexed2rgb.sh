#!/bin/sh
#
#  Script to convert PNGs from indexed to RGB mode.
#
#  Run-time requirements: ImageMagick
#

while [ "${1}" != "" ]; do
    # Rather useless until we start accepting options, but whatever.
    filelist="${filelist} ${1}"
    shift
done

if test -z "$filelist"; then
    filelist=`find -iname "*.png"`
fi

number=`echo $filelist|wc -w`

echo "Converting $number files to RGB mode..."

for f in $filelist
do
    # 6 = RGBA http://www.imagemagick.org/Usage/formats/#png_write
    convert -define png:color-type=6 $f $f.new
    mv -f $f.new $f
done

echo "Done. Now would be a good time to run optipng"

