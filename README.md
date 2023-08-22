<div align="center">
 
# PRE Project : OPENGL with pix2pix

</div>

This project is made in 2023 summer PRE program.
You can run in windows OS.

You can get input image data with project.sln 
AND
You can train and deeplearning with pix2pix, so that you can generate new output.

Input image data is hard shadow (shadow mapping) and soft shadow (pcss).

<img width="80%" src="https://github.com/ParkJaeSeong98/PRE-OPENGL/assets/48883899/23641b1e-b610-4f3d-92aa-1c92c2664e47">


Project folder is C++ solution folder using OPENGL.
- you can make input data with this program.
- you have to modify PATH values.
- you can change scene by modifying global variable 'sceneCounter' (1, 2, 3)

cgan.py can be executed in jupyter notebook.
- you have to modify PATH values.
- you can to modify buffer_size, batch_size, steps as you want.

<img width="80%" src="https://github.com/ParkJaeSeong98/PRE-OPENGL/assets/48883899/fca347eb-933e-4559-9ae4-e5cbc15810b3">


## 📖 Libraries

project.sln

 |libraries|description|
 |---|-----|
 |[**GLAD**](https://github.com/Dav1dde/glad)|We can use opengl API using GLAD.|
 |[**GLFW**](https://github.com/glfw/glfw)|We can control window.|
 |[**GLM**](https://github.com/g-truc/glm)|Mathematical functions like matrix and vector.|
 |[**stb_image**](https://github.com/nothings/stb)|We can load image files.|

cgan.py

 |libraries|description|
 |---|-----|
 |[**tensorflow**](https://github.com/tensorflow/tensorflow)|We can use Deeplearning functions.|
 |[**mathPlotLib**](https://github.com/matplotlib/matplotlib)|We can visualize data.|
 |[**IPython**](https://github.com/ipython/ipython)|We can control jupyter notebook output.|


## 🔎 Important Functions in Project.sln

take_screenshot()
- send data from GPU to CPU and save image as .jpg file.
- saved as formatted naming.

processinput()
- if you press SPACEBAR, execute take_screenshot() and change light position.
- you can change camera position with W, A, S, D.


## 🔎 Important Functions in cgan.py

generator()
- generate image based on training.

discriminator()
- compete with generator so that generator can do more accurate training.
