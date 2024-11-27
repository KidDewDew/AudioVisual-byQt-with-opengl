本项目采用fftw3库进行快速傅里叶变化。
以下为运行截图
![截图1](https://github.com/user-attachments/assets/27dc1156-71c6-4606-a091-ba02c8ec0eb8)
图中左:实时响度(多声道) 中上:时域曲线图 中下:频谱图  右:3D频谱图by opengl
项目文件:
vShader.glsl和fShader.glsl为顶点和片元着色器。
Main.qml是主界面文件(太懒了，没分文件写)
c++代码中Spectrum3D类实例化了一个QQuickItem，并进行3D频谱图的绘制。
c++代码中MyAudioPlayer类封装了一个QMediaPlayer给qml使用。
main.cpp中使用QAudioProbe监测音频输出，并进行响度和频谱的处理。
^_^
