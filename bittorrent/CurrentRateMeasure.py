# Written by Bram Cohen
# see LICENSE.txt for license information

from time import time

# 测量速率
class Measure:
    def __init__(self, max_rate_period, fudge = 1):
        # 计算速率的时间的阀值 10秒等
        self.max_rate_period = max_rate_period

        # 速率计算的开始时间
        self.ratesince = time() - fudge

        # 最后计算速率的时间
        self.last = self.ratesince
        # 速率（1 s bytes）
        self.rate = 0.0
        # 总字节数
        self.total = 0l

    # 更新速率
    def update_rate(self, amount):
        # 总字节数变大
        self.total += amount
        # 当前时间
        t = time()

        # 更新速率 ？？？？
        self.rate = (self.rate * (self.last - self.ratesince) + 
            amount) / (t - self.ratesince)

        # 更新最后时间
        self.last = t

        # 更新开始时间
        if self.ratesince < t - self.max_rate_period:
            self.ratesince = t - self.max_rate_period

    def get_rate(self):
        self.update_rate(0)
        return self.rate

    # 取得速率
    def get_rate_noupdate(self):
        return self.rate

    #
    def time_until_rate(self, newrate):
        if self.rate <= newrate:
            return 0
        t = time() - self.ratesince
        return ((self.rate * t) / newrate) - t

    # 取得总节数
    def get_total(self):
        return self.total
