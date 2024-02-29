bubbleRise.c ：主程序
	run_continure.sh 计算中断后，继续计算。
	run.sh ：新计算，会清除结果文件。

dumpToVTU.c ：将snap转化为paraview文件
	run_vtu.sh：加载snap，批量生成vtu文件
getResults.c ：根据snap文件，进行restore，然后计算气泡的位置，变形，速度。
	run_results.sh：加载snap，提取气泡结果

plot_script.gp：使用gnuplot绘制图


头文件：
adapt_wavelet_limited.h：用于网格细化，根据指定细化区域最大的level，进行细化。
output_boundary.h： 根据f场计算气泡的边界，并计算上下左右坐标，计算Ratio
output_mpi.h：改变了原来的dump和restore文件（与mpi冲突），经过修改后可以运行，修改内容如下，注释最后几行
		//   for (scalar s in list){
		//     // s.dirty = true;
		//   }
output_vtu_foreach.h 转化paraview文件头文件

