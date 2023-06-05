import warnings
warnings.filterwarnings("ignore")

import os
import numpy as np
import json
import torch
import pickle
import torch.nn as nn
import torch.optim as optim
import pandas as pd
from tqdm import tqdm
from ark_nlp.model.ner.global_pointer_bert import GlobalPointerBert
from ark_nlp.model.ner.global_pointer_bert import GlobalPointerBertConfig
from ark_nlp.model.ner.global_pointer_bert import Dataset
from ark_nlp.model.ner.global_pointer_bert import Task
from ark_nlp.model.ner.global_pointer_bert import get_default_model_optimizer
from ark_nlp.model.ner.global_pointer_bert import Tokenizer
from ark_nlp.model.ner.global_pointer_bert import Predictor
from ark_nlp.factory.utils.conlleval import get_entity_bio

train_path = r'D:\py\NLP\train_data\train.txt'
test_path = r'D:\py\NLP\preliminary_test_b\sample_per_line_preliminary_B.txt'
datalist = []
with open(train_path, 'r', encoding='utf-8') as f:
    lines = f.readlines()
    lines.append('\n')

    text = []
    labels = []
    label_set = set()

    for line in lines:
        if line == '\n':
            text = ''.join(text)
            entity_labels = []
            for _type, _start_idx, _end_idx in get_entity_bio(labels, id2label=None):
                entity_labels.append({
                    'start_idx': _start_idx,
                    'end_idx': _end_idx,
                    'type': _type,
                    'entity': text[_start_idx: _end_idx + 1]
                })
            if text == '':
                continue
            datalist.append({
                'text': text,
                'label': entity_labels
            })
            text = []
            labels = []
        elif line == '  O\n':
            text.append(' ')
            labels.append('O')
        else:
            line = line.strip('\n').split()
            if len(line) == 1:
                term = ' '
                label = line[0]
            else:
                term, label = line
            text.append(term)
            label_set.add(label.split('-')[-1])
            labels.append(label)

train_data_df = pd.DataFrame(datalist)
train_data_df['label'] = train_data_df['label'].apply(lambda x: str(x))
dev_data_df = pd.DataFrame(datalist[-400:])
dev_data_df['label'] = dev_data_df['label'].apply(lambda x: str(x))

ner_train_dataset = Dataset(train_data_df, categories=label_set)
ner_dev_dataset = Dataset(dev_data_df, categories=ner_train_dataset.categories)

tokenizer = Tokenizer(vocab='hfl/chinese-bert-wwm', max_seq_len=128)

ner_train_dataset.convert_to_ids(tokenizer)
ner_dev_dataset.convert_to_ids(tokenizer)

config = GlobalPointerBertConfig.from_pretrained('hfl/chinese-bert-wwm',
                                                 num_labels=len(ner_train_dataset.cat2id))
torch.cuda.empty_cache()
dl_module = GlobalPointerBert.from_pretrained('hfl/chinese-bert-wwm',
                                              config=config)

# 设置运行次数
num_epoches = 5
batch_size = 8

optimizer = get_default_model_optimizer(dl_module)

model = Task(dl_module, optimizer, 'gpce', cuda_device=0)

model.fit(ner_train_dataset,
          ner_dev_dataset,
          lr=2e-5,
          epochs=num_epoches,
          batch_size=batch_size
         )


ner_predictor_instance = Predictor(model.module, tokenizer, ner_train_dataset.cat2id)

predict_results = []

with open(test_path, 'r', encoding='utf-8') as f:
    lines = f.readlines()
    for _line in tqdm(lines):
        label = len(_line) * ['O']
        for _preditc in ner_predictor_instance.predict_one_sample(_line[:-1]):
            if 'I' in label[_preditc['start_idx']]:
                continue
            if 'B' in label[_preditc['start_idx']] and 'O' not in label[_preditc['end_idx']]:
                continue
            if 'O' in label[_preditc['start_idx']] and 'B' in label[_preditc['end_idx']]:
                continue

            label[_preditc['start_idx']] = 'B-' + _preditc['type']
            label[_preditc['start_idx'] + 1: _preditc['end_idx'] + 1] = (_preditc['end_idx'] - _preditc[
                'start_idx']) * [('I-' + _preditc['type'])]

        predict_results.append([_line, label])

with open('gobal_pointer_baseline1.txt', 'w', encoding='utf-8') as f:
    for _result in predict_results:
        for word, tag in zip(_result[0], _result[1]):
            if word == '\n':
                continue
            f.write(f'{word} {tag}\n')
        f.write('\n')