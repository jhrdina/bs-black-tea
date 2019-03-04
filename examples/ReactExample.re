open BlackTea;

type model = int;
type msg =
  | Increment
  | Decrement;

let init = () => (42, Cmd.none);
let update = (model, msg) =>
  switch (msg) {
  | Increment => (model + 1, Cmd.none)
  | Decrement => (model - 1, Cmd.none)
  };

let subscriptions = _model => Sub.none;

let store =
  Store.create(
    ~init,
    ~update,
    ~subscriptions=_ => Sub.none,
    ~shutdown=_ => Cmd.none,
  );

module Provider = {
  let make = ReactProvider.createMake(~name="Provider", ~store);
};

module App = {
  let component = ReasonReact.statelessComponent("App");
  let make = (~model, ~pushMsg, _children) => {
    ...component,
    render: _self =>
      <div>
        <div> {model |> string_of_int |> ReasonReact.string} </div>
        <button onClick={_ => pushMsg(Decrement)}>
          {"Decrement" |> ReasonReact.string}
        </button>
        <button onClick={_ => pushMsg(Increment)}>
          {"Increment" |> ReasonReact.string}
        </button>
      </div>,
  };
};

ReactDOMRe.renderToElementWithId(<Provider component=App.make />, "app");