export type HardwareAlias = "ptl" | "wcl";

export type HardwareInfo = {
  alias: HardwareAlias;
  chipset: string;
  tier: string;
  color: string;
};

export const HARDWARE: Record<HardwareAlias, HardwareInfo> = {
  ptl: {
    alias: "ptl",
    chipset: "Intel® Core™ Ultra Series 3 Processors",
    tier: "Intel® Core™ Ultra Series 3 Processors",
    color: "#63b3ed",
  },
  wcl: {
    alias: "wcl",
    chipset: "Intel® Core™ Series 3 Processors",
    tier: "Intel® Core™ Series 3 Processors",
    color: "#ff8a3d",
  },
};

const HARDWARE_ALIASES = Object.keys(HARDWARE) as HardwareAlias[];

const HARDWARE_COLOR_FALLBACKS = ["#a78bfa", "#34d399", "#f472b6"];
const DEFAULT_HARDWARE_COLOR = "#c8f000";

const isKnownAlias = (alias: string): alias is HardwareAlias =>
  Object.prototype.hasOwnProperty.call(HARDWARE, alias);

export const getHardware = (alias: string): HardwareInfo | undefined =>
  isKnownAlias(alias) ? HARDWARE[alias] : undefined;

export const listHardware = (): HardwareInfo[] =>
  HARDWARE_ALIASES.map((alias) => HARDWARE[alias]);

export const getChipsetLabel = (alias: string): string =>
  getHardware(alias)?.tier ?? alias.toUpperCase();

export const getTierLabel = (alias: string): string =>
  getHardware(alias)?.tier ?? "";

export const getHardwareLabel = (alias: string): string => {
  const info = getHardware(alias);
  return info ? info.tier : alias.toUpperCase();
};

export const getHardwareColor = (alias: string, index = 0): string =>
  getHardware(alias)?.color ??
  HARDWARE_COLOR_FALLBACKS[index % HARDWARE_COLOR_FALLBACKS.length] ??
  DEFAULT_HARDWARE_COLOR;
