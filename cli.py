#!/usr/bin/env python

import msgpack

from cocaine.services import Service

from tornado.gen import coroutine
from tornado.ioloop import IOLoop

service = Service('Echo')

@coroutine
def main():

    ch = yield service.enqueue("ping")

    yield ch.tx.write("test msg")
    yield ch.tx.close()

    res = yield ch.rx.get()
    print('got response message: {0}'.format(res))

@coroutine
def call_inc():
    ch = yield service.enqueue("inc")

    yield ch.tx.write(str(14))
    yield ch.tx.close()

    res = yield ch.rx.get()
    print('got response message: {0}'.format(res))

@coroutine
def getfile(ns,name):

    try:
        ch = yield service.enqueue("getfile")

        print("requesting ns:{} key:{}".format(ns,name) )
        yield ch.tx.write( msgpack.packb([ns, name]) )
        yield ch.tx.close()

        res = yield ch.rx.get()

    except Exception as e:
        print('error: {}'.format(str(e)))
    else:
        print('got response message: {0}'.format(res))


if __name__ == '__main__':
    # IOLoop.current().run_sync(main, timeout=5)
    IOLoop.current().run_sync( lambda : getfile('store', 'test.txt') , timeout=5)
