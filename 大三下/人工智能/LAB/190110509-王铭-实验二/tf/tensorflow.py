import glob                # 用于查询符合特定规则的文件路径名
import os                  # 处理文件和目录
import cv2                 # 用于图像处理
import  tensorflow as tf
from tensorflow.keras import layers, optimizers, datasets, Sequential  #从tensorflow.keras模块下导入layers，optimizers, datasets, Sequential等方法
import numpy as np                #导入numpy数据库
import matplotlib.pyplot as plt   #导入matplotlib.pyplot模块，主要用于展示图像
from sklearn.model_selection import train_test_split   #从sklearn.model_selection模块导入train_test_split方法,用于拆分数据集
path = '/content/drive/MyDrive/Colab Notebooks/flower_photos/'   # 数据集的相对地址，改为你自己的，建议将数据集放入代码文件夹下
# TODO 对图片进行缩放，统一处理为大小为w*h的图像，具体参数需自己定
w = 256      #设置图片宽度为10
h = 256      #设置图片高度为10
c = 3        #设置图片通道为3
def read_img(path,flower):                                                    # 定义函数read_img，用于读取图像数据，并且对图像进行resize格式统一处理
    cate=[path+x for x in os.listdir(path) if os.path.isdir(path+x)]  # 创建层级列表cate，用于对数据存放目录下面的数据文件夹进行遍历，os.path.isdir用于判断文件是否是目录，然后对是目录的文件(os.listdir(path))进行遍历
    imgs=[]                                                            # 创建保存图像的空列表
    labels=[]                                                          # 创建用于保存图像标签的空列表
    for idx,folder in enumerate(cate):
        flower[idx]=folder.split('/')[-1]                                # enumerate函数用于将一个可遍历的数据对象组合为一个索引序列，同时列出数据和下标,一般用在for循环当中
        for im in glob.glob(folder+'/*.jpg'):                         # 利用glob.glob函数搜索每个层级文件下面符合特定格式“/*.jpg”的图片，并进行遍历
            # print('reading the images:%s'%(im))                      # 遍历图像的同时，打印每张图片的“路径+名称”信息
            img=cv2.imread(im)                                        # 利用cv2.imread函数读取每一张被遍历的图像并将其赋值给img
            img=cv2.resize(img,(w,h))                                 # 利用cv2.resize函数对每张img图像进行大小缩放，统一处理为大小为w*h的图像
            imgs.append(img)                                          # 将每张经过处理的图像数据保存在之前创建的imgs空列表当中
            labels.append(idx)                                        # 将每张经过处理的图像的标签数据保存在之前创建的labels列表当中
    return np.asarray(imgs,np.float32),np.asarray(labels,np.int32)

flower_dict = {}
data,label=read_img(path,flower_dict)                                              # 将read_img函数处理之后的数据定义为样本数据data和标签数据label
print("shape of data:",data.shape)                                     # 查看样本数据的大小
print("shape of label:",label.shape)

seed = 109             # 设置随机数种子，即seed值
np.random.seed(seed)   # 保证生成的随机数具有可预测性,即相同的种子（seed值）所产生的随机数是相同的

(x_train, x_val, y_train, y_val) = train_test_split(data, label, test_size=0.20, random_state=seed) #拆分数据集
x_train = x_train / 255  #训练集图片标准化
x_val = x_val / 255      #测试集图片标准化

model = Sequential()
model.add(layers.Conv2D(64, (7, 7), activation='relu', strides=(3, 3), padding='same' ,input_shape=(h, w, 3)))
model.add(layers.MaxPooling2D((3, 3),strides=(2, 2),padding='same'))
model.add(layers.Conv2D(128, (3, 3), activation='relu',strides=(2, 2),padding='same'))
model.add(layers.MaxPooling2D((3, 3),strides=(2, 2),padding='same'))
model.add(layers.Conv2D(256, (3, 3), activation='relu',strides=(1, 1),padding='same'))
model.add(layers.MaxPooling2D((3, 3),strides=(2, 2)))
model.add(layers.Conv2D(256,(3, 3),activation='relu',strides=(1, 1),padding='same'))
model.add(layers.MaxPooling2D((2, 2)))
model.add(layers.Flatten())
model.add(layers.Dense(256,activation='relu'))
model.add(layers.Dense(128,activation='relu'))
model.add(layers.Dense(6))

# Todo 可调整超参数lr，可修改优化器类型
opt = optimizers.Adam(learning_rate=0.001)   #使用Adam优化器，优化模型参数。lr(learning rate, 学习率)

# Todo 可选择其他损失函数
#编译模型以供训练。使用多分类损失函数'sparse_categorical_crossentropy'，使用metrics=['accuracy']即评估模型在训练和测试时的性能的指标，使用的准确率。
model.compile(optimizer=opt,
              loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
              metrics=['accuracy'])

# Todo 可调整超参数
#训练模型，决定训练集和验证集，batch size：进行梯度下降训练模型时每个batch包含的样本数。
#verbose：日志显示，0为不在标准输出流输出日志信息，1为输出进度条记录，2为每个epoch输出一行记录
model.fit(x_train, y_train, epochs=30, validation_data=(x_val, y_val),batch_size=64, verbose=1)
#输出模型的结构和参数量,如果没有定义好模型或者模型参数有问题，运行会出错
model.summary()
path_test = '/content/drive/MyDrive/Colab Notebooks/TestImages/'
imgs=[]                                                 # 创建保存图像的空列表
pic_names=[]
for im in glob.glob(path_test+'/*.jpg'):               # 利用glob.glob函数搜索每个层级文件下面符合特定格式“/*.jpg”进行遍历
    # print('reading the images:%s'%(im))                # 遍历图像的同时，打印每张图片的“路径+名称”信息
    pic_names.append(im.split('/')[-1])
    img=cv2.imread(im)                                  # 利用io.imread函数读取每一张被遍历的图像并将其赋值给img
    img=cv2.resize(img,(w,h))                           # 利用cv2.resize函数对每张img图像进行大小缩放，统一处理为大小为w*h的图像
    imgs.append(img)                                    # 将每张经过处理的图像数据保存在之前创建的imgs空列表当中
imgs = np.asarray(imgs,np.float32)                      # 利用np.asarray()函数对imgs进行数据转换
print("shape of data:",imgs.shape)

# prediction = model.predict_classes(imgs)      #将图像导入模型进行预测
prediction = np.argmax(model.predict(imgs), axis=1)
#绘制预测图像
print(flower_dict)
for i in range(np.size(prediction)):
    #打印每张图像的预测结果
    print(pic_names[i],"预测为:"+flower_dict[prediction[i]])  # flower_dict:定义的标签列表，prediction[i]：预测的结果