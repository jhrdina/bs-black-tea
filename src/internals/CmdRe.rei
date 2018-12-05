type applicationCallbacks('msg) = VdomRe.applicationCallbacks('msg);
type t('msg);
let none: t('a);
let batch: list(t('a)) => t('a);
let call: (ref(applicationCallbacks('a)) => unit) => t('a);
let msg: 'a => t('a);
let run: (ref(applicationCallbacks('a)), t('a)) => unit;
let map: ('a => 'b, t('a)) => t('b);