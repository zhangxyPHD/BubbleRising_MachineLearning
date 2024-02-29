import subprocess

# 文件列表
file_list = ['1_SVM_minmax_grid.py', '2_SVM_log_minmax_grid.py', '3_SVM_StandardScaler_grid.py', '4_SVM_log_StandardScaler_grid.py', '5_SVM_log_grid.py']

# 存储子进程对象的列表
processes = []

# 启动每个文件的子进程
for file in file_list:
    process = subprocess.Popen(['python3.10', file])
    processes.append(process)

# 等待所有子进程完成
for process in processes:
    process.wait()

print("所有文件已运行完成。")