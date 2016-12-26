#!/usr/bin/env python

import msgpack, pprint, sys

from cocaine.services import Service

from tornado.gen import coroutine
from tornado.ioloop import IOLoop

service = Service('Echo', (( open("endpoints.list").read().strip(),10053), ) )
storage = Service('storage')
locator = Service('locator')

@coroutine
def direct_storage_get(ns,key):

    ch = yield storage.read(ns, key)
    data = yield ch.rx.get()

    print(str(data))

@coroutine
def main(msg):

    ch = yield service.enqueue("ping")

    yield ch.tx.write(str(msg))
    yield ch.tx.close()

    res = yield ch.rx.get()
    print('got response message: {0}'.format(res))

@coroutine
def inc():
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

@coroutine
def locate(srvName):

    ch  = yield locator.resolve(srvName)
    loc = yield ch.rx.get()

    print(str(loc))
    pprint.pprint(loc)

if __name__ == '__main__':

    cmd = dict(
        ping = main,
        locate = locate,
        getfile = direct_storage_get,
        inc = inc )

    IOLoop.current().run_sync( lambda : cmd[ sys.argv[1] ]( *sys.argv[2:] ), timeout=5 )
