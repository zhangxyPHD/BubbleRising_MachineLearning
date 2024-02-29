import matplotlib.pyplot as plt
import numpy as np
import os
import matplotlib
matplotlib.rcParams['mathtext.default'] = 'regular'
plt.rcParams['font.family'] = 'Times New Roman'
plt.rcParams['font.size'] = 20

# Set a common figsize and dpi for both R2 and NRMSE plots
common_figsize = (6, 4)
common_dpi = 1000

for label in ['svr', 'bpnn', 'rf']:
    if label == 'svr':
        id_R2 = '(a) R2 for SVR model'
        id_NRMSE = '(d) NRMSE for SVR model'
    elif label == 'bpnn':
        id_R2 = '(b) R2 for BPNN model'
        id_NRMSE = '(e) NRMSE for BPNN model'
    elif label == 'rf':
        id_R2 = '(c) R2 for RF model'
        id_NRMSE = '(f) NRMSE for RF model'

    script_directory = os.path.dirname(os.path.abspath(__file__))
    plotFile = os.path.join(script_directory, f'r2_{label}.png')
    plotFile_nrmse = os.path.join(script_directory, f'nrmse_{label}.png')

    # 数据
    features = ['log', 'minmax', 'StandardScaler', 'log_minmax', 'log_StandardScaler']
    X_labels = ['Log', 'MinMax', 'Standard ', 'Log+\nMinMax', 'Log+\nStandard ']
    X = [1, 2, 3, 4, 5]
    markers = ['o-', 's-', '^-', 'D-', 'v-', 'p-']
    labels = [r'$V_{max}$', r'$\chi_{V_{max}}$', r'$\chi_{max}$', r'$L_{bounce}$', r'$L_{bounce,upper}$', r'$t_{stop}$']
    R2_all = []
    NRMSE_all = []
    for feature in features:
        file_name = os.path.join(script_directory, f'b_3_Results_{label}_pre_feature_{feature}.csv')
        data = np.genfromtxt(file_name, delimiter=",", skip_header=1)
        R2_all.append(data[:, 1])
        NRMSE_all.append(data[:, 2])

    # R2 plot
    plt.figure(figsize=common_figsize, dpi=common_dpi)
    for i in range(len(labels)):
        if i == 4:
            continue
        data_at_i = [arr[i] for arr in R2_all]
        plt.plot(X, data_at_i, markers[i], label=labels[i])

    plt.legend(loc='lower right')
    plt.xlabel('Normalization Scalar')
    plt.ylabel(f'R2')
    plt.title(f'{id_R2}')
    plt.savefig(plotFile, dpi=common_dpi, bbox_inches='tight')
    plt.clf()

    # NRMSE plot
    plt.figure(figsize=common_figsize, dpi=common_dpi)
    for i in range(len(labels)):
        if i == 4:
            continue
        data_at_i_nrmse = [arr[i] for arr in NRMSE_all]
        plt.plot(X, data_at_i_nrmse, markers[i], label=labels[i], linestyle='--', alpha=0.5)

    plt.xlabel('Normalization Scalar')
    plt.ylabel(f'NRMSE')
    plt.title(f'{id_NRMSE}')
    plt.legend(loc='upper right')
    plt.savefig(plotFile_nrmse, dpi=common_dpi, bbox_inches='tight')
    plt.clf()
