""" path为数据集的路径，type为数据集的类型 train / test"""
from skimage import io,transform
import os
import numpy as np
from scipy.signal import convolve2d
from sklearn.svm import SVC

def load_dataset(type = 'train'):
    def load_image(path):
        # 图形灰度化 将64*128*3的图片转换为64*128*1的图片
        img = io.imread(path,as_gray= True)
        img = transform.resize(img,[128,64])
        return img

    pos_image_path = ''
    neg_image_path = ''
    # 加载train/test的正例和反例样本
    if(type == 'train'):
        pos_image_path = r"D:\INRIAPerson\96X160H96\Train\pos"
        neg_image_path = r"D:\INRIAPerson\Train\neg"
    elif(type == 'test'):
        pos_image_path = r"D:\INRIAPerson\70X134H96\Test\pos"
        neg_image_path = r"D:\INRIAPerson\Test\neg"
    # 加载每一个图片的路径 用os.path的join方法
    pos_image = [os.path.join(pos_image_path,p) for p in os.listdir(pos_image_path)]
    neg_image = [os.path.join(neg_image_path, p) for p in os.listdir(neg_image_path)]
    # 处理每一个图片
    pos_images = [load_image(p) for p in pos_image]
    neg_images = [load_image(p) for p in neg_image]
    pos_label = np.ones(len(pos_images),dtype=int)
    neg_label = np.zeros(len(pos_images), dtype=int)
    # 合并
    image_path = np.array(pos_image + neg_image)
    images = np.array(pos_images+neg_images)
    labels = np.append(pos_label,neg_label)
    # 为使得数据随机分布，需要打乱顺序，用np.random.permutation函数获得一个顺序
    order = np.random.permutation(images.shape[0])
    images = images[order]
    labels = labels[order]
    image_path = image_path[order]
    # 返回图片及其标签
    return images,labels

""" 避免光线对后续计算梯度的影响，要进行gama矫正,gama取0.5较合适"""
def gama_correction(image,gama = 0.5):
    return np.power(image,gama)

""" 计算梯度"""
def gradient(img):
    op = np.array([[-1,0,1]])
    # 调用scipy提供的库计算卷积
    gradient_x = convolve2d(img,op,mode = "same",boundary="symm")
    gradient_y = convolve2d(img,op.T,mode = "same",boundary = "symm")
    return gradient_x,gradient_y

""" 划分cell
    magnitude是梯度的大小
    orientation是梯度的方向
    bins是划分的区间数目，默认分为9份
    cell_size是cell所占的像素数（如2*2 则 cell_size = 2） 
"""
def cell_hog(magnitude,orientation,bins,cell_size):
    m,n = magnitude.shape
    mcells = m // cell_size
    ncells = n // cell_size
    # 存每一个cell的划分,切片
    slices = []
    for i in range(mcells):
        for j in range(ncells):
            slices.append(np.s_[i*cell_size : (i + 1)*cell_size,j*cell_size:(j + 1) * cell_size])
    bin = np.linspace(-np.pi/2,np.pi/2,10) # 将-pi/2到pi/2的区间分乘20度一份 pi/2和-pi/2是一个
    # 计算每一个cell的梯度 方向
    def get_num(theta):
        for i in range(len(bin) - 1):
            if(bin[i] <= theta < bin[i + 1]):
                    return i
        if(theta == bin[-1]):
            return len(bin) - 1
    # 根据每个cell的角度，投票到各个bins的区间
    all_cell_vote=np.empty(shape=(mcells,ncells,bins))
    for s in slices:
        # 得到每个cell的梯度 方向矩阵
        cell_vote = np.zeros(shape=(bins,))
        mag= magnitude[s]
        ori = orientation[s]
        result = []
        for x in ori:
            for y in x:
                result.append(get_num(y))
        ori = np.array(result)
        ori.resize(cell_size,cell_size)
        # print(mag.shape)
        # 计算单个cell的hog投票
        for i in range(bins):
            cell_vote[i] = mag[np.where(ori == i)].sum()
        # 将单个cell的hog投票加入到全局hog中
        x,y = s[0].start // cell_size,s[1].start // cell_size
        all_cell_vote[x,y] = cell_vote
    return all_cell_vote # 8 * 16 * 9

def block_hog(cell_vote,block_size):
    m_cells, n_cells, bins = cell_vote.shape
    block_slices = []
    for i in range(0, m_cells - block_size + 1, 1):
        for j in range(0, n_cells - block_size + 1, 1):
            block_slices.append(np.s_[i: i + block_size, j: j + block_size])
    result_rogs = []
    for s in block_slices:
        block_rog_vec = cell_vote[s].flatten()
        block_rog_vec = L2Hys(block_rog_vec)
        result_rogs.append(block_rog_vec)
    result_rogs = np.array(result_rogs)
    return result_rogs

def L2Hys(vector):
    result = np.zeros(vector.shape)
    # 先按照L2 norm 归一 分母要加极小值
    result = vector / (np.sqrt(np.power(vector,2).sum() + np.finfo(float).eps))
    result = np.minimum(result,0.2)
    result = result / (np.sqrt(np.power(result,2).sum() + np.finfo(float).eps))
    return result

images,labels = load_dataset()
hog_features = []
for img in images:
    img = gama_correction(img)
    gx,gy = gradient(img)
    magnitude = np.sqrt(np.power(gx,2) + np.power(gy,2))
    orientation = np.arctan(gy / (gx + np.finfo(float).eps))
    cell_hogs = cell_hog(magnitude,orientation,9,8)
    block_hogs = block_hog(cell_hogs,2).flatten()
    hog_features.append(block_hogs)
hog_features = np.array(hog_features)
svm = SVC(kernel='linear')
svm.fit(hog_features,labels)

images,labels = load_dataset(type='test')
hog_features = []
for img in images:
    img = gama_correction(img)
    gx,gy = gradient(img)
    magnitude = np.sqrt(np.power(gx,2) + np.power(gy,2))
    orientation = np.arctan(gy / (gx + np.finfo(float).eps))
    cell_hogs = cell_hog(magnitude,orientation,9,8)
    block_hogs = block_hog(cell_hogs,2).flatten()
    hog_features.append(block_hogs)
hog_features = np.array(hog_features)
labels_pred = svm.predict(hog_features)
from sklearn.metrics import accuracy_score, recall_score, precision_score, f1_score

print('Accuracy: %.5f' % accuracy_score(labels, labels_pred))
print('Recall: %.5f' % recall_score(labels, labels_pred))
print('Precision: %.5f' % precision_score(labels, labels_pred))
print('F1-score: %.5f' % f1_score(labels, labels_pred))
