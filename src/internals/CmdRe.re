// Based on https://github.com/OvermindDL1/bucklescript-tea by OvermindDL1

type applicationCallbacks('msg) = VdomRe.applicationCallbacks('msg);

type t('msg) =
  | NoCmd
  | Tagger(ref(applicationCallbacks('msg)) => unit)
  | Batch(list(t('msg)))
  | EnqueueCall(ref(applicationCallbacks('msg)) => unit);

let none = NoCmd;
let batch = cmds => Batch(cmds);
let call = call => EnqueueCall(call);
let msg = msg => VdomRe.(EnqueueCall(callbacks => callbacks^.enqueue(msg)));

let rec run = callbacks =>
  fun
  | NoCmd => ()
  | Tagger(tagger) => tagger(callbacks)
  | Batch(cmds) => cmds |> List.iter(cmd => run(callbacks, cmd))
  | EnqueueCall(cb) =>
    /* let () = Js.log ("Cmd.run", "enqueue", cb) in */
    cb(callbacks);

let wrapCallbacks = (toWrapperMsg, callbacks) =>
  VdomRe.(ref({enqueue: msg => callbacks^.enqueue(msg |> toWrapperMsg)}));

let map = (toWrapperMsg, cmd) =>
  Tagger(callbacks => run(wrapCallbacks(toWrapperMsg, callbacks), cmd));