<div align="center">
 
# PRE Project : OPENGL with pix2pix

</div>

This project is made in 2023 summer PRE program.
You can run in windows OS.

You can get input image data with project.sln 
AND
You can train and deeplearning with pix2pix, so that you can generate new output.

Input image data is hard shadow (shadow mapping) and soft shadow (pcss).

![image](https://github.com/ParkJaeSeong98/PRE-OPENGL/assets/48883899/e6c6aeef-52e4-4f7a-85e5-fabb7e8b9600)


Project folder is C++ solution folder using OPENGL.
- you can make input data with this program.
- you have to modify PATH values.
- you can change scene by modifying global variable 'sceneCounter' (1, 2, 3)

cgan.py can be executed in jupyter notebook.
- you have to modify PATH values.
- you can to modify buffer_size, batch_size, steps as you want.


![image](https://github.com/ParkJaeSeong98/PRE-OPENGL/assets/48883899/947c176b-5eee-4258-bb45-ee8b86133a2f)




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
