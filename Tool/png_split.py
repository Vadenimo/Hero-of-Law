import sys
from PIL import Image, ImageDraw, ImageFont

def split(input, output, basename, count):
	img = Image.open(input)
	
	w, h = img.size
	slice = int(h / count)
	
	fs = min(slice, w) - 4
	
	if True == True:
		left = 0
		top = 0
		right = w
		bot = slice
		
		first:Image.Image = img.crop((left, top, right, bot))
		other:Image.Image = img.crop((left, top, right, bot))
		
		font = ImageFont.load_default(fs)
		draw = ImageDraw.Draw(other)
		tl, tt, tr, tb  = draw.textbbox((0, 0), 'A', font=font)
		
		draw.text((
				(w / 2) - (tr / 2), 
				0,
			), 
			text='A', 
			fill="#FFFFFF80", 
			font=font, 
			align='center',
			stroke_width=2, 
			stroke_fill="#00000080")
		
		first = Image.blend(first, other, 0.5)
		
		first.save(output + '/' + basename + '_0000.png')
		
	
	for i in range(count):
		left = 0
		top = i * slice
		right = w
		bot = top + slice
		
		img.crop((left, top, right, bot)).save(output + '/' + basename + f'_{i + 1:04d}' + '.png')
		
split(sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4]))