import glob                # 用于查询符合特定规则的文件路径名
import os                  # 处理文件和目录
import cv2                 # 用于图像处理
import torch
import numpy as np                #导入numpy数据库
import matplotlib.pyplot as plt   #导入matplotlib.pyplot模块，主要用于展示图像
from sklearn.model_selection import train_test_split   #从sklearn.model_selection模块导入train_test_split方法,用于拆分数据集
import torch.nn as nn
import torch.nn.functional as F
from collections import OrderedDict
w = 128
h = 128
c = 3
class FlattenLayer(nn.Module):
    def __init__(self):
        super(FlattenLayer, self).__init__()
    def forward(self, x): # x shape: (batch, *, *, ...)
        return x.view(x.shape[0], -1)


model = nn.Sequential(
    nn.Conv2d(3, 64, kernel_size=9, stride=2, padding=4), # 64*64
    nn.BatchNorm2d(64),
    nn.ReLU(),
    nn.MaxPool2d(kernel_size=5, stride=2, padding=2), # 32*32
    # 卷积+池化层二
    nn.Conv2d(64, 128, kernel_size=7, stride=2, padding=3),  # 16*16
    nn.BatchNorm2d(128),
    nn.ReLU(),
    nn.MaxPool2d(kernel_size=3, stride=2, padding=1),  # 8*8
    # 卷积+池化层三
    nn.Conv2d(128, 256, kernel_size=5, stride=2, padding=2),  # 4*4
    nn.BatchNorm2d(256),
    nn.ReLU(),
    nn.MaxPool2d(kernel_size=3, stride=1, padding=1),  # 4*4
    # 卷积+池化层四
    nn.Conv2d(256, 512, kernel_size=3, stride=1, padding=1),  # 4*4
    nn.BatchNorm2d(512),
    nn.ReLU(),
    nn.MaxPool2d(kernel_size=4),  # 1*1

    FlattenLayer(),
    nn.Linear(512, 256),
    nn.ReLU(),
    nn.Linear(256, 128),
    nn.ReLU(),
    nn.Linear(128, 6)
)
model.load_state_dict(torch.load('flower_mlp.pt'))
# flower_dict = {0:'bee',1:'blackberry',2:'blanket',3:'bougainvillea',4:'bromelia',5:'foxglove'} #创建图像
path_test = './TestImages/' # 测试图像的地址 （改为自己的）
def softmax(X):
    X_exp = X.exp()
    partition = X_exp.sum(dim=1, keepdim=True)
    return X_exp / partition
imgs=[]                                                 # 创建保存图像的空列表
for im in glob.glob(path_test+'/*.jpg'):               # 利用glob.glob函数搜索每个层级文件下面符合特定格式“/*.jpg”进行遍历
    print('reading the images:%s'%(im))                # 遍历图像的同时，打印每张图片的“路径+名称”信息
    img=cv2.imread(im)                                  # 利用io.imread函数读取每一张被遍历的图像并将其赋值给img
    img=cv2.resize(img,(w,h))                           # 利用cv2.resize函数对每张img图像进行大小缩放，统一处理为大小为w*h的图像
    imgs.append(img)                                    # 将每张经过处理的图像数据保存在之前创建的imgs空列表当中
imgs = np.asarray(imgs,np.float32)                      # 利用np.asarray()函数对imgs进行数据转换
print("shape of data:",imgs.shape)
imgs = imgs.transpose(0, 3, 1, 2)
test_imgs = torch.from_numpy(imgs)
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
print(device)
test_imgs = softmax(model(test_imgs))
test_imgs = test_imgs.argmax(dim=1).to('cpu').numpy()
#绘制预测图像
for i in range(np.size(test_imgs)):
    #打印每张图像的预测结果
    print("第",i+1,"朵花预测:",test_imgs[i])