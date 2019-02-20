type t('msg, 'model) = {
  mutable model: 'model,
  mutable update: ('model, 'msg) => ('model, CmdRe.t('msg)),
  mutable listeners: list(unit => unit),
  customDispatcher: option((t('msg, 'model), 'msg => unit, 'msg) => unit),
};

type pumpInterface('model, 'msg) = {
  startup: unit => unit,
  handleMsg: ('model, 'msg) => 'model,
  shutdown: CmdRe.t('msg) => unit,
};

let programLoop = (update, subscriptions, initModel, initCmd, callbacks) => {
  let subscriptionsMap = ref(SubRe.Map.empty);
  let handleSubscriptionChange = model => {
    /* let open Vdom in */
    let newSub = subscriptions(model);
    subscriptionsMap := SubRe.run(callbacks, subscriptionsMap^, newSub);
  };
  {
    startup: () => {
      CmdRe.run(callbacks, initCmd);
      handleSubscriptionChange(initModel);
    },
    handleMsg: (model, msg) => {
      let (newModel, cmd) = update(model, msg);
      CmdRe.run(callbacks, cmd);
      handleSubscriptionChange(newModel);
      newModel;
    },
    shutdown: cmd => {
      CmdRe.run(callbacks, cmd);
      subscriptionsMap := SubRe.run(callbacks, subscriptionsMap^, SubRe.none);
      ();
    },
  };
};

/* type programInterface('msg) = {. "pushMsg": 'msg => unit};

   [@bs.obj]
      external makeProgramInterface :
        (~pushMsg: 'msg => unit, ~shutdown: unit => unit) => programInterface('msg) =
        ""; */

type programInterface('msg) = {
  pushMsg: 'msg => unit,
  shutdown: unit => unit,
};

let programStateWrapper = (initModel, pump, shutdown) => {
  let model = ref(initModel);
  let callbacks =
    ref({VdomRe.enqueue: _msg => Printf.eprintf("INVALID enqueue CALL!\n")});
  let pumperInterface = pump(callbacks);
  let pending: ref(option(list('msg))) = ref(None);
  let rec handler = msg =>
    switch (pending^) {
    | None =>
      pending := Some([]);
      let newModel = pumperInterface.handleMsg(model^, msg);
      model := newModel;
      switch (pending^) {
      | None =>
        failwith(
          "INVALID message queue state, should never be None during message processing!",
        )
      | Some([]) => pending := None
      | Some(msgs) =>
        pending := None;
        List.iter(handler, List.rev(msgs));
      };
    | Some(msgs) => pending := Some([msg, ...msgs])
    };
  let finalizedCBs: VdomRe.applicationCallbacks('msg) = {
    enqueue: msg => handler(msg),
  };
  callbacks := finalizedCBs;
  let pi_requestShutdown = () => {
    callbacks :=
      {
        enqueue: _msg =>
          Printf.eprintf("INVALID message enqueued when shut down\n"),
      };
    let cmd = shutdown(model^);
    pumperInterface.shutdown(cmd);
  };
  pumperInterface.startup();
  {pushMsg: handler, shutdown: pi_requestShutdown};
};

let create =
    (
      ~init: unit => ('model, CmdRe.t('msg)),
      ~update: ('model, 'msg) => ('model, CmdRe.t('msg)),
      ~subscriptions,
      ~shutdown,
    ) => {
  let (initModel, initCmd) = init();
  /* let store =
     {model: initModel, listeners: [], update, customDispatcher: enhancer}; */
  let pumpInterface = programLoop(update, subscriptions, initModel, initCmd);
  programStateWrapper(initModel, pumpInterface, shutdown);
};

let unsubscribe = (store, listener, ()) =>
  store.listeners = List.filter(l => listener !== l, store.listeners);
let subscribe = (store, listener) => {
  store.listeners = [listener, ...store.listeners];
  unsubscribe(store, listener);
};
let nativeDispatch = (store, msg) => {
  store.model = fst(store.update(store.model, msg));
  List.iter(listener => listener(), store.listeners);
};
let dispatch = (store, msg) =>
  switch (store.customDispatcher) {
  | Some(customDispatcher) =>
    customDispatcher(store, nativeDispatch(store), msg)
  | None => nativeDispatch(store, msg)
  };
let getState = store => store.model;
let replaceReducer = (store, update) => store.update = update;