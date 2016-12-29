#!/usr/bin/env python

from __future__ import with_statement

import msgpack, pprint, sys

from cocaine.services import Service

from tornado.gen import coroutine, Return
from tornado.ioloop import IOLoop
from tornado.concurrent import Future

import functools

class StatPrinter:
    def __init__(self, resDict):
        self.d = resDict

    def pprint(self, sep='='):
        for k in self.d:
            print(k)
            pprint.pprint(self.d[k])
            print(sep * 70)

def readEndpoint(fname):
    with open(fname) as f:
        return f.read().strip()

remote1 = ( ( readEndpoint("endpoints.list") ,  10053), )

service = Service( 'Echo',    remote1 )
storage = Service( 'storage', remote1 )
locator = Service( 'locator', remote1 )


@coroutine
def resolve_all():

    apps = yield direct_storage_find( 'manifests',  'app' )

    # apps = IOLoop.current().run_sync( lambda: direct_storage_find( 'manifests',  'app' ) )

    for a in apps:
        srvData = yield locate(a,None)
        StatPrinter(srvData).pprint()

@coroutine
def direct_storage_find(ns,*keys):

    # print(ns)
    # print(keys)

    ch   = yield storage.find(ns, keys)
    data = yield ch.rx.get()

    print(data)

    raise Return(data)

@coroutine
def direct_storage_get(ns, key):

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
def getapps():
    pass

@coroutine
def locate(srvName, PrintFlag = None, *args):

    print("locating service: {}".format(srvName) )

    ch  = yield locator.resolve(srvName)
    loc = yield ch.rx.get()

    ch = yield locator.cluster(srvName)
    clust = yield ch.rx.get()

    ch = yield locator.routing(srvName)
    rt = yield ch.rx.get()

    ret = dict(locate=loc, cluster=clust, routing=rt)

    # PrintFlag = next(iter(args), None)
    if PrintFlag != None:
        StatPrinter(ret).pprint()

    raise Return(ret)

if __name__ == '__main__':

    cmd = dict(
        ping = main,
        locate = functools.partial(locate, PrintFlag=True),
        getfile = direct_storage_get,
        inc = inc,
        getapps = getapps,
        fnd = direct_storage_find,
        res = resolve_all )

    IOLoop.current().run_sync( lambda : cmd[ sys.argv[1] ]( *sys.argv[2:] ), timeout=5 )
