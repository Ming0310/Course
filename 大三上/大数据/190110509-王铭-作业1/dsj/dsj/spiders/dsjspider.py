import scrapy
from dsj.items import DsjItem
from dsj.items import DsjItem2
# import re
import time
class DsjspiderSpider(scrapy.Spider):
    name = 'dsjspider'
    # allowed_domains = ['movie.douban.com']
    # start_url = 'https://sz.zu.anjuke.com/fangyuan/p{}/?pi=360-cpchz-sz-ty1&kwid=28059985352'
    # start_url = 'https://bj.zu.anjuke.com/fangyuan/p{}/'
    # start_url = 'https://sh.zu.anjuke.com/fangyuan/p{}'
    # start_url = 'https://tj.zu.anjuke.com/fangyuan/p{}'
    # start_url = 'https://gz.zu.anjuke.com/fangyuan/p{}'
    start_url = 'https://cd.zu.anjuke.com/fangyuan/p{}'
    X = 1

    def start_requests(self):
        # urls = ['https://movie.douban.com/chart']
        # # 'https://movie.douban.com/chart'
        # for url in urls:
        #     yield scrapy.Request(url,callback=self.parse_url)
        url = self.start_url.format(self.X)
        yield scrapy.Request(url,callback=self.parse_url2)
        pass

    def parse_url2(self,response):
        list = response.xpath('//*[@id="list-content"]/div')
        for i,div in zip(range(0,62,1),list):
            if i > 1:
                url = div.xpath('./div[1]/h3/a/@href')
                url = url[0].extract()
                yield scrapy.Request(url,callback=self.parse_fy)
        self.X = self.X + 1
        print('self.X:'+str(self.X))
        if self.X < 50 :
            yield scrapy.Request(self.start_url.format(self.X),callback=self.parse_url2)
        pass

    def parse_fy(self,response):
        item = DsjItem2()
        '/html/body/div[3]/div[2]/div[1]/div[6]/b'
        item['descriptor'] = response.xpath('/html/body/div[3]/div[2]/div[1]/div[6]/b')
        if len(item['descriptor'].xpath('string(.)')) > 0:
            item['descriptor'] = item['descriptor'].xpath('string(.)')[0].extract()
        else:
            item['descriptor'] = response.xpath('/html/body/div[3]/div[2]/div[1]/div[7]/b')
            item['descriptor'] = item['descriptor'].xpath('string(.)')[0].extract()
        item['descriptor'] = item['descriptor'].replace('\\n',' ')
        item['price'] = response.xpath('/html/body/div[3]/div[2]/div[1]/ul[1]/li[1]/span[1]/em/b/text()')[0].extract()
        yield item

    # 找每一部电影
    def parse_url(self,response):
        div_list = response.xpath('//*[@id="content"]/div/div[1]/div/div/table')
        for i in range(0,10,1):
            url = div_list[i].xpath('./tr/td[2]/div/a/@href')
            url = url[0].extract()
            yield scrapy.Request(url,callback=self.parse_comment)

    # 找评论url
    def parse_comment(self,response):
        first = response.xpath('//*[@id="comments-section"]/div[1]/h2/span/a/@href')[0].extract()
        # first为要找的评论页面url
        yield scrapy.Request(first,callback=self.parse_next)


    def parse_next(self,response):
        X = 0
        # 一个电影爬10页 十个电影爬100页
        while X < 300:
            next_url = response.xpath('//*[@id="content"]/div/div[2]/p/a/@href')[0].extract()
            next_url = next_url + "comments?start={}&limit=20&status=P&sort=new_score".format(X)
            X = X + 20
            print("------------------------")
            print(next_url)
            yield scrapy.Request(next_url, callback=self.parse)
        pass


    def parse(self, response):
        div_list = response.xpath('//*[@id="comments"]/div')
        name = response.xpath('//*[@id="content"]/h1/text()')[0].extract().split(' ')[0]
        for x in range(0,20,1):
            div = div_list[x]
            comment = div.xpath('./div[2]/p/span/text()')
            item = DsjItem()
            item['name'] = name
            item['comment'] = comment[0].extract()
            yield item
        pass