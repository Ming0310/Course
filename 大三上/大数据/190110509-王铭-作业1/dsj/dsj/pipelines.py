# Define your item pipelines here
#
# Don't forget to add your pipeline to the ITEM_PIPELINES setting
# See: https://docs.scrapy.org/en/latest/topics/item-pipeline.html


# useful for handling different item types with a single interface
from itemadapter import ItemAdapter

import json
class DsjPipeline:
    def __init__(self):
        self.f = open(r"./test5.txt","w",encoding='utf-8')
    def process_item(self, item, spider):
        content = str(dict(item))
        content.replace('\n', ' ')
        self.f.write(content)
        self.f.write("\n")
        return item
    def close_spider(self,spider):
        self.f.close()

class DsjPipeline2:
    def __init__(self):
        self.f = open(r"./成都.txt","a+",encoding='utf-8')
        self.x = 0
    def process_item(self, item, spider):
        self.x = self.x + 1
        content = str(dict(item))
        content.replace('<br>', ' ')
        content.replace('</br>',' ')
        content.replace('\\n',' ')
        self.f.write(content)
        self.f.write("\n")
        print(self.x)
        return item
    def close_spider(self,spider):
        self.f.close()