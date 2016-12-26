#!/usr/bin/env python

import msgpack

from cocaine.logger import Logger
from cocaine.worker import Worker
from cocaine.services import Service

from cocaine.decorators import http

log = Logger()

def echo(req,resp):
    log.error("on enter")
    msg = yield req.read()
    log.error("on answer")
    resp.write(str(msg))
    log.error("on leave")
    resp.close()

def inc(req,resp):
    log.error("on enter")
    msg = yield req.read()
    log.error("on answer")
    resp.write(str(int(msg)+1))
    log.error("on leave")
    resp.close()

storage = Service('storage')

@http
def http_getfile(req,resp):

    req = yield req.read()
    # ns, key = msgpack.unpackb(msg, use_list=False)

    args = req.request

    ns  = args['ns'] if args.has_key('ns') else 'store'
    key = args['key'] if args.has_key('key') else 'test.txt'

    try:

        log.error("requesting storage from ns {} key {}".format(ns,key) )

        ch   = yield storage.read(ns, key)
        data = yield ch.rx.get()

    except Exception as err:

        log.error("error in http " + str(err) )
        resp.write_head(502, {})
        resp.write(str(err))

    else:
        log.info("sending response")
        resp.write_head(200, {})
        resp.write(data)

def main():
    w = Worker()
    w.run({
        "ping" : echo,
        "inc"  : inc,
        "getfile" : http_getfile })

if __name__ == '__main__':
    main()
