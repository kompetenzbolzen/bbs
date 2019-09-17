PICDIR=~/pics

echo Your screen width?
read width

while true 
do
	read -p " > " input
	case $input in
		[qQ]*) exit;;
		*);;
	esac

	files=($PICDIR/*)
	FILE=${files[RANDOM % ${#files[@]}]}
	magick convert $FILE -depth 24 bmp:- | asciimap -wic -s $width
done
