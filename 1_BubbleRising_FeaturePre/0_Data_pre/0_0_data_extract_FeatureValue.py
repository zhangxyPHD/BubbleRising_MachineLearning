import os
import numpy as np
import matplotlib.pyplot as plt

# 获取脚本的绝对路径
script_dir = os.path.dirname(os.path.abspath(__file__))

Lrise_values = [40, 20, 15, 12.5,10, 7.5,6.25,5, 3.75,2.5,1.75, 1, 0.75, 0.5, 0.25, 0.1]
Ga_values = [100, 80, 60, 50, 40, 30, 20, 15, 10, 7.5, 5, 2.5, 1]
Bo_values = [0.001, 0.004, 0.007, 0.01, 0.04, 0.07, 0.1, 0.4, 0.7, 1, 4, 7, 10, 20]

# 创建公共文件来存储最大的velocity以及对应的ratio和time
output_file = os.path.join(script_dir, "a_data_Ga_Bo.csv")

Ga_Bo_damp=[]
Ga_Bo_bounce=[]
Ga_Bo_FullyBounce=[]
thred_center=0.004
thred_upper=0.004
ifGa=0
with open(output_file, "w") as result_file:
    result_file.write(
        "Lrise,Ga,Bo,maxVelocity,Ratio,max_ratio,L_bounce_center,L_bounce_upper,t_last\n"
    )  # 写入文件头部

    # 遍历参数组合
    for Lrise in Lrise_values:
        for Ga in Ga_values:
            for Bo in Bo_values:
                if Ga > 25 and Bo > 1:
                    continue
                
                # 生成文件名
                filename = os.path.join(
                    script_dir, f"a_Ga_Bo_results_level6\\Ga{Ga}-Bo{Bo}-Lrise{Lrise}-Level6.csv"
                )

                # 检查文件是否存在
                if os.path.isfile(filename):
                    # 使用NumPy加载数据
                    data = np.genfromtxt(filename, delimiter=",", skip_header=1)

                    # 找到最大的velocity、ratio和time
                    max_velocity_index = np.argmax(data[:, 1])  # 第二列是velocity
                    maxVelocity = data[max_velocity_index, 1]
                    ratio = data[max_velocity_index, 2]
                    Center = data[max_velocity_index, 4]
                    time = data[max_velocity_index, 0]

                    max_ratio_index = np.argmax(data[:, 2])  # 第二列是velocity
                    max_ratio = data[max_ratio_index, 2]  # 第二列是velocity
                    velocities=data[:, 1]
                    velocities_1=velocities[len(velocities) // 2:]

                    min_velocity_index = len(velocities) // 2+np.argmin(velocities_1)  # 第二列是velocity
                    min_velocity = data[min_velocity_index, 1]
                    # if min_velocity<-0.002:
                    center_1=data[:, 3]
                    center_min_index = np.argmin(center_1)                    
                    center_min= center_1[center_min_index]
                    center_max_index=np.argmax(center_1[center_min_index:])  
                    center_max=center_1[center_min_index:][center_max_index]
                    L_bounce_center=center_max-center_min

                    upper_1=data[:, 4]
                    upper_min_index = np.argmin(upper_1)                    
                    upper_min= upper_1[upper_min_index]
                    upper_max_index=np.argmax(upper_1[upper_min_index:])                      
                    upper_max=upper_1[upper_min_index:][upper_max_index]                  
                    L_bounce_upper= upper_max-upper_min
                    t_max_index=max(center_min_index+center_max_index,upper_min_index+upper_max_index)
                    t_last = data[t_max_index, 0]

                    L_bounce_center=round(L_bounce_center,4)
                    L_bounce_upper=round(L_bounce_upper,4)                    

                    # 写入数据到公共文件
                    result_file.write(
                        f"{Lrise},{Ga},{Bo},{maxVelocity},{ratio},{max_ratio},{L_bounce_center},{L_bounce_upper},{t_last}\n"
                    )

                else:
                    print(f"警告：文件 {filename} 不存在。")
