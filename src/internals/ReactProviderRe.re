type state('model) = {
  model: 'model,
  unsubscribe: option(unit => unit),
};

type action('model) =
  | UpdateModel('model)
  | Subscribed(unit => unit);

let createMake =
    (~name="Provider", ~store: StoreRe.programInterface('msg, 'model)) => {
  let innerComponent = ReasonReact.reducerComponent(name);
  let make =
      (
        ~component:
           (
             ~model: 'model,
             ~pushMsg: 'msg => unit,
             array(ReasonReact.reactElement)
           ) =>
           ReasonReact.component('a, 'b, 'c),
        _children: array(ReasonReact.reactElement),
      )
      : ReasonReact.component(
          state('model),
          ReasonReact.noRetainedProps,
          action('model),
        ) => {
    ...innerComponent,
    initialState: () => {model: store.getModel(), unsubscribe: None},
    reducer: (action, state) =>
      switch (action) {
      | Subscribed(unsubscribe) =>
        ReasonReact.Update({...state, unsubscribe: Some(unsubscribe)})
      | UpdateModel(model) => ReasonReact.Update({...state, model})
      },
    didMount: ({send}) => {
      send(Subscribed(store.subscribe(model => send(UpdateModel(model)))));
    },
    willUnmount: ({state}) =>
      switch (state.unsubscribe) {
      | Some(unsubscribe) => unsubscribe()
      | None => ()
      },
    render: ({state}) =>
      ReasonReact.element(
        component(~model=state.model, ~pushMsg=store.pushMsg, [||]),
      ),
  };
  make;
};
