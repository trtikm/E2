import matplotlib.image

img = matplotlib.image.imread("./in.png", "png")

#print(str(img.ndim))
#print(str(img.shape))
for i in range(img.shape[0]):
    for j in range(img.shape[1]):
        img[i, j, 3] = img[i, j, 0]     # copy red-channel to alpha-channel

matplotlib.image.imsave("./out.png", img, format="png")
