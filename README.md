# Tanner L-Edit Macro Scripts

## Introduction

Welcome to the Tanner L-Edit Macro Scripts repository! This collection of Macro scripts is designed to enhance the functionality of Tanner L-Edit, providing improved precision and efficiency in your layout design tasks. The scripts included in this project focus on enhancing Object alignment, array operations, Object rotation/mirroring, drawing regular polygons, and creating special structures such as rounded corners.

## Features

### 1. Object Alignment

Achieve more accurate Object alignment with the help of specialized Macro scripts. These scripts are tailored to enhance the precision of your layout design.

### 2. Array Operations

Facilitate the creation of arrays with ease using our Macro scripts. Currently, the supported array types include hexagonal arrays, circular arrays, and unidirectional arrays.

### 3. Object Rotation/Mirroring

Customize Object rotation and mirroring with specified rotation centers and mirror lines. These scripts provide flexibility in transforming your layout elements.

### 4. Regular Polygon Drawing

Effortlessly draw regular polygons, including inscribed regular polygons and polygons with specified side lengths. These scripts simplify the process of incorporating geometric shapes into your design.

### 5. Special Structure Drawing

Create special structures in your layout, including rounded corners. These scripts add versatility to your design capabilities, allowing you to implement unique and intricate features.

## Usage

1. Clone this repository to your local machine.
2. Copy the desired Macro script from the repository into your Tanner L-Edit environment.
3. Follow the specific instructions provided within each script for seamless integration and utilization.

### Macro source files

| File | Commands |
| --- | --- |
| Ledit_AlignmentMacros.c | Selection alignment |
| Ledit_ArrayMacros.c | Arrays, rotations, and mirrors |
| Ledit_ObjectEditingMacros.c | Editing, scaling, and route placement |
| Ledit_RegularShapeMacros.c | Regular polygons and ellipses |
| Ledit_ShapeCenterMacros.c | Circle placement at the selection center |
| Ledit_ShapeConversionMacros.c | Shape conversion and corner rounding |
| Ledit_ContourMacros.c | Contours and tangent rings |

Update the script paths in L-Edit's UPI Macro Files settings after renaming. The regular boundary and alignment commands now open a mode picker. For corner rounding, load Ledit_ShapeConversionMacros.c from the current checkout. The updated dialog reads **Round corners - select result, report crescent radius (10 nm)**.

### Boundary, alignment, and circle placement commands

- **Draw_Regular_Boundary_func** offers circular and rounded hexagonal contours for edge circles from a regular array. Set **Draw helper circles (0 or 1)** to **1** to draw construction circles on a separate layer. Use **Select_Boundry_Circles_func** to select edge circles from a honeycomb layout.
- **Draw_Arbitrary_Circle_Boundary_func** grows each selected circle by the specified distance from its edge. It adds a concave arc tangent to two grown circles when the requested smoothing radius fits. An exposed arc can also join two disks backed by a chain of overlapping grown disks. Gaps that cannot be bridged remain open for manual editing. The original circles are kept.
- **Align_Selection_func** offers left, right, bottom, top, and center alignment.
- **Route_Circle_func** offers inside tangency, centers on the boundary, and outside tangency. Spacing is measured along the route followed by the centers. Positions that cannot fit on the requested side are skipped.

### Conversion and rounding macros

Load the desired .c scripts in L-Edit's UPI Macro Files settings. Keep ldata.h and the Ledit_*.h helper headers beside the scripts.

- **Round_Selected_Corners_func** converts selected rectangles, polygons, circles, wires, circular sectors, and annular sectors into rounded polygons on the chosen output layer. The sources are kept. The output is checked on the design grid for at least 10 nm between consecutive points, including the closing edge. Re-running the macro with a different radius recalculates fillets from the source contour stored with new results. For older results it can recover sampled circular fillets that still have straight tangent edges. Nearby corners can be merged. For circular crescents and corners on sampled curves, the macro fits a circle tangent to both sides of the corner and reports the smallest radius used on those corners. The new polygons are selected after the macro runs. Shapes with holes or contours that cannot be rounded at this spacing are reported and skipped.
- **Approximate_Circle_func** offers equal-area, inscribed, and enclosing circles. The original shapes are kept. The older Approximate_Circle_Sample_func entry opens the same dialog.
- **Conversion_To_Rounded_Hexagon_func** displays the automatic radius in the dialog title. Enter -1 to use each source circle's radius divided by 3.7, or enter a corner radius in micrometres.
- Commands that create geometry prompt for a writable output layer once per run. The current layer is the default when writable.

## Contributions

Contributions to this project are welcome! If you have ideas for additional features, improvements, or bug fixes, please feel free to submit a pull request.

## Disclaimer

Please note that these Macro scripts are currently in development, and additional features may be added over time. Always ensure compatibility with your specific version of Tanner L-Edit before integrating the scripts into your workflow.

