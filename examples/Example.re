open BlackTea;

let hookOnClick: (string, unit => unit) => unit = [%bs.raw
  (id, cb) => "
  const el = document.getElementById(id);
  el.addEventListener('click', cb);
"
];

let setValue: (string, string) => unit = [%bs.raw
  (id, text) => "
    const el = document.getElementById(id);
    el.textContent = text;
  "
];

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

let prog =
  Store.create(
    ~init,
    ~update,
    ~subscriptions=_ => Sub.none,
    ~shutdown=_ => Cmd.none,
  );

prog.subscribe(model => setValue("value", model |> string_of_int));

hookOnClick("incrementBtn", () => prog.pushMsg(Increment));
hookOnClick("decrementBtn", () => prog.pushMsg(Decrement));