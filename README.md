# win32-mask

此项目为使用win32api 创建一个窗口，并在窗口上创建出一个蒙版，在蒙版上创建一个图片 

，拖拽窗口，蒙版会随着窗口一起移动，放大缩小。达到的效果类似于一个loading界面

之后会将界面移植到delphi上



更新：

移植到Delphi上封装成为一个类 这个类public为startloading与stoploading为公共接口，调用也只是需要调用这两个接口，调用startloading需要传入两个函数，第一个函数为当loading界面要显示在哪一个窗口的上方，第二个接口为loading界面中的静态图片(注意：静态图片需要为bmp格式的图片)，需要加载的资源中。正常调用即可