# Camera catalog data

This folder is the single source of truth for the camera catalog and the
kit-compatibility filter rendered by
`src/components/CameraTable` on the **Cameras** landing page
(`docs/sensors/cameras/index.md`). The same data drives two more views:

- `devKits.js` powers the development-kit cards on
  `docs/hardware/index.md`.
- each camera page shows an auto-derived **Compatible development kits** callout
  via `src/components/CameraKitSupport`.

Compatibility is **computed**, not stored as a table. Edit the data here and the
catalog, filters, kit cards, and per-camera callouts update automatically.

## Files

| File | Purpose |
|------|---------|
| `constants.js` | Canonical values: interfaces, PHY types, IPU generations (+ platform names), capability tags. **Reference these instead of typing string literals.** |
| `cameras.js` | One entry per documented camera module. |
| `devKits.js` | One entry per development kit. |
| `compat.js` | Shared `support()` / `supportingKits()` rule used by both `CameraTable` and `CameraKitSupport`. |

## How compatibility is decided

For each camera/kit pair, `compat.js` computes a level (used by both the catalog
table and the per-camera callout):

1. **Not supported** if the kit does not expose the camera's `interface`, **or**
   the camera's `ipuSupport` does not include the kit's `ipu`.
2. **Adapter required** if supported but the camera is MIPI and its `phy`
   differs from the kit's `mipiPhy` (a DPHY sensor on a CPHY kit). Shown as
   "Requires `<kitPhy>`-`<cameraPhy>` adapter" in the Notes column.
3. **Native** otherwise.

## IPU generations

The `ipu` / `ipuSupport` codes map to Intel platforms (see `IPU_PLATFORMS` in
`constants.js`):

| Code | Platforms |
|------|-----------|
| `ipu6epmtl` | Meteor Lake, Arrow Lake |
| `ipu75xa` | Intel® Core™ Ultra Series 3 Processors |
| `ipu6ep` | Earlier IPU6EP platforms |
| `ipu8` | IPU8 platforms |

A camera's `ipuSupport` is the set of IPU generations it ships configuration
files for. A kit's `ipu` is its single IPU generation.

## Add a camera

1. If it needs a new capability tag, add it to `CAPABILITY` (and
   `CAPABILITY_ORDER`) in `constants.js` first.
2. Add an entry to `cameras.js`:
   ```js
   {
     id: 'mipi-xyz123',                 // unique, kebab-case
     name: 'XYZ123',
     capabilities: [CAPABILITY.COLOR, CAPABILITY.HDR],
     interface: INTERFACE.MIPI,         // GMSL | MIPI | USB
     phy: PHY.DPHY,                     // MIPI only; null otherwise
     vendor: 'Acme',
     ipuSupport: [IPU.IPU6EPMTL, IPU.IPU75XA],
     href: '/docs/sensors/cameras/mipi-csi-2/xyz123',
   },
   ```
3. Create the matching camera doc page at `href`, and add the auto-derived
   compatibility callout near the top:
   ```mdx
   import CameraKitSupport from '@site/src/components/CameraKitSupport';

   <CameraKitSupport camera="mipi-xyz123" />
   ```

The **Type** column and the **Capability** filter are derived from
`capabilities` — there is no separate `type` field to keep in sync.

## Add a development kit

Add an entry to `devKits.js`:
```js
{
  id: 'vendor-board',
  name: 'Vendor Board',
  platform: 'Intel® Core™ Ultra Series 3 Processors (Core Ultra X7)',
  ipu: IPU.IPU75XA,
  interfaces: [INTERFACE.GMSL, INTERFACE.MIPI],
  mipiPhy: PHY.CPHY,                    // PHY of the kit's MIPI connectors
  docLink: '/docs/hardware/development-kits/vendor-board/quick-start',
  connectLink: '/docs/hardware/development-kits/vendor-board/connect-a-camera',
  image: require('@site/../docs/hardware/img/vendor-board.png').default,
  description: 'Short card blurb.',
  specs: {                             // card spec bullets
    ai: '…', memory: '…', vision: '…', control: '…',
  },
}
```
The new kit appears as an option in the **Development kit** filter (compatibility
is computed automatically), as a card on `docs/hardware/index.md`, and
in the **Compatible development kits** callout on every camera it supports. Give
it a `connect-a-camera.md` wiring page at `connectLink`.

## Validate

Run `npm run build` after any change. The build fails on broken doc links, so a
camera `href` pointing to a missing page is caught here.
