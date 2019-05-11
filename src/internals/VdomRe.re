// Based on https://github.com/OvermindDL1/bucklescript-tea by OvermindDL1

type applicationCallbacks('msg) = {enqueue: 'msg => unit};