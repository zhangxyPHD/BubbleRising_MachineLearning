# BubbleRising_MachineLearning
## 0_BubbleRising_Basilisk_Code
### 多任务运行脚本
run_script.sh
### basilisk code
<ol>
<li>bubbleRise.c ：主程序</li>
<ol>
<li>run_continure.sh 计算中断后，继续计算</li>
<li>run.sh 新计算，会清除结果文件</li>
</ol>
<li>后处理
<ol>
<li>dumpToVTU.c 将snap转化为paraview文件</li>
<li>run_vtu.sh 加载snap，批量生成vtu文件</li>
<li>getResults.c 根据snap文件，进行restore，然后计算气泡的位置，变形，速度</li>
<li>run_results.sh 加载snap，提取气泡结果</li>
<li>plot_script.gp 使用gnuplot绘制图</li>
</ol>

<li>头文件</li>
<ol>
<li>adapt_wavelet_limited.h 用于网格细化，根据指定细化区域最大的level，进行细化</li>
<li>output_boundary.h 根据f场计算气泡的边界，并计算上下左右坐标，计算Ratio</li>
<li>output_vtu_foreach.h 转化paraview文件头文件</li>
<li>output_mpi.h：改变了原来的dump和restore文件（与mpi冲突），经过修改后可以运行，修改内容如下，注释最后几行(//s.dirty = true;)
</ol>
</ol>

### An example
take Ga40-Bo0.1-Lrise1-Level6 as an example
<ol>
<li>model 存放不同参数对应的basilisk源文件</li>
<li>results 存放不同参数对应的结果文件</li>
<li>results_Picture 存放不同参数对应的绘图</li>
</ol>


## 1_BubbleRising_FeaturePre
<l>
<li>0_Data_pre 提取数据</li>
<li>1_Normalization 训练集预处理</li>
<li>2_Pre_Feature 气泡上升特征值预测</li>
</ul>


