module Provider = {
  type state('reductiveState) = {
    reductiveState: option('reductiveState),
    unsubscribe: option(unit => unit),
  };
  type action =
    | UpdateState
    | AddListener(action => unit);
  let createMake = (~name="Provider", store: StoreRe.t('action, 'state)) => {
    let innerComponent = ReasonReact.reducerComponent(name);
    let make =
        (
          ~component:
             (
               ~state: 'state,
               ~dispatch: 'action => unit,
               array(ReasonReact.reactElement)
             ) =>
             ReasonReact.component('a, 'b, 'c),
          _children: array(ReasonReact.reactElement),
        )
        : ReasonReact.component(
            state('state),
            ReasonReact.noRetainedProps,
            action,
          ) => {
      ...innerComponent,
      initialState: () => {
        reductiveState: Some(StoreRe.getState(store)),
        unsubscribe: None,
      },
      reducer: (action, state) =>
        switch (action) {
        | AddListener(send) =>
          ReasonReact.Update({
            unsubscribe:
              Some(StoreRe.subscribe(store, _ => send(UpdateState))),
            reductiveState: Some(StoreRe.getState(store)),
          })
        | UpdateState =>
          ReasonReact.Update({
            ...state,
            reductiveState: Some(StoreRe.getState(store)),
          })
        },
      didMount: ({send}) => send(AddListener(send)),
      willUnmount: ({state}) =>
        switch (state.unsubscribe) {
        | Some(unsubscribe) => unsubscribe()
        | None => ()
        },
      render: ({state}) =>
        switch (state.reductiveState) {
        | None => ReasonReact.null
        | Some(state) =>
          ReasonReact.element(
            component(~state, ~dispatch=StoreRe.dispatch(store), [||]),
          )
        },
    };
    make;
  };
};