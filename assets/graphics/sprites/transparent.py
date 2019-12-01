from PIL import Image as img

def transparent(f, c, out):
	im = img.open(f).convert("RGBA")
	pxs = im.load()

	for i in range(im.size[0]):
		for j in range(im.size[1]):
			if pxs[i, j][:-1] == c:
				pxs[i, j] = (0, 0, 0, 0)

	im.save(open(out, "wb"))

def main():
	names = [i for j in range(2) for i in map(lambda x: str(x) + "." + str(j) + str(".png"), range(1, 252))]

	for file in names:
		transparent(file, (0xff, 0xff, 0xff), file)

if __name__ == "__main__":
	main()