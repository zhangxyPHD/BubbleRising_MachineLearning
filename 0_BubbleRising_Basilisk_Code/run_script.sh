#!/bin/bash

# 定义参数列表
Lrise_values="0.1 0.25 0.5 0.75 1 1.75 2.5 3.75 5 6.25 7.5 10 12.5 15 20 40"
Ga_values="100 80 60 50 40 30 20 15 10 7.5 5 2.5 1"
Bo_values="0.001 0.004 0.007 0.01 0.04 0.07 0.1 0.4 0.7 1 4 7 10"

radiusLevel=6
t_max=300
tsnap=0.01
mpinum=6  
Nproc=15  # 可同时运行的最大作业数

if [ ! -d "models" ]; then
    mkdir -p "models"
fi

if [ ! -d "results" ]; then
    mkdir -p "results"
fi

if [ ! -d "results_Picture" ]; then
    mkdir -p "results_Picture"
fi

# 定义函数来运行任务
run_task() {
    Ga=$1
    Bo=$2
    Lrise=$3
	if (( $(echo "$Lrise + 4 < 8.5" | bc -l) )); then
        domainSize=8
        basicLevel=3
    elif (( $(echo "$Lrise + 4 < 16.5" | bc -l) )); then
        domainSize=16
        basicLevel=4
    elif (( $(echo "$Lrise + 4 < 32.5" | bc -l) )); then
        domainSize=32
        basicLevel=5
    elif (( $(echo "$Lrise + 4 < 64.5" | bc -l) )); then
        domainSize=64
        basicLevel=6
    else
        echo "Lrise is greater than or equal to 64.5"  # 可选的默认情况
    fi
	
    file="Ga$Ga-Bo$Bo-Lrise$Lrise-Level$radiusLevel"

    if [ -d "$file" ]; then
        rm -r "$file"
    fi
    cp -r bubbleRiseImpact/ "$file" 
    cd "$file"
    (
        CC99='mpicc -std=c99' qcc -w -Wall -O2 -D_MPI=1 bubbleRise.c -o $file -lm
        mpirun -np $mpinum ./$file $Ga $Bo $domainSize $Lrise $basicLevel $radiusLevel $t_max $tsnap                
        pngfile="Ga$Ga-Bo$Bo-png"
        mkdir -p "$pngfile"
        gnuplot -c plot_script.gp "$pngfile/Ga$Ga-Bo$Bo-Lrise$Lrise-velocity.png" "$pngfile/Ga$Ga-Bo$Bo-Lrise$Lrise-position.png" "$pngfile/Ga$Ga-Bo$Bo-Lrise$Lrise-ratio.png"
        cp results_shape.dat "../results/$file.csv"
        cp -r "$pngfile" "../results_Picture/$file"
    )
    cd ..
    mv "$file" "./models/$file"
}


# 生成所有组合并存入一个数组
combinations=()
for Bo in $Bo_values; do 
    for Ga in $Ga_values; do
        for Lrise in $Lrise_values; do
            file1="./results1/Ga$Ga-Bo$Bo-Lrise$Lrise-Level$radiusLevel.csv"
            if [ ! -e "$file1" ]; then
                combinations+=("$Ga $Bo $Lrise")
            fi
        done        
    done
done

combinations_size="${#combinations[@]}"
PID=()     # 记录PID到数组, 检查PID是否存在以确定是否运行完毕

for((i=0; i<combinations_size; )); do
    for((Ijob=0; Ijob<Nproc; Ijob++)); do
        if [[ $i -gt $combinations_size ]]; then
            break;
        fi		
        if [[ ! "${PID[Ijob]}" ]] || ! kill -0 ${PID[Ijob]} 2> /dev/null; then
            run_task ${combinations[i]} &
            PID[Ijob]=$!
            i=$((i+1))
        fi
    done
    sleep 1
done
wait

