# PBR-Renderer
一个基于OpenGL的简单PBR渲染器

## 启动
首先，你需要下载并编译[glad](https://glad.dav1d.de/)、[glfw](https://www.glfw.org/)、[assimp](https://github.com/assimp/assimp)等依赖库

在得到相应的动态链接库文件后(`.so`、`.dylib`、`.dll`等文件)，将其移动至`lib`文件夹下，随后调整`CMakeLists.txt`文件中相应的链接库名称

然后即可开始编译：
``` bash
mkdir build && cd build
cmake ..
make
```

运行并得到以下的结果
``` bash
cd ..
./build/Renderer
```

![example 1](https://github.com/sinngetu/PBR-Renderer/blob/main/export/example_1.png)
![example 2](https://github.com/sinngetu/PBR-Renderer/blob/main/export/example_2.png)

> 在项目编译完成后，请确保在项目根目录下启动，这是由于根路径设置使用的是`std::filesystem::current_path()`，因此与当前终端的`pwd`一致

## 目录结构
```
.
├── ...
├── assets              # 资源
├── export              # 程序导出内容
├── include
│   ├── ...
│   └── renderer        # 项目自定义头文件
├── lib
├── references          # 参考文献
└── src
    ├── material        # 材质
    ├── scene           # 场景
    ├── shader          # Shader
    ├── util            # 工具方法/类
    ├── global.cpp      # 全局共用参数/方法
    ├── main.cpp        # 入口文件
    └── prebuilt.cpp    # 预处理入口
```

## TODO LIST（待补充）
> 待处理内容与大致思路

### **`光源与阴影`**
* 点光源、聚光灯及其阴影
    * 点光源：半径平方衰减，阴影贴图为cubemap
    * 聚光灯：通过camera的视角方向与view方向夹角计算
        * 2D阴影贴图即可，超过180°的聚光灯不符合生活经验
        * 内外双锥体计算，中间插值以平滑边缘
* 彩色阴影：
    * RGB通道记录颜色，A通道记录深度（最远的透明物体深度）
    * 仅用于透明物体，需注意绘制顺序
    * 与普通阴影贴图一起使用
* 软阴影
    * 一阶段：通过卷积模糊边缘
    * 二阶段：[Percentage-closer soft shadows](https://dl.acm.org/doi/10.1145/1187112.1187153)

### **`模型`**
* 视锥体剔除
    * AABB包围盒
    * BVH树
    * 判断内含、相交、外离
* 模型与材质LOD：通过camera与顶点位置的距离计算：distance(fragPosition, viewPosition) / (near - far)
* mesh合并
    * 一阶段：同种材质的模型，mesh合并后仅需调用一次Draw
    * 二阶段：贴图合并，同种shader的材质均可合并（参数可能都需要贴图存储），顶点UV需要调整

### **`场景`**
* 场景拍摄与导出（cubemap）
    * 后可实现反射探针
* 透明物体排序
* HDR图片存储
* minimap图片存储
* 抗锯齿：FXAA
* 环境光烘焙：Path Tracing
    * 面光源
    * 自发光材质
