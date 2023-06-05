import glob                # 用于查询符合特定规则的文件路径名
import os                  # 处理文件和目录
import cv2                 # 用于图像处理
import torch
import numpy as np                #导入numpy数据库
import matplotlib.pyplot as plt   #导入matplotlib.pyplot模块，主要用于展示图像
from sklearn.model_selection import train_test_split   #从sklearn.model_selection模块导入train_test_split方法,用于拆分数据集
import torch.nn as nn
import torch.nn.functional as F
import time
from collections import OrderedDict
import argparse
# 创建解析
parser = argparse.ArgumentParser(description="train flower classify",
                                 formatter_class=argparse.ArgumentDefaultsHelpFormatter)
# 添加参数
parser.add_argument('--train_url', type=str,
                    help='the path model saved')
parser.add_argument('--data_url', type=str, help='the training data')
# 解析参数
args, unkown = parser.parse_known_args()

path = args.data_url
model_path = args.train_url

# path = './flower_photos/'   # 数据集的相对地址，改为你自己的，建议将数据集放入代码文件夹下

# Todo 对图片进行缩放，统一处理为大小为w*h的图像，具体数据需自己定
w = 128       #设置图片宽度
h = 128       #设置图片高度
c = 3        #设置图片通道为3
# (batch,channel,height,width)
flower_dict = {}
def read_img(path,flower):                                                    # 定义函数read_img，用于读取图像数据，并且对图像进行resize格式统一处理
    cate=[path+x for x in os.listdir(path) if os.path.isdir(path+x)]  # 创建层级列表cate，用于对数据存放目录下面的数据文件夹进行遍历，os.path.isdir用于判断文件是否是目录，然后对是目录的文件(os.listdir(path))进行遍历
    imgs=[]                                                            # 创建保存图像的空列表
    labels=[]                                                          # 创建用于保存图像标签的空列表
    for idx,folder in enumerate(cate):# enumerate函数用于将一个可遍历的数据对象组合为一个索引序列，同时列出数据和下标,一般用在for循环当中
        flower[idx] = folder.split('/')[-1]
        for im in glob.glob(folder+'/*.jpg'):                         # 利用glob.glob函数搜索每个层级文件下面符合特定格式“/*.jpg”的图片，并进行遍历
            #print('reading the images:%s'%(im))                      # 遍历图像的同时，打印每张图片的“路径+名称”信息
            img=cv2.imread(im)                                        # 利用cv2.imread函数读取每一张被遍历的图像并将其赋值给img
            img=cv2.resize(img,(w,h))                                 # 利用cv2.resize函数对每张img图像进行大小缩放，统一处理为大小为w*h的图像
            imgs.append(img)                                          # 将每张经过处理的图像数据保存在之前创建的imgs空列表当中
            labels.append(idx)                                        # 将每张经过处理的图像的标签数据保存在之前创建的labels列表当中
    return np.asarray(imgs,np.float32),np.asarray(labels,np.int32)   # 利用np.asarray函数对生成的imgs和labels列表数据进行转化，之后转化成数组数据（imgs转成浮点数型，labels转成整数型）


data, label = read_img(path,flower_dict)                                           # 将read_img函数处理之后的数据定义为样本数据data和标签数据label
print("shape of data:",data.shape)                                     # 查看样本数据的大小
print("shape of label:",label.shape)                                   # 查看标签数据的大小


seed = 109             # 设置随机数种子，即seed值
np.random.seed(seed)   # 保证生成的随机数具有可预测性,即相同的种子（seed值）所产生的随机数是相同的

(x_train, x_val, y_train, y_val) = train_test_split(data, label, test_size=0.20, random_state=seed) #拆分数据集一部分为训练集一部分为验证集，拆分比例可调整
x_train = x_train / 255  #训练集图片标准化
x_val = x_val / 255      #测试集图片标准化

# flower_dict = {0:'bee',1:'blackberry',2:'blanket',3:'bougainvillea',4:'bromelia',5:'foxglove'} #创建图像标签列表

x_train = x_train.transpose(0, 3, 1, 2)
x_val = x_val.transpose(0, 3, 1, 2)

train_iter = torch.from_numpy(x_train)
train_iter_y = torch.from_numpy(y_train)
test_iter = torch.from_numpy(x_val)
test_iter_y = torch.from_numpy(y_val)

train_iter_y = train_iter_y.to(torch.int64)
test_iter_y = test_iter_y.to(torch.int64)

def evaluate(X, y, net, device=None):
    if device is None:
        device = list(net.parameters())[0].device
    acc_sum, n = 0.0, 0
    with torch.no_grad():
        net.eval() # 评估模式
        acc_sum += (net(X.to(device)).argmax(dim=1) == y.to(device)).float().sum().cpu().item()
        net.train() # 改回训练模式
        n += y.shape[0]
    return acc_sum / n

# 卷积后展开再进行全连接
class FlattenLayer(nn.Module):
    def __init__(self):
        super(FlattenLayer, self).__init__()
    def forward(self, x): # x shape: (batch, *, *, ...)
        return x.view(x.shape[0], -1)

def train(net, train_iter, train_iter_y, test_iter, test_iter_y, batch_size, optimizer, device, num_epochs):
    net = net.to(device)
    print("training on ", device)
    loss = torch.nn.CrossEntropyLoss() # 定义损失函数为交叉熵
    batch_count = 0
    for epoch in range(num_epochs):
        train_l_sum, train_acc_sum, n, start = 0.0, 0.0, 0, time.time()
        for i in range(train_iter.shape[0] // batch_size + 1):
            X = train_iter[i * batch_size : (i + 1) * batch_size]
            y = train_iter_y[i * batch_size : (i + 1) * batch_size]
            X = X.to(device)
            y = y.to(device)
            y_hat = net(X)
            l = loss(y_hat, y)
            optimizer.zero_grad()
            l.backward()
            optimizer.step()
            train_l_sum += l.cpu().item()
            train_acc_sum += (y_hat.argmax(dim=1) == y).sum().cpu().item()
            n += y.shape[0]
            batch_count += 1
        test_acc = evaluate(test_iter, test_iter_y, net)
        print('epoch %d, loss %.4f, train acc %.3f, test acc %.3f, time %.1f sec'
              % (epoch + 1, train_l_sum / batch_count, train_acc_sum / n, test_acc, time.time() - start))

print(flower_dict)

# Todo 自行实现模型结构
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
# 训练
lr, num_epochs = 0.0001, 16
batch_size = 64
optimizer = torch.optim.Adam(model.parameters(), lr=lr)
device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
train(model,train_iter,train_iter_y,test_iter,test_iter_y,batch_size,optimizer,device,num_epochs)
torch.save(model.state_dict(), model_path + "/flower_mlp.pt")