Thank you for choosing our Tanner L-Edit Macro Scripts. We hope they enhance your layout design experience! If you have any questions or issues, feel free to open an [issue](https://github.com/muyeyifeng/Ledit_Marco/issues) on GitHub. Happy designing!




# Tanner L-Edit Macro Scripts（Tanner L-Edit Marco脚本）

## 简介

欢迎来到Tanner L-Edit Marco脚本仓库！这个Marco脚本的集合旨在增强Tanner L-Edit的功能，提供更精确和高效的布局设计任务。本项目包含的脚本主要用于优化对象对齐、阵列操作、对象旋转/镜像、绘制正多边形以及创建特殊结构（如圆角）等功能。

## 特性

### 1. 对象对齐

通过专用的Marco脚本实现更精准的对象对齐，提高布局设计的精度。

### 2. 阵列操作

使用我们的Marco脚本轻松创建阵列。目前支持的阵列类型包括蜂窝状阵列、环形阵列和单向阵列。

### 3. 对象旋转/镜像

使用指定的旋转中心和镜像线自定义对象的旋转和镜像效果。这些脚本为您的布局元素转换提供了灵活性。

### 4. 正多边形绘制

轻松绘制正多边形，包括内切正多边形和指定边长的正多边形。这些脚本简化了将几何形状纳入设计的过程。

### 5. 特殊结构绘制

创建布局中的特殊结构，包括圆角等。这些脚本为您的设计提供了多样性，使您能够实现独特而复杂的特征。

## 使用方法

1. 将此仓库克隆到您的本地机器。
2. 从仓库中复制所需的Marco脚本到您的Tanner L-Edit环境中。
3. 按照每个脚本提供的具体说明，轻松集成和使用。

### 边界、对齐与沿路径布圆

- **Draw_Regular_Boundary_func** 可选择圆形或圆角六边形边界，沿用规则蜂窝排布的边缘圆作为基准。将 **Draw helper circles (0 or 1)** 设为 **1** 可在独立图层绘制辅助圆，默认 **0**。仍可用 **Select_Boundry_Circles_func** 选择边缘圆。
- **Draw_Arbitrary_Circle_Boundary_func** 按指定距离扩张各选中圆，再用指定半径的圆弧对可行的凹角做相切过渡。若缺口内侧已有相互重叠的扩张圆形成支撑，可只补外侧圆弧。无法闭合的缺口保留供手工处理，原圆保留。
- **Align_Selection_func** 可选择左对齐、右对齐、底部对齐、顶部对齐或居中对齐。
- **Route_Circle_func** 可选择内侧相切、圆心在边界上或外侧相切。间距按圆心经过的路径测量，无法在指定侧放下完整圆的位置会跳过。

### 图形转换与圆角宏

在 L-Edit 的 UPI Macro Files 设置中更新宏文件路径。圆角功能使用当前工作目录中的 Ledit_ShapeConversionMacros.c。请将 ldata.h 和 Ledit_*.h 辅助头文件与脚本放在同一目录。新版圆角窗口标题为 **Round corners - select result, report crescent radius (10 nm)**。

- **Round_Selected_Corners_func** 支持处理矩形、多边形、圆、线宽路径（Wire）、扇形和环形扇区。它保留原图形，并在目标图层新建多边形。输出会按设计坐标网格检查，闭合边在内的相邻顶点间距至少为 10 nm。新结果会保存圆角前的轮廓，再运行时可按不同半径重新计算。旧结果若保留了圆弧两侧的直线切边，宏会尝试恢复尖角。相邻近角会尝试合并处理。对于圆弧月牙或曲线采样边上的尖角，宏会沿两侧边界寻找切点，并报告这些角使用的最小半径。执行后会选中新生成的多边形。整圆环等带孔形状，以及无法满足点距和圆滑度的轮廓，会跳过并提示。
- **Approximate_Circle_func** 可选择等面积圆、内切圆或外切圆，保留原图形。旧的 Approximate_Circle_Sample_func 也进入同一个模式选择窗口。
- **Conversion_To_Rounded_Hexagon_func** 在标题中显示自动圆角半径。输入 -1 时按每个圆的半径除以 3.7 计算，也可以输入明确的圆角半径（微米）。
- 新建图形的宏每次运行都可选目标图层。当前图层可写时会默认选中。

## 贡献

欢迎对这个项目进行贡献！如果您有关于额外功能、改进或错误修复的想法，请随时提交拉取请求。

## 免责声明

请注意，这些Marco脚本目前仍在开发中，随着时间的推移可能会添加更多功能。在将脚本集成到工作流程之前，请始终确保其与您的具体版本的Tanner L-Edit兼容。

感谢选择我们的Tanner L-Edit Marco脚本。希望它们能够提升您的布局设计体验！如果有任何问题或疑虑，请随时在GitHub上提出 [issue](https://github.com/muyeyifeng/Ledit_Marco/issues)。祝设计愉快！
