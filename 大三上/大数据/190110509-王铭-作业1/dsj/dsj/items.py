# Define here the models for your scraped items
#
# See documentation in:
# https://docs.scrapy.org/en/latest/topics/items.html

import scrapy


class DsjItem(scrapy.Item):
    # define the fields for your item here like:
    name = scrapy.Field()
    comment = scrapy.Field()
    pass

class DsjItem2(scrapy.Item):
    descriptor = scrapy.Field()
    price = scrapy.Field()
    pass