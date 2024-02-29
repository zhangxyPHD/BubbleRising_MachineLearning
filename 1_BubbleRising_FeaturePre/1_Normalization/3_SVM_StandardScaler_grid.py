import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split, GridSearchCV
from sklearn.svm import SVR

from sklearn.preprocessing import StandardScaler
from sklearn.metrics import mean_squared_error, r2_score
import os
import joblib 

# 获取当前Python脚本文件的路径,构建data.csv文件的完整路径

script_directory = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(script_directory)
data_csv_path = os.path.join(parent_dir, '0_Data_pre/a_data_Ga_Bo.csv')
save_path = os.path.join(script_directory, 'b_3_Results_svr_pre_feature_StandardScaler.csv')
best_svr_dir = os.path.join(script_directory, 'a_best_svr_model_StandardScaler')
os.makedirs(best_svr_dir, exist_ok=True)

data = pd.read_csv(data_csv_path)
filewrite = open(save_path, 'w', encoding='utf-8', buffering=1)
filewrite.write(f"target_col,r2,nrmse,mape(%),best_params\n")

# 特征列表
feature_columns = ['Lrise', 'Ga', 'Bo']
# 输出目标列表
target_columns = ['maxVelocity', 'Ratio','max_ratio', 'L_bounce_center','L_bounce_upper','t_last']

def dict_to_str(dictionary):
    return ';'.join([f'{key}={value}' for key, value in dictionary.items()])

# 数据log处理
X = data[feature_columns]
y = data[target_columns]
# 创建StratifiedShuffleSplit对象来进行分层抽样
X_train_log, X_test_log, y_trains, y_tests = train_test_split(X, y, test_size=0.2, random_state=42)


scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train_log) 
scaler_path = os.path.join(best_svr_dir, 'scaler_model.pkl')   
joblib.dump(scaler, scaler_path)
X_test_scaled = scaler.transform(X_test_log)


for target_col in target_columns:
    y_train=y_trains[target_col]
    y_test=y_tests[target_col]
    X_train=X_train_scaled
    X_test=X_test_scaled

    # 创建随机森林回归模型
    model = SVR()
    param_grid = {
        # 'kernel': ['linear', 'poly', 'rbf', 'sigmoid'],
        'kernel': ['rbf'],
        'C': [0.01, 0.1, 1, 10, 100],
        'epsilon': [0.001, 0.01, 0.1, 1, 10],
        'gamma': ['scale', 'auto', 0.01, 0.1, 1, 10]
    }
    # 创建GridSearchCV对象
    grid_search = GridSearchCV(estimator=model, param_grid=param_grid, cv=5, scoring='neg_mean_squared_error', n_jobs=-1)

    # 执行网格搜索
    grid_search.fit(X_train, y_train)
    best_params = grid_search.best_params_
    best_params_str = dict_to_str(best_params)
    best_model = SVR(**best_params)
    best_model.fit(X_train, y_train)
    
    model_path = os.path.join(best_svr_dir, f'svr_best_model_{target_col}.joblib')
    joblib.dump(best_model, model_path)    

    # 使用最佳模型进行预测
    y_pred = best_model.predict(X_test_scaled)
    # 计算模型性能
    # 计算归一化均方根误差 (NRMSE)
    mse = mean_squared_error(y_test, y_pred)
    nrmse = np.sqrt(mse) / (y_test.max() - y_test.min())
    r2 = r2_score(y_test, y_pred)
    mape = np.mean(np.abs((y_test - y_pred) / np.maximum(1e-4, y_test))) * 100

    filewrite.write(f"{target_col},{r2:.5f},{nrmse:.4f},{mape:.3f}%,{best_params_str}\n")

filewrite.close()
