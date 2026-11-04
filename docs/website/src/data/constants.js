// Canonical values shared by cameras.js and devKits.js.
// Always reference these constants instead of retyping string literals — a
// mistyped reference (e.g. IPU.IPU75XAA) is far easier to spot than a silently
// wrong string ('ipu75xaa'), which would just drop a camera from the catalog.

// Camera connection interfaces (how the camera wires to the kit).
export const INTERFACE = {
  GMSL: 'GMSL',
  MIPI: 'MIPI CSI-2',
  USB: 'USB UVC',
};

// MIPI physical layer. A DPHY sensor on a CPHY connector needs an adapter.
export const PHY = {
  DPHY: 'DPHY',
  CPHY: 'CPHY',
};

// Intel IPU generations. The value is the config-folder code used by the
// camera userspace stack; the label names the platforms that ship that IPU.
export const IPU = {
  IPU6EP: 'ipu6ep',
  IPU6EPMTL: 'ipu6epmtl',
  IPU75XA: 'ipu75xa',
  IPU8: 'ipu8',
};

export const IPU_PLATFORMS = {
  [IPU.IPU6EP]: 'Earlier IPU6EP platforms',
  [IPU.IPU6EPMTL]: 'Meteor Lake, Arrow Lake',
  [IPU.IPU75XA]: 'Intel® Core™ Ultra Series 3 Processors',
  [IPU.IPU8]: 'IPU8 platforms',
};

// Canonical camera capability tags (drive the Capability filter and the
// Type column). Add new tags here first, then use them in cameras.js.
export const CAPABILITY = {
  COLOR: 'Color',
  HDR: 'HDR',
  GLOBAL_SHUTTER: 'Global shutter',
  HIGH_RESOLUTION: 'High resolution',
  DEPTH: 'Depth',
};

// Display order for the Capability filter and the Type column.
export const CAPABILITY_ORDER = [
  CAPABILITY.DEPTH,
  CAPABILITY.HDR,
  CAPABILITY.GLOBAL_SHUTTER,
  CAPABILITY.HIGH_RESOLUTION,
  CAPABILITY.COLOR,
];
