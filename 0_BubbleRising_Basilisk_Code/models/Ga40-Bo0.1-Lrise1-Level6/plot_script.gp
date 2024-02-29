# 设置绘图样式
set style data lines
# Set the data separator (comma in this case)
set datafile separator ","
# 提取命令行参数中的文件名和输出文件名
data_file = 'results_shape.dat'
velocity_output = ARGV[1]
position_output = ARGV[2]
ratio_output = ARGV[3]

# 第一幅图（time-velocity）并保存为照片
set terminal pngcairo size 800,600
set output velocity_output  # 设置输出文件名
set title velocity_output
set xlabel 'Time'
set ylabel 'Velocity'
plot data_file using 1:2 with lines title 'Velocity'

# 重置输出设置
set output

# 第2幅图（time-ratio）并保存为照片
set terminal pngcairo size 800,600
set output ratio_output  # 设置输出文件名
set title ratio_output
set xlabel 'Time'
set ylabel 'Ratio'
plot data_file using 1:3 with lines title 'Ratio'

# 重置输出设置
set output

# 第3幅图（time-(-1*position)）y轴使用log坐标并保存为照片
set terminal pngcairo size 800,600
set output position_output  # 设置输出文件名
set title position_output
set xlabel 'Time'
set ylabel 'Values'
set logscale y
set key at 0.7,0.9
set key box opaque
set key above

# 绘制 Xmax，使用实线（linetype 1）
plot data_file using 1:4 with lines lt 1 title 'CenterX',\
    data_file using 1:5 with lines lt 2 title 'Xmax',\
    data_file using 1:6 with lines lt 3 title 'Xmin'

# 重置输出设置
set output
