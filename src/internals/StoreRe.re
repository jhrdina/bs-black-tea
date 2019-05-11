// Based on https://github.com/OvermindDL1/bucklescript-tea by OvermindDL1

type pumpInterface('model, 'msg) = {
  startup: unit => unit,
  handleMsg: ('model, 'msg) => 'model,
  shutdown: CmdRe.t('msg) => unit,
};

let programLoop =
    (update, subscriptions, initModel, initCmd, listeners, callbacks) => {
  let subscriptionsMap = ref(SubRe.Map.empty);
  let handleSubscriptionChange = model => {
    let newSub = subscriptions(model);
    subscriptionsMap := SubRe.run(callbacks, subscriptionsMap^, newSub);
  };
  let notifyListeners = model => {
    List.iter(l => l(model), listeners^);
  };
  {
    startup: () => {
      CmdRe.run(callbacks, initCmd);
      notifyListeners(initModel);
      handleSubscriptionChange(initModel);
    },
    handleMsg: (model, msg) => {
      let (newModel, cmd) = update(model, msg);
      CmdRe.run(callbacks, cmd);
      notifyListeners(newModel);
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

type programInterface('msg, 'model) = {
  pushMsg: 'msg => unit,
  shutdown: unit => unit,
  subscribe: ('model => unit, unit) => unit,
  getModel: unit => 'model
};

let programStateWrapper = (initModel, pump, shutdown) => {
  let model = ref(initModel);
  let listeners = ref([]);
  let callbacks =
    ref({VdomRe.enqueue: _msg => Printf.eprintf("INVALID enqueue CALL!\n")});
  let pumperInterface = pump(listeners, callbacks);
  let pending: ref(option(list('msg))) = ref(None);

  let subscribe = listener => {
    listeners := [listener, ...listeners^];
    listener(initModel);
    () => {
      listeners := List.filter(l => listener !== l, listeners^);
    };
  };

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

  let getModel = () => model^;

  pumperInterface.startup();
  {pushMsg: handler, shutdown: pi_requestShutdown, subscribe, getModel};
};

let create =
    (
      ~init: unit => ('model, CmdRe.t('msg)),
      ~update: ('model, 'msg) => ('model, CmdRe.t('msg)),
      ~subscriptions,
      ~shutdown,
    ) => {
  let (initModel, initCmd) = init();
  let pumpInterface = programLoop(update, subscriptions, initModel, initCmd);
  programStateWrapper(initModel, pumpInterface, shutdown);
};