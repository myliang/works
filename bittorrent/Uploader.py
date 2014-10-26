# Written by Bram Cohen
# see LICENSE.txt for license information

from CurrentRateMeasure import Measure

# 上传数据类
class Upload:
    def __init__(self, connection, choker, storage, 
            max_slice_length, max_rate_period, fudge):

        # sock connection
        self.connection = connection
        self.choker = choker
        self.storage = storage
        # 最大slice
        self.max_slice_length = max_slice_length
        # 最大速率间距
        self.max_rate_period = max_rate_period

        # local choke remote
        self.choked = True
        # remote not interested  local
        self.interested = False

        # 缓存
        self.buffer = []

        # 测速对象
        self.measure = Measure(max_rate_period, fudge)

        # 如果存储了数据，那么发送bitfield
        if storage.do_I_have_anything():
            connection.send_bitfield(storage.get_have_list())

    # 处理收到的不感兴趣消息
    def got_not_interested(self):
        if self.interested:
            self.interested = False
            # 删除缓存数据
            del self.buffer[:]

            # remote 对Local不感兴趣，重新rechoke
            self.choker.not_interested(self.connection)

    # 处理收到的感兴趣的消息
    def got_interested(self):
        if not self.interested:
            self.interested = True
            # remote is interested local, and rechoke
            self.choker.interested(self.connection)


    # flushed buffer and send message
    def flushed(self):
        # if the buffer len great zero and connection buffer is zero
        while len(self.buffer) > 0 and self.connection.is_flushed():
            index, begin, length = self.buffer[0]
            del self.buffer[0]
            # read piece from disk
            piece = self.storage.get_piece(index, begin, length)
            # if piece is not readed, so close connection
            if piece is None:
                self.connection.close()
                return
            # updat rate
            self.measure.update_rate(len(piece))
            # send piece
            self.connection.send_piece(index, begin, piece)

    # 处理收到的请求消息
    def got_request(self, index, begin, length):
        #如果remote is not interested local or request len gt max slice length (16K)
        # so close conn
        if not self.interested or length > self.max_slice_length:
            self.connection.close()
            return

        # if local choked remote
        # add request to buffer, and flushed buffer
        if not self.choked:
            self.buffer.append((index, begin, length))
            self.flushed()

    # got cancel message received
    def got_cancel(self, index, begin, length):
        try:
            # remove request message from buffer
            self.buffer.remove((index, begin, length))
        except ValueError:
            pass

    # send choke
    def choke(self):
        # if local unchoked remote
        # and choke it
        if not self.choked:
            self.choked = True
            # clean buffer
            del self.buffer[:]
            # send choke to remoate
            self.connection.send_choke()

    # send unchoke
    def unchoke(self):
        if self.choked:
            self.choked = False
            self.connection.send_unchoke()

    # local is choked remote
    def is_choked(self):
        return self.choked

    # remote is interested local
    def is_interested(self):
        return self.interested

    # has data or not in buffer
    def has_queries(self):
        return len(self.buffer) > 0

    # get update rate
    def get_rate(self):
        return self.measure.get_rate()

class DummyConnection:
    def __init__(self, events):
        self.events = events
        self.flushed = False

    def send_bitfield(self, bitfield):
        self.events.append(('bitfield', bitfield))
    
    def is_flushed(self):
        return self.flushed

    def close(self):
        self.events.append('closed')

    def send_piece(self, index, begin, piece):
        self.events.append(('piece', index, begin, piece))

    def send_choke(self):
        self.events.append('choke')

    def send_unchoke(self):
        self.events.append('unchoke')

class DummyChoker:
    def __init__(self, events):
        self.events = events

    def interested(self, connection):
        self.events.append('interested')
    
    def not_interested(self, connection):
        self.events.append('not interested')

class DummyStorage:
    def __init__(self, events):
        self.events = events

    def do_I_have_anything(self):
        self.events.append('do I have')
        return True

    def get_have_list(self):
        self.events.append('get have list')
        return [False, True]

    def get_piece(self, index, begin, length):
        self.events.append(('get piece', index, begin, length))
        if length == 4:
            return None
        return 'a' * length

def test_skip_over_choke():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    u = Upload(dco, dch, ds, 100, 20, 5)
    assert u.is_choked()
    assert not u.is_interested()
    u.got_interested()
    assert u.is_interested()
    u.got_request(0, 0, 3)
    dco.flushed = True
    u.flushed()
    assert events == ['do I have', 'get have list', 
        ('bitfield', [False, True]), 'interested']

def test_bad_piece():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    u = Upload(dco, dch, ds, 100, 20, 5)
    assert u.is_choked()
    assert not u.is_interested()
    u.got_interested()
    assert u.is_interested()
    u.unchoke()
    assert not u.is_choked()
    u.got_request(0, 0, 4)
    dco.flushed = True
    u.flushed()
    assert events == ['do I have', 'get have list', 
        ('bitfield', [False, True]), 'interested', 'unchoke', 
        ('get piece', 0, 0, 4), 'closed']

def test_still_rejected_after_unchoke():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    u = Upload(dco, dch, ds, 100, 20, 5)
    assert u.is_choked()
    assert not u.is_interested()
    u.got_interested()
    assert u.is_interested()
    u.unchoke()
    assert not u.is_choked()
    u.got_request(0, 0, 3)
    u.choke()
    u.unchoke()
    dco.flushed = True
    u.flushed()
    assert events == ['do I have', 'get have list', 
        ('bitfield', [False, True]), 'interested', 'unchoke', 
        'choke', 'unchoke']

def test_sends_when_flushed():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    u = Upload(dco, dch, ds, 100, 20, 5)
    u.unchoke()
    u.got_interested()
    u.got_request(0, 1, 3)
    dco.flushed = True
    u.flushed()
    u.flushed()
    assert events == ['do I have', 'get have list', 
        ('bitfield', [False, True]), 'unchoke', 'interested', 
        ('get piece', 0, 1, 3), ('piece', 0, 1, 'aaa')]

def test_sends_immediately():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    u = Upload(dco, dch, ds, 100, 20, 5)
    u.unchoke()
    u.got_interested()
    dco.flushed = True
    u.got_request(0, 1, 3)
    assert events == ['do I have', 'get have list', 
        ('bitfield', [False, True]), 'unchoke', 'interested', 
        ('get piece', 0, 1, 3), ('piece', 0, 1, 'aaa')]

def test_cancel():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    u = Upload(dco, dch, ds, 100, 20, 5)
    u.unchoke()
    u.got_interested()
    u.got_request(0, 1, 3)
    u.got_cancel(0, 1, 3)
    u.got_cancel(0, 1, 2)
    u.flushed()
    dco.flushed = True
    assert events == ['do I have', 'get have list', 
        ('bitfield', [False, True]), 'unchoke', 'interested']

def test_clears_on_not_interested():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    u = Upload(dco, dch, ds, 100, 20, 5)
    u.unchoke()
    u.got_interested()
    u.got_request(0, 1, 3)
    u.got_not_interested()
    dco.flushed = True
    u.flushed()
    assert events == ['do I have', 'get have list', 
        ('bitfield', [False, True]), 'unchoke', 'interested', 
        'not interested']

def test_close_when_sends_on_not_interested():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    u = Upload(dco, dch, ds, 100, 20, 5)
    u.got_request(0, 1, 3)
    assert events == ['do I have', 'get have list', 
        ('bitfield', [False, True]), 'closed']

def test_close_over_max_length():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    u = Upload(dco, dch, ds, 100, 20, 5)
    u.got_interested()
    u.got_request(0, 1, 101)
    assert events == ['do I have', 'get have list', 
        ('bitfield', [False, True]), 'interested', 'closed']

def test_no_bitfield_on_start_empty():
    events = []
    dco = DummyConnection(events)
    dch = DummyChoker(events)
    ds = DummyStorage(events)
    ds.do_I_have_anything = lambda: False
    u = Upload(dco, dch, ds, 100, 20, 5)
    assert events == []
