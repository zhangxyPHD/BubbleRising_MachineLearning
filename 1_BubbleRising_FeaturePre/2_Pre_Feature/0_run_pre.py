import subprocess

# 文件列表
file_list = ['1_1_SVM_grid.py', '1_2_BPNN_grid.py', '1_3_RF_grid.py']

# 存储子进程对象的列表
processes = []

# 启动每个文件的子进程
for file in file_list:
    process = subprocess.Popen(['python', file])
    processes.append(process)

# 等待所有子进程完成
for process in processes:
    process.wait()

print("所有文件已运行完成。")