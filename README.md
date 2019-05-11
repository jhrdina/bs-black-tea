# Black TEA

Simplified version of [OvermindDL1/bucklescript-tea](https://github.com/OvermindDL1/bucklescript-tea) with following extra features:

- No view layer. You can subscribe to model changes and specify your own handling ([example](examples/Example.re)) or use our Provider for **ReasonReact** (see [example](examples/ReactExample.re)).
- Improved subscriptions handling
- Future support of both Native and JavaScript compile targets (ReasonReact provider is not available on Native)

## Usage

Add the package to your project

```
npm install bs-black-tea
```

and add a corresponding item to the `bsconfig.json`:

```json
"bs-dependencies": ["bs-black-tea"],
```

For usage info see [**examples**](examples) folder.

## Development

### Build

```
npm run build
```

### Build + Watch

```
npm run start
```

### Run examples

First run webpack in watch mode to create JS bundles using

```
npm run webpack
```

and open `examples/Example.html` or `examples/ReactExample.html`. There is no need to run a webserver.
