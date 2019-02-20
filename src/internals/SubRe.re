module Map = OcamlDiff.Map.Make(String);
type subMapItem('msg) = (
  (ref(VdomRe.applicationCallbacks('msg)), unit) => unit,
  ref(option(unit => unit)),
);

type t('msg) =
  | NoSub: t(_)
  | Batch(list(t('msg))): t('msg)
  | Registration(
      string,
      (ref(VdomRe.applicationCallbacks('msg)), unit) => unit,
      ref(option(unit => unit)),
    )
    : t('msg)
  | Mapper(
      ref(VdomRe.applicationCallbacks('msg)) =>
      ref(VdomRe.applicationCallbacks('msgB)),
      t('msgB),
    )
    : t('msg);

type applicationCallbacks('msg) = VdomRe.applicationCallbacks('msg);

let none = NoSub;

let batch = subs => Batch(subs);

let registration = (key, enableCall) =>
  [@implicit_arity]
  Registration(key, callbacks => enableCall(callbacks^), ref(None));

let map = (msgMapper, sub) => {
  open VdomRe;
  let func = callbacks =>
    ref({enqueue: userMsg => callbacks^.enqueue(msgMapper(userMsg))});
  [@implicit_arity] Mapper(func, sub);
};

let mapFunc = (func, sub) => [@implicit_arity] Mapper(func, sub);

let globalizeEnCB:
  type msgB.
    (
      (ref(VdomRe.applicationCallbacks(msgB)), unit) => unit,
      ref(VdomRe.applicationCallbacks('msg)) =>
      ref(VdomRe.applicationCallbacks(msgB)),
      ref(VdomRe.applicationCallbacks('msg)),
      unit
    ) =>
    unit =
  (enCB, mapper, callbacks) => enCB(callbacks |> mapper);



let run:
  (
    ref(VdomRe.applicationCallbacks('glMsg)),
    Map.t(subMapItem('glMsg)),
    t('glMsg)
  ) =>
  Map.t(subMapItem('glMsg)) =
  (callbacks, oldSub, newSub) => {
    let rec subToMap:
      type msg.
        (
          ref(VdomRe.applicationCallbacks('globMsg)) =>
          ref(VdomRe.applicationCallbacks(msg)),
          Map.t(subMapItem('globMsg)),
          t(msg)
        ) =>
        Map.t(subMapItem('globMsg)) =
      (mapper, map, sub) => {
        switch (sub) {
        | NoSub => map
        | [@implicit_arity] Registration(newKey, newEnCB, newDiCB) =>
          map |> Map.add(newKey, (globalizeEnCB(newEnCB, mapper), newDiCB))
        | [@implicit_arity] Mapper(newMapper, newSubSub) =>
          subToMap(r => r |> mapper |> newMapper, map, newSubSub)
        | Batch(newSubs) => newSubs |> List.fold_left(subToMap(mapper), map)
        };
      };

    let newSubMap = newSub |> subToMap(a => a, Map.empty);

    Map.symmetric_diff(
      oldSub,
      newSubMap,
      ~f=
        ((_key, diffRes), _acc) =>
          switch (diffRes) {
          | Left((_enCB, diCB)) =>
            // Js.log("[SubRe] Disabling " ++ key);
            switch (diCB^) {
            | None => ()
            | Some(cb) =>
              diCB := None;
              cb();
            }
          | Right((enCB, diCB)) =>
            // Js.log("[SubRe] Enabling " ++ key);
            diCB := Some(enCB(callbacks))
          | Unequal((_enCB1, diCB1), (_enCB2, diCB2)) => diCB2 := diCB1^
          },
      ~acc=(),
      ~veq=
        ((enCB1, diCB1), (enCB2, diCB2)) =>
          enCB1 === enCB2 && diCB1 === diCB2,
    );

    newSubMap;
  };
