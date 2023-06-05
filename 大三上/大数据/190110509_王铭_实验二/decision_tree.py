import pandas as pd
import numpy as np


# 计算信息熵
def entropy(data):
    rows = data.shape[0]
    columns_set = data.iloc[:, -1].value_counts()
    p = columns_set / rows
    result = (-p * np.log2(p)).sum()
    return result
# 返回按照col_name属性的val值划分的数据集
def split(data,col_name,val):
    data_ret = data.loc[data[col_name] == val, :]
    data_ret = data_ret.drop([col_name],axis=1)
    return data_ret

def ID3_best_split(data):
    rows = data.shape[0]
    cols = data.shape[1]
    data_entropy = entropy(data)
    data_col = data.columns
    max = 0.0
    feature_ret = ''
    for i in range(cols - 1):
        t = dict(data.iloc[:,i].value_counts())
        entropy_i = 0.0
        for k,v in t.items():
            # print()
            entropy_i = entropy_i + entropy(split(data,data_col[i],k)) * v / rows
        if (data_entropy - entropy_i) > max:
            max = data_entropy - entropy_i
            feature_ret = data_col[i]
    return feature_ret


def cal_vote(data):
    vote = dict(data.iloc[:, data.shape[1] - 1].value_counts())
    return list(vote.keys())[0]


def create_tree(data):
    # 递归结束的条件，即当前仅有一个属性或是所有数据都是同一个类
    if (data.shape[1] <= 1 or len(data.iloc[:,data.shape[1] - 1].unique()) == 1):
        # 计算所属类别最多的那个类返回
        return cal_vote(data)
    feature_best = ID3_best_split(data)
    if (feature_best == ''):
        return cal_vote(data)
    # 用字典 递归创建决策树
    tree = {feature_best:{}}
    val = dict(data.loc[:,feature_best].value_counts())
    for k,v in val.items():
        tree[feature_best][k] = create_tree(split(data,feature_best,k))
    return tree

def classify(decision_tree,vector):
    first = list(decision_tree.keys())[0]
    label = 0
    for k in decision_tree[first].keys():
        if k == vector[first]:
            # 输入测试样例的属性值与当前节点的属性值相同
            if(isinstance(decision_tree[first][k],dict)):
                #不是叶子
                label = classify(decision_tree[first][k],vector)
            else:
                label = decision_tree[first][k]
    return label

def prepruning_cut(feature,data,valdata):
    rows = valdata.shape[0]
    cols = valdata.shape[1]
    if(rows == 0):
        return True
    # 计算未划分时的正确划分数
    label = cal_vote(data)
    right = 0
    for i in range(rows):
        if int(valdata.iloc[i]['y']) == int(label):
            right = right + 1
    right = float(right) / (rows)
    right2 = 0
    right_classify = {}
    choosen_feature = dict(data.loc[:,feature].value_counts())
    for k in choosen_feature.keys():
        after_cla = split(data,feature,k)
        right_classify[k] = cal_vote(after_cla)
    for i in range(rows):
        for k,v in right_classify.items():
            if(valdata.iloc[i][feature] == k and valdata.iloc[i,cols - 1] == v):
                right2 = right2 + 1
    right2 = float(right2) / (rows)
    if right2 < right and right - right2 > 0.08:
        return False
    return True

def create_precutting_tree(data,valdata):
    if (data.shape[1] <= 1 or len(data.iloc[:,data.shape[1] - 1].unique()) == 1):
        # 计算所属类别最多的那个类返回
        return cal_vote(data)
    feature_best = ID3_best_split(data)
    if (feature_best == ''):
        return cal_vote(data)
    if (prepruning_cut(feature_best,data,valdata) == False):
        return cal_vote(data)
    # 用字典 递归创建决策树
    tree = {feature_best:{}}
    val = dict(data.loc[:,feature_best].value_counts())
    for k,v in val.items():
        tree[feature_best][k] = create_precutting_tree(split(data,feature_best,k),split(valdata,feature_best,k))
    return tree

train = pd.read_csv('D:/DSJ/lab2/train.csv')
test = pd.read_csv('D:/DSJ/lab2/test.csv')
data_entropy = entropy(train)

# print(data_entropy)
# tree = create_tree(train)
tree = create_precutting_tree(train,test)
print('训练结束......')

rows = test.shape[0]
right = 0
predict = []
for i in range(rows):
    predict_label = classify(tree,test.iloc[i,:])
    predict.append(predict_label)
    if predict_label == test.loc[i]['y']:
        right = right + 1
test.insert(16,'predict',predict)
test.to_csv('D:/DSJ/lab2/test_predict.csv')
print('precison:',right / rows